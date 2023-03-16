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

static int uninitialized_int[1024];
static int initialized_int[1024] = {42};

const int CONSTANT_DATA[1024] = {43};

int main(int argc, char **argv)
{
	printf("My Guesses:\n");

	// Both of these are maped to the executable itself
	printf("-> Data (Initialized global data): %p\n", initialized_int);
	printf("-> BBS (Uninitialized global data): %p\n", uninitialized_int);

	printf("-> RO-Data (global constant): %p\n", CONSTANT_DATA); // this is nowhere?

	int *heap_int = malloc(1);
	printf("-> Heap (normal): %p\n", heap_int); // this is in the normal heap section

	int *heap_big = malloc(1024 * 1024);
	printf("-> Heap (big): %p\n", heap_big); // this gets a new section (created by mmap)

	int stack_int = 3;
	printf("-> Stack: %p\n", &stack_int); // this is on the stack

	printf("-> main(): %p\n", main);						// in the executable section of the file
	printf("-> LibC (specifically, printf): %p\n", printf); // in the executable section of LibC

	// mmap a file
	int fd = open("part1.c", O_RDONLY);
	char *mmaped_data = mmap(NULL, 20, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mmaped_data == MAP_FAILED)
	{
		printf("mmap failed!");
		perror("mmap");
		return 0;
	}

	printf("-> mmaped file: %p\n", mmaped_data); // this is its own nicely-labeled section

	// Now print the truth
	printf("\n\nActual:\n");

	char *pid_str = malloc(16);
	snprintf(pid_str, 16, "%d", getpid());

	int child_pid = fork();
	if (child_pid == 0)
	{
		execl("/usr/bin/pmap", "pmap", "-X", pid_str, NULL);
	}
	else
	{
		wait(NULL);
	}

	return 0;
}