#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ENABLE_VERBOSE_LOGGING

#define NOT_PRINTF_LONG(maxlen, args...)           \
	{                                              \
		char *__buf = malloc(maxlen);              \
		int __len = snprintf(__buf, maxlen, args); \
		write(1, __buf, __len);                    \
		free(__buf);                               \
	}

#define NOT_PRINTF(args...) NOT_PRINTF_LONG(1024, args)

#ifdef ENABLE_VERBOSE_LOGGING
#define DEBUG_LOG(args...) NOT_PRINTF(args);
#else
#define DEBUG_LOG(args...)
#endif // ENABLE_VERBOSE_LOGGING

int main(int argc, char *argv[])
{
	struct stat *info = malloc(sizeof(struct stat));

	stat(argv[0], info);
	NOT_PRINTF("argv[0] block size: %d\n", info->st_blksize);

	fstat(1, info); // stat("/dev/stdin", info);
	NOT_PRINTF("stdin block size: %d\n", info->st_blksize);

	return 0;
}
