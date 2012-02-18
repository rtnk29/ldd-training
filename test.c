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
    char pix[4] = {0x00, 0xff, 0x00, 0xff};

    fd = open("/dev/cdata", O_RDWR);
    ioctl(fd, CDATA_CLEAR, &i);
    write(fd, pix, 4); // write 1 dot
    write(fd, pix, 4); // write 1 dot

    close(fd);
}
