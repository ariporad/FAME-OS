#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
	printf("argc=%d\n", argc);
	for (int i = 0; i < argc; i++)
	{
		printf("Arg %d=%s\n", i, argv[i]);
	}

	if (argc < 2)
	{
		printf("Must provide a command!\n");
		return 1;
	}

	// Combine arguments
	char *cmd = "";
	int cmdsize = sizeof(cmd);

	for (int i = 1; i < argc; i++)
	{
		char *tmp = cmd;
		int newsize = (cmdsize + strlen(argv[i]) + 1);

		cmd = malloc(sizeof(char) * newsize);
		cmdsize = newsize;
		strcat(cmd, tmp);
		strcat(cmd, " ");
		strcat(cmd, argv[i]);
	}

	// https://stackoverflow.com/a/19209148
	printf("Running: `%s`\n", cmd);

	return system(cmd);
}