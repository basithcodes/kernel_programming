#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

char buffer[2048];
#define TRY_READ 10

int main(int argc, char *argv[]) {

	if (argc != 2) {
		printf("Please input a value <executable file name> <number of bytes to read>\n");
		goto out;
	}

	int fd;

	/* This variable holds remaining data bytes to be read */
	int remaining = atoi(argv[1]);

	printf("Number of bytes requested = %d\n", remaining);

	/* Holds count of total data bytes read so far */
	int total_read = 0;

	int reading = 0, ret = 0;

	fd = open("/dev/pcdev-3", O_RDONLY); 
	if (fd < 0) {
		perror("open");
		goto out;
	}

	while (reading != 2 && remaining) {

		ret = read(fd, &buffer[total_read], remaining);

		if (!ret) {
			/* There is nothing to read */
			printf("End of file\n");
			break;
		}
		else if (ret < 0) {
			printf("Error occured\n");
			break;
		}
		else if (ret <= remaining) {
			/* ret contains count of data bytes successfully read */
			printf("Number of bytes read = %d\n", ret);

			/* add the ret value to total_bytes, so the writing starts from that position*/
			total_read += ret;

			/* Update the remaining bytes to be read */
			remaining -= ret;
			break;
		}
		else 
			break;
		reading++;
	}
	printf("total bytes read = %d\n", total_read);

	for (int i = 0; i < total_read; i++) {
		printf("%c", buffer[i]);
	}

out:
	close(fd);
	return 0;

}
