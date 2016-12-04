#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "scullbuffer.h"

//#define DEBUG 1

int main(int argc, char **argv) {

	int fd_sb, fd_log, noofitems = 0, res;
	int item_size = SCULL_B_ITEM_SIZE;
	char buffer[item_size];
	char item[item_size];
	char filename[255];
	
	//Process arguments
	if (argc < 2) {
		printf("Too few arguments to producer. Usage:\n   producer <num_items> [consumer_name]\n");
		return 0;
	}
	noofitems = atoi(argv[1]);
	if (argc == 3)
		sprintf(filename, "Cons_%s.log", argv[2]);
	else
		sprintf(filename, "Cons.log");

	//Open the scullbuffer device in write mode
	fd_sb = open("/dev/scullbuffer", O_RDONLY);
	if (fd_sb == -1) {
		#ifdef DEBUG
		printf("Failed to open the device %d\n", errno);
		#endif
		goto cleanup;
	}

	//Sleep for a moment to give each process time to open scullbuffer
	sleep(1);

	//Clear and open the logfile
	unlink(filename);
	fd_log = open(filename, O_WRONLY|O_CREAT, 0664);
	if (fd_log == -1) {
		printf("Failed to open the consumer log file: %s\n", filename);
		goto cleanup;
	}

	//Read the required number of items from the scull buffer
	for (int i = 0; i < noofitems; i++) {
	
		#ifdef DEBUG
		printf("Before read call\n");
		#endif

		//Read an item from scullbuffer into consumer buffer and log the result
		memset(buffer, '\0', item_size);
		res = read(fd_sb, buffer, item_size);
		if (res == item_size) {
			snprintf(item, strlen(buffer) + 1, "%s", buffer);
			sprintf(item, "%s\n", item);
			write(fd_log, item, strlen(item));
		}

		#ifdef DEBUG
		printf("The buffer read is %s\n", buffer);
		#endif

		//read returns 0 if there are no producers and no data left to read
		//read returns -1 if an unrecoverable error occurred.
		if (res <= 0)
			goto cleanup;
	}

cleanup:
	close(fd_sb);
	close(fd_log);

	return 0;
}
