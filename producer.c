#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

//#define DEBUG 1
#define ITEM_SIZE 32

/*The program takes the No of items to be produced and the item string as the parameter*/
int main(int argc, char **argv) {

	int fd, fd1, noofitems = 0, res;
	char * buffer = NULL;
	char * item = NULL;
	char *filename = NULL;
	int  perms = 0740;

	if (argc != 3) {
		#ifdef DEBUG
		printf("\r\nToo few arguments to producer\r\n");
		#endif
		return 0;
	}

	//Open the file in write mode
	fd = open("/dev/scullbuffer", O_WRONLY);

	if (fd == -1) {
		#ifdef DEBUG
		printf("\r\nFailed to open the device\r\n");
		#endif
		return 0;
	}

	sleep(1);

	//Allocate an item of size 32 bytes	
	buffer = (char *)malloc(ITEM_SIZE);
	item = (char *)malloc(ITEM_SIZE);
	filename = (char *)malloc(255);	
	noofitems = atoi(argv[1]);

	memset(filename, 0, 255);

	sprintf(filename, "Prod%s.log", argv[2]);

	#ifdef DEBUG
	printf("\r\nThe name of prod file is %s\r\n", filename);
	#endif

	//Removing the file on every execution
	unlink(filename);

	fd1 = open(filename, O_WRONLY|O_CREAT, perms);

	if (fd1 == -1) {
		#ifdef DEBUG
		printf("\r\nFailed to open the prodlog file\r\n");
		#endif
		goto cleanup;
	}

	//Write the required number of items into the scull buffer
	for (int i = 0; i < noofitems; i++) {
		
		sprintf(buffer, "%s000%d", argv[2], i);

		#ifdef DEBUG
		printf("\r\nThe buffer contents are %s\r\n", buffer);
		#endif

		res = write (fd, buffer, ITEM_SIZE);

		if (res == ITEM_SIZE) {
			memset(item, '\0', ITEM_SIZE);
			snprintf(item, strlen(buffer) + 1, "%s", buffer);
			sprintf(item, "%s\n", item);
			write (fd1, item, strlen(item));
		}

		#ifdef DEBUG
		printf("\r\nThe number of bytes written are %d\r\n", res);
		#endif

		if (!res)
			goto cleanup;
			//Should we exit or continue to write the next item

		if (res < 0)
			//Should we exit??
		
		memset(buffer, '\0', ITEM_SIZE);
	}

	//free the buffer
cleanup:
	free(buffer);
	free(filename);
	close(fd);
	close(fd1);
	return 0;
}
