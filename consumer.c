#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

//#define DEBUG 1
#define ITEM_SIZE 32

int main(int argc, char **argv) {

	int fd, fd1, noofitems = 0, res;
	char *buffer = NULL;
	char *item = NULL;
	
	if (argc != 2) {
		#ifdef DEBUG
		printf("\r\nToo few arguments to producer\r\n");
		#endif
		return 0;
	}

	//Open the file in write mode
	//Which scullbuffer device to open from 0-3????
	fd = open("/dev/scullbuffer", O_RDONLY);

	if (fd == -1) {
		#ifdef DEBUG
		printf("\r\nFailed to open the device %d\r\n", errno);
		#endif
		goto cleanup;
	}

	sleep(1);

	//Allocate an item of size 32 bytes	
	buffer = (char *)malloc(ITEM_SIZE);
	item = (char *)malloc(ITEM_SIZE);
	
	noofitems = atoi(argv[1]);

	//Deleting the file before every execution
	unlink("cons.log");

	fd1 = open("cons.log", O_WRONLY|O_CREAT, 0740);

	if (fd1 == -1) {
		#ifdef DEBUG
		printf("\r\nFailed to open the conslog file\r\n");
		#endif
		goto cleanup;
	}

	//Write the required number of items into the scull buffer
	for (int i = 0; i < noofitems; i++) {
	
		#ifdef DEBUG
		printf("\r\nBefore read call\r\n");
		#endif

		res = read (fd, buffer, ITEM_SIZE);

		if (res == ITEM_SIZE) {
			memset(item, '\0', ITEM_SIZE);
			snprintf(item, strlen(buffer) + 1, "%s", buffer);
			sprintf(item, "%s\n", item);
			write (fd1, item, strlen(item));
		}
	
		if (!res) {
			//Should we exit or continue to write the next item
		}

		if (res < 0) {
			//Should we exit??
			goto cleanup;
		}

		#ifdef DEBUG
		printf("\r\nThe buffer read is %s\r\n", buffer);
		#endif
		memset(buffer, '\0', ITEM_SIZE);
	}

	//free the buffer
cleanup:
	free(buffer);
	close(fd);
	close(fd1);

	return 0;
}
