#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
	printf("argc=%d\n", argc);

	int count = 0;

	if (argc >= 2)
	{
		count = atoi(argv[1]);
		printf("Got count %d, PID=%d\n", count, getpid());
	}
	else
	{
		printf("Root, PID=%d\n", getpid());
	}

	count++;

	if (count >= 5)
	{
		return 0;
	}

	char count_str[2];

	sprintf(count_str, "%d", count);

	return execl(argv[0], "part4_2", count_str, NULL);
}