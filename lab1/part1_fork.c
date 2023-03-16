#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_PCB()
{
    printf("\
PCB | PPID: %d\n\
    | PID:  %d\n\
    | UID:  %d\n\
    | GID:  %d\n\
",
           getppid(), getpid(), getuid(), getgid());
}

int main()
{
    pid_t ret = fork();

    if (ret == 0)
    {
        printf("I am the new child!\n");
    }
    else
    {
        printf("In the parent, fork returned: %d\n", ret);
    }
    print_PCB();
    exit(EXIT_SUCCESS);
}

/**
 * Answer to 1.3: There are 3 forks, each of which produces two children,
 * so there are 2^3 = 8 children total
 *
 * original:  fork -> fork -> fork -> end
 *             \         \        \-> end
 *              \         \-> fork -> end
 *               \                \-> end
 *                \-> fork -> fork -> end
 *                       \        \-> end
 *                        \-> fork -> end
 *                                \-> end
 *
 */