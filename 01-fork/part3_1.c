#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/*
a) Whatever main() returns, in this case 0.
b) `kill <child>` (SIGTERM) returns 15.
c) If you don't wait for the child, then the parent exits immediately. The child remains alive.
d) Essentially the same as (c)
*/

int main()
{
	printf("Parent Starting, pid=%d...\n", getpid());
	int exitval;
	int pid = fork();
	if (pid == 0)
	{
		printf("Child Starting...\n");
		sleep(10);
		printf("Child Exiting...\n");
		return 0;
	}
	else
	{
		printf("Parent forked, child PID=%d\n", pid);
		wait(&exitval);
		printf("Parent child exited, val=%d\n", exitval);
	}
	printf("Parent Exiting...\n");
	return 0;
}