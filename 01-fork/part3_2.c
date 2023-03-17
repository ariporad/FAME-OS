#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define NUM_PROCS 10
#define MIN_SLEEP 2
#define MAX_SLEEP 10

int child()
{
	int pid = getpid();
	int sleep_time = (pid % MAX_SLEEP) + MIN_SLEEP;
	printf("[%d] Starting, sleeping for %d seconds...\n", pid, sleep_time);
	sleep(sleep_time);
	printf("[%d] Exiting\n", pid);
	return 0;
}

int main()
{
	printf("[Parent] Starting, pid=%d...\n", getpid());

	// Spawn
	for (int i = 1; i <= NUM_PROCS; i++)
	{
		int pid = fork();
		if (pid < 0)
		{
			printf("ERROR! Failed to fork: %d\n", errno);
			return 1;
		}
		else if (pid > 0)
		{
			printf("[Parent] Created child #%d, pid=%d\n", i, pid);
		}
		else
		{
			return child();
		}
	}

	int status;

	printf("[Parent] Waiting for exits...\n");

	while (1)
	{
		int pid = wait(&status);

		if (pid == -1)
		{
			printf("[Parent] No more child processes!\n");
			break;
		}

		printf("[Parent] Child %d exited with status=%d\n", pid, status);
	}
	printf("[Parent] Exiting...\n");
	return 0;
}