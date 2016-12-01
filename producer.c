#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define DEBUG 1
#define ITEM_SIZE 32

/*The program takes the No of items to be produced and the item string as the parameter*/
int main(int argc, char **argv) {

	int fd, noofitems = 0, res;
	char * buffer = NULL;

	if (argc != 3) {
		#ifdef DEBUG
		printf("\r\nToo few arguments to producer\r\n");
		#endif
		return 0;
	}

	//Open the file in write mode
	fd = open("/dev/scullbbuffer", O_WRONLY);

	if (fd == -1) {
		#ifdef DEBUG
		printf("\r\nFailed to open the device\r\n");
		#endif
		return 0;
	}

	sleep(3);

	//Allocate an item of size 32 bytes	
	buffer = (char *)malloc(ITEM_SIZE);
	
	noofitems = atoi(argv[1]);

	//Write the required number of items into the scull buffer
	for (int i = 0; i < noofitems; i++) {
		
		sprintf(buffer, "%s000%d", argv[2], i);

		#ifdef DEBUG
		printf("\r\nThe buffer contents are %s\r\n", buffer);
		#endif

		res = write (fd, buffer, ITEM_SIZE);

		#ifdef DEBUG
		printf("\r\nThe number of bytes written are %d\r\n", res);
		#endif

		if (!res)
			//Should we exit or continue to write the next item

		if (res < 0)
			//Should we exit??
		
		memset(buffer, '\0', ITEM_SIZE);
	}

	//free the buffer
	cleanup: free(buffer);
	close(fd);

	return 0;
}
