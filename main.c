/*
 * main.c -- the bare scull char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#include <linux/sched.h>
#include <linux/poll.h>

#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <linux/uaccess.h>	/* copy_*_user */

#include "scullbuffer.h"	/* local definitions */

/*
 * Our parameters which can be set at load time.
 */

struct scull_buffer {
        char *buffer, *end;                /* begin of buf, end of buf */
        int buffersize;                    /* used in pointer arithmetic */
        char *rp, *wp;                     /* where to read, where to write */
        int nreaders, nwriters;            /* number of openings for r/w */
        struct semaphore sem;              /* mutual exclusion semaphore */
        struct semaphore sem_itemavail;    /* */
        struct semaphore sem_spaceavail;   /* */
        struct cdev cdev;                  /* Char device structure */
};

/* parameters */
static int scull_b_nr_devs = SCULL_B_NR_DEVS;	/* number of buffer devices */
dev_t scull_b_devno;			/* Our first device number */

static struct scull_buffer *scull_b_devices;

#define init_MUTEX(_m) sema_init(_m, 1);

int scull_major =   SCULL_MAJOR;
int scull_minor =   0;
int nitems 		= 	20;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_b_nr_devs, int, 0);	/* FIXME check perms */
module_param(nitems, int, 0);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

/*
 * Open and close
 */
static int scull_b_open(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev;

	dev = container_of(inode->i_cdev, struct scull_buffer, cdev);
	filp->private_data = dev;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	//TODO: init buffers in module_init code
	if (!dev->buffer) {
		/* allocate the buffer */
		dev->buffer = kmalloc(nitems * SCULL_B_ITEM_SIZE, GFP_KERNEL);
		if (!dev->buffer) {
			up(&dev->sem);
			return -ENOMEM;
		}
	}
	dev->buffersize = nitems * SCULL_B_ITEM_SIZE;
	dev->end = dev->buffer + dev->buffersize;
	dev->rp = dev->wp = dev->buffer; /* rd and wr from the beginning */

	/* use f_mode,not  f_flags: it's cleaner (fs/open.c tells why) */
	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	up(&dev->sem);

	return nonseekable_open(inode, filp);
}

static int scull_b_release(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev = filp->private_data;

	down(&dev->sem);
	if (filp->f_mode & FMODE_READ) {
		dev->nreaders--;
	}
	if (filp->f_mode & FMODE_WRITE) {
		dev->nwriters--;
	}
	if (dev->nreaders + dev->nwriters == 0) {
		kfree(dev->buffer);
		dev->buffer = NULL; /* the other fields are not checked on open */
	}
	up(&dev->sem);
	return 0;
}

/*
 * Data management: read and write
*/
static ssize_t scull_b_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_buffer *dev = filp->private_data;

	/* check if an item is available without blocking */
	if (down_trylock(&dev->sem_itemavail)) {
		// BUFFER EMPTY
		/* if there are no writers exit */ 
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		if (!dev->nwriters) {
			up(&dev->sem);
			return 0;
		}
		up (&dev->sem);

		/* if there are writers, wait for an item to become available */
		if (down_interruptible(&dev->sem_itemavail))
			return -ERESTARTSYS;
	}
	// BUFFER NOT EMPTY
		
	PDEBUG("\" (scull_b_read) dev->wp:%p    dev->rp:%p\" \n",dev->wp,dev->rp);

	/* check correct flags */
	if (filp->f_flags & O_NONBLOCK) {
		up(&dev->sem_itemavail); //we didn't actually consume the item
		return -EAGAIN;
	}

	// <CRITICAL consume an item from the buffer
	if (down_interruptible(&dev->sem)) {
		up(&dev->sem_itemavail); //we didn't actually consume the item
		return -ERESTARTSYS;
	}

	if (copy_to_user(buf, dev->rp, count)) {
		up(&dev->sem);
		up(&dev->sem_itemavail); //we didn't actually consume the item
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp == dev->end)
		dev->rp = dev->buffer; /* wrapped */
	up (&dev->sem);
	// CRITICAL\>

	/* signal writers that space is available */
	up(&dev->sem_spaceavail);

	PDEBUG("\"%s\" did read %li bytes\n",current->comm, (long)count);
	return count;
}

