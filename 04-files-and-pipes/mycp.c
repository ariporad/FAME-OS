#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ENABLE_VERBOSE_LOGGING

#define NOT_PRINTF_LONG(maxlen, args...)           \
	{                                              \
		char __buf[maxlen];                        \
		int __len = snprintf(__buf, maxlen, args); \
		write(1, __buf, __len);                    \
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
	ASSERT(argc >= 2 && argc <= 3, "Usage: $0 src [dest]\n");

	// Validate input
	struct stat src_info;

	// XXX: Technically, stat could return nonzero for other reasons
	ASSERT(stat(argv[1], &src_info) == 0, "Source file must exist!\n");
	ASSERT(S_ISREG(src_info.st_mode), "Source must be a normal file!\n");

	if (argc == 3)
	{
		struct stat dst_info;
		ASSERT(stat(argv[2], &dst_info) != 0, "Destination file must not exist!\n");
	}

	// Copy
	int block_size = src_info.st_blksize;
	int src_fd = open(argv[1], O_RDONLY);
	int dst_fd = argc == 3 ? open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR) : 1;

	void *buf = malloc(block_size);
	int i = 0;

	while (true)
	{
		int bytes_read = read(src_fd, buf, block_size);

		ASSERT(bytes_read >= 0, "Failed to read file!\n");

		if (bytes_read == 0)
		{
			break;
		}

		int bytes_written = write(dst_fd, buf, bytes_read);
		ASSERT(bytes_written == bytes_read, "Wrote the wrong number of bytes! (Read: %d, Wrote: %d)", bytes_read, bytes_written)

		i += bytes_read;
	}

	return 0;
}
