#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define ENABLE_VERBOSE_LOGGING

#define NOT_PRINTF_FD 2
#define NOT_PRINTF_LONG(maxlen, args...)           \
	{                                              \
		char __buf[maxlen];                        \
		int __len = snprintf(__buf, maxlen, args); \
		write(NOT_PRINTF_FD, __buf, __len);        \
	}

#define NOT_PRINTF(args...) NOT_PRINTF_LONG(1024, args)

#define ASSERT(condition, args...) \
	if (!(condition))              \
	{                              \
		NOT_PRINTF(args);          \
		exit(0);                   \
	}

#ifdef ENABLE_VERBOSE_LOGGING
#define DEBUG_LOG(args...) NOT_PRINTF(args);
#else
#define DEBUG_LOG(args...)
#endif // ENABLE_VERBOSE_LOGGING

int main(int argc, char *argv[])
{
	ASSERT(argc == 2, "Usage: $0 file\n");

	// Validate input
	char *filename = argv[1];
	struct stat src_info;

	// XXX: Technically, stat could return nonzero for other reasons
	ASSERT(stat(filename, &src_info) == 0, "Source file must exist!\n");
	ASSERT(S_ISREG(src_info.st_mode), "Source must be a normal file!\n");

	int fd = open(filename, O_RDONLY);
	ASSERT(fd > 0, "Failed to open file!\n");

	// NB: I wanted to try implementing this with mmap, and it's definitely much simpler... however,
	// this prevents us from using stdin as input.
	char *file_start = mmap(NULL, src_info.st_size, PROT_READ, MAP_SHARED, fd, 0);
	ASSERT(file_start != MAP_FAILED, "Failed to mmap!\n");

	for (char *cur = file_start + src_info.st_size; cur >= file_start; cur--)
	{
		ASSERT(write(1, cur, 1) == 1, "Failed to print!\n");
	}

	return 0;
}
