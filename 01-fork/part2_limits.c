#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/**
 * This fails around the 15,000th child with EAGAIN.
 */

int main()
{
    int original_pid = -1;
    int num_kids = 0;
    while (1)
    {
        int ret = fork();
        if (ret == 0)
        {
            sleep(10);
            return 0;
        }
        else
        {
            if (original_pid == -1)
            {
                original_pid = ret;
            }
            num_kids++;
            if (ret == -1)
            {
                printf("FAILED! To create %dth child! Errno: %d\n", num_kids, errno);
                return 1;
            }
            printf("%dth child created, PID: %d, dPID: %d\n", num_kids, ret, ret - original_pid);
        }
    }
}