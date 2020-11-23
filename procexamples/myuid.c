#include <stdlib.h>
#include <stdio.h>

main()
{
    printf("UID:%d EUID:%d GID:%d EGID:%d\n",
	getuid(),geteuid(),getgid(),getegid());
}
