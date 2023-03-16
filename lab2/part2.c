#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FILENAME "test.txt"

int main(int argc, char **argv)
{
	int fd = open(FILENAME, O_RDWR);
	printf("Opened %s, fd: %d\n", FILENAME, fd);

	struct stat statbuf;
	fstat(fd, &statbuf);

	printf("File size: %ld bytes\n", statbuf.st_size);

	char *data = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
	{
		printf("mmap failed!");
		perror("mmap");
		return 1;
	}

	printf("Read the file!\n");
	printf("\n------------------------------------\n%s\n------------------------------------\n\n", data);

	// Reverse the file
	char tmp;
	for (int start = 0, end = statbuf.st_size - 1; start < end; start++, end--)
	{
		tmp = data[start];
		data[start] = data[end];
		data[end] = tmp;
	}

	printf("Reversed the file:\n");
	printf("\n------------------------------------\n%s\n------------------------------------\n\n", data);
	printf("Closing memory mapping...\n");
	munmap(data, statbuf.st_size);
	printf("Done!\n");
}