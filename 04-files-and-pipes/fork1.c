#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define ENABLE_VERBOSE_LOGGING

#define NOT_PRINTF_FD STDERR_FILENO
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

int parent(pid_t child_pid, int fd_write)
{
	sleep(2);

	DEBUG_LOG("[P] Parent starting...\n");

	int chunk_size = 100;
	void *buf = malloc(chunk_size);

	while (true)
	{
		int len = read(STDIN_FILENO, buf, chunk_size);
		ASSERT(len >= 0, "[P] Failed to read stdin!\n");
		if (len > 0)
		{
			DEBUG_LOG("[P] Writing %d bytes...\n", len);
			write(fd_write, buf, len);
			// sleep(1); // This is required for the pipe to work for some reason? Isn't that the point of threads?
		}
		else // EOF
		{
			DEBUG_LOG("[P] EOF, exiting...\n");
			kill(child_pid, SIGKILL);
			break;
		}
	}
	waitpid(child_pid, NULL, 0);
	return 0;
}

int child(int fd_read)
{
	DEBUG_LOG("[C] Hello!\n");

	int chunk_size = 100;
	void *buf = malloc(chunk_size);

	while (true)
	{
		int len = read(fd_read, buf, chunk_size);
		DEBUG_LOG("[C] Read %d bytes...\n", len);
		ASSERT(len >= 0, "[C] Failed to read from pipe!\n");
		ASSERT(write(STDOUT_FILENO, buf, len) == len, "[C] Failed to write!\n");
	}
}

int main(int argc, char *argv[])
{
	int pipe_fds[2];
	ASSERT(pipe(pipe_fds) == 0, "[M] Failed to establish pipe!\n");

	int fd_write = pipe_fds[1];
	int fd_read = pipe_fds[0];

	pid_t pid;
	if ((pid = fork()))
	{
		close(fd_read);
		return parent(pid, fd_write);
	}
	else
	{
		close(fd_write);
		return child(fd_read);
	}

	return 0;
}
