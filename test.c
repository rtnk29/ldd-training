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
    char pix2[4] = {0xff, 0xff, 0xff, 0xff};
    //pid_t pid;

    //pid=fork();
    fd = open("/dev/cdata", O_RDWR);
/*
    //pid=fork();
    if(pid==0){
        ioctl(fd, CDATA_CLEAR, &i);
        while(1) {
            write(fd, pix, 4); // write 1 dot
        }
    } else {
        while(1) {
            write(fd, pix2, 4); // write 1 dot
        }        
    }
*/
    mmap(0, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    sleep(15);
    close(fd);
}
