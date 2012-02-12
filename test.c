/*
* Filename: test.c
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

int main(void)
{
    int fd;
    char buffer[100];
    fd = open("/dev/cdata", O_RDWR);
    write(fd,buffer,10);
    close(fd);
}
