#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "scullbuffer.h"

//#define DEBUG 1

/*The program takes the No of items to be produced and the item string as the parameter*/
int main(int argc, char **argv)
{
	int fd_sb, fd_log, noofitems = 0, res;
	int item_size = SCULL_B_ITEM_SIZE;
	char buffer[item_size];
	char item[item_size];
	char filename[255];

	//Process arguments
	if (argc != 3) {
		printf("Too few arguments to producer. Usage:\n   producer <num_items> <item_type>\n");
		return 0;
	}
	noofitems = atoi(argv[1]);
	sprintf(filename, "Prod_%s.log", argv[2]);

	//Open the scullbuffer device in write mode
	fd_sb = open("/dev/scullbuffer", O_WRONLY);
	if (fd_sb == -1) {
		printf("Failed to open the scullbuffer device\n");
		return 0;
	}

	//Sleep for a moment to give each process time to open scullbuffer
	sleep(1);

	//Clear and open the logfile
	unlink(filename);
	fd_log = open(filename, O_WRONLY|O_CREAT, 0664);
	if (fd_log == -1) {
		printf("Failed to open the producer log file: %s\n", filename);
		goto cleanup;
	} else {
		#ifdef DEBUG
		printf("The name of the new producer log file is %s\n", filename);
		#endif
	}

	//Write the required number of items into the scull buffer
	for (int i = 0; i < noofitems; i++) {
		
		//Write the item content to the producer temporary buffer
		memset(buffer, '\0', item_size);
		sprintf(buffer, "%s%d", argv[2], i);
		#ifdef DEBUG
		printf("The buffer contents are %s\n", buffer);
		#endif

		//Write to scullbuffer and Log the result
		res = write(fd_sb, buffer, item_size);
		if (res == item_size) {
			snprintf(item, strlen(buffer) + 1, "%s", buffer);
			sprintf(item, "%s\n", item);
			write(fd_log, item, strlen(item));
		}

		#ifdef DEBUG
		printf("The number of bytes written are %d\n", res);
		#endif

		//write returns 0 if there is no space left to write and no readers.
		//write returns -1 if an unrecoverable error occurred.
		if (res <= 0)
			goto cleanup;
	}

cleanup:
	close(fd_sb);
	close(fd_log);
	return res;
}