static ssize_t scull_b_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_buffer *dev = filp->private_data;

	/* check if space is available without blocking */
	if (down_trylock(&dev->sem_spaceavail)) {
		// BUFFER FULL
		/* if there are no readers exit */ 
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		if (!dev->nreaders) {
			up (&dev->sem);
			return 0;
		}
		up (&dev->sem);

		/* if there are readers, wait for space to become available */
		if (down_interruptible(&dev->sem_spaceavail))
			return -ERESTARTSYS;
	}
	// BUFFER NOT FULL

	PDEBUG("Going to accept %li bytes to %p from %p\n", (long)count, dev->wp, buf);

	//CRITICAL deposit item into buffer
	if (down_interruptible(&dev->sem)) {
		up(&dev->sem_spaceavail); //we didn't actually consume the space
		return -ERESTARTSYS;
	}

	if (copy_from_user(dev->wp, buf, count)) {
		up(&dev->sem);            //give up access to the buffer
		up(&dev->sem_spaceavail); //we didn't actually consume the space
		return -EFAULT;
	}
	dev->wp += count;
	if (dev->wp == dev->end)
		dev->wp = dev->buffer; /* wrapped */
	PDEBUG("\" (scull_b_write) dev->wp:%p    dev->rp:%p\" \n",dev->wp,dev->rp);
	up(&dev->sem);
	//end CRITICAL

	/* finally, awake a reader */
	up(&dev->sem_itemavail);

	PDEBUG("\"%s\" did write %li bytes\n",current->comm, (long)count);
	return count;
}

/*
 * The file operations for the buffer device
 * (some are overlayed with bare scull)
 */
struct file_operations scull_buffer_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.read =		scull_b_read,
	.write =	scull_b_write,
	.open =		scull_b_open,
	.release =	scull_b_release,
};

/*
 * Set up a cdev entry.
 */
static void scull_b_setup_cdev(struct scull_buffer *dev, int index)
{
	int err, devno = scull_b_devno + index;

	cdev_init(&dev->cdev, &scull_buffer_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scullbuffer%d", err, index);
}

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void scull_b_cleanup_module(void)
{
	int i;

	if (!scull_b_devices)
		return; /* nothing else to release */

	for (i = 0; i < scull_b_nr_devs; i++) {
		cdev_del(&scull_b_devices[i].cdev);
		kfree(scull_b_devices[i].buffer);
	}

	kfree(scull_b_devices);
	unregister_chrdev_region(scull_b_devno, scull_b_nr_devs);
}

int scull_b_init_module(void)
{
	int result, i;
	dev_t dev = 0;
/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_b_nr_devs, "scullbuffer");
	} else {
		result = alloc_chrdev_region(&dev, scull_minor, scull_b_nr_devs, "scullbuffer");
		scull_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scullb: can't get major %d\n", scull_major);
		return 0;
	}

	scull_b_devno = dev;
	scull_b_devices = kmalloc(scull_b_nr_devs * sizeof(struct scull_buffer), GFP_KERNEL);
	if (scull_b_devices == NULL) {
		unregister_chrdev_region(dev, scull_b_nr_devs);
		return 0;
	}
	memset(scull_b_devices, 0, scull_b_nr_devs * sizeof(struct scull_buffer));
	for (i = 0; i < scull_b_nr_devs; i++) {
		init_MUTEX(&scull_b_devices[i].sem);
		sema_init(&scull_b_devices[i].sem_itemavail, 0);
		sema_init(&scull_b_devices[i].sem_spaceavail, nitems);
		scull_b_setup_cdev(scull_b_devices + i, i);
	}

	return scull_b_nr_devs;
}

module_init(scull_b_init_module);
module_exit(scull_b_cleanup_module);
