#ifndef _CDATA_IOCTL_H_
#define _CDATA_IOCTO_H_

#include <linux/ioctl.h>

#define CDATA_CLEAR _IOW(0x34, 1, int)
#define CDATA_RED   _IO(0x34, 2)
#define CDATA_GREEN _IO(0x34, 3)
#define CDATA_BLUE  _IO(0x34, 4)

#endif
