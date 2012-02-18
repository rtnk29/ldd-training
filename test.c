/*
* Filename: test.c
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "cdata_ioctl.h"

int main(void)
{
    int fd;
    int i= 10000;
    //char buffer[100];
    fd = open("/dev/cdata", O_RDWR);
    ioctl(fd, CDATA_CLEAR, &i);
    //write(fd,buffer,10);

    close(fd);
}
