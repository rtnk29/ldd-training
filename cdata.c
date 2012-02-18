#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "cdata_ioctl.h"

#define BUF_SIZE 128
#define	LCD_SIZE	(320*240*4)

struct cdata_t {
    unsigned long* fb;
    unsigned char* buf;
    unsigned int   index;
    unsigned int   offset;
};

static int cdata_open(struct inode *inode, struct file *filp)
{
    int i;
    int minor;
    struct cdata_t *cdata;

    printk(KERN_INFO "CDATA: in open\n");

    minor = MINOR(inode->i_rdev);
    printk(KERN_INFO "CDATA: minor = %d\n", minor);

    cdata= kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
    cdata->fb = ioremap(0x33f00000, 320*240*4);
    cdata->buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    cdata->index = 0;
    cdata->offset = 0;
    filp->private_data = (void*)cdata;
    return 0;
}

void flush_lcd(void *priv)
{
    struct cdata_t *cdata = (struct cdata *)priv;
    unsigned char *fb;
    unsigned char *pixel;
    int index;
    int offset;
    int i;

    fb = cdata->fb;
    index = cdata->index;
    pixel = cdata->buf;
    offset = cdata->offset;

    for (i = 0; i < index; i++) {
        writeb(pixel[i], fb+offset);
        offset++;
        if (offset >= LCD_SIZE)
        offset = 0;
    }

    cdata->index = 0;
    cdata->offset = offset;
}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size,
    loff_t *off)
{
    int i;
    struct cdata_t *cdata = (struct cdata*)filp->private_data;
    //unsigned long *fb;
    unsigned char* pixel;
    unsigned int index;

    //printk(KERN_INFO "CDATA: in write\n");
#if 0
    while(1) {
        current->state = TASK_INTERRUPTIBLE;
        schedule();
    }
#endif

    // lock
    //fb = cdata->fb;
    index = cdata->index;
    pixel = cdata->buf;
    // unlock

     for (i = 0; i < size; i++) {
        if (index >= BUF_SIZE) {
            // buffer full handle
             flush_lcd((void *)cdata); //Use void* to pass private data to avoid platform dependent issue.
            //index = 0; // This is not a good concept. Use state method like follow.
            index = cdata->index;
        }
        // fb[index] = buf[i]; // Big mistakes to access user space memory
        copy_from_user(&pixel[index], &buf[i], 1);
        index++;
    }

    cdata->index = index;

    return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
    struct cdata_t *cdata = (struct cdata *)filp->private_data;

    flush_lcd((void *)cdata);

    kfree(cdata->buf);
    kfree(cdata);

    return 0;
}

static ssize_t cdata_read(struct file *filp, char *buf, size_t size, loff_t *off)
{
    return 0;
}

static int cdata_ioctl(struct inode *inode, struct file *filp,
unsigned int cmd, unsigned long arg)
{
    int n;
    struct cdata_t *cdata = (struct cdata*)filp->private_data;
    int i;
    unsigned long *fb;

    switch (cmd) {
        case CDATA_CLEAR:
            n = *((int*)arg); //FIXME: need to modify to use copy_from_user
            printk(KERN_INFO "CDATA_CLEAR: %d pixel\n", n);

            //FIXME: lock
            fb = cdata->fb;
            //FIXME unlock

            for(i=0; i<n; i++)
                writel(0x00ff00ff, fb++); //clear to pink
            break;
        default:
            break;
    }
    return 0;
}

static struct file_operations cdata_fops = {
    owner: THIS_MODULE,
    open: cdata_open,
    release: cdata_close,
    write: cdata_write,
    read: cdata_read,
    ioctl: cdata_ioctl,
};

static int cdata_init_module(void)
{
    unsigned long *fb;
    int i;

    if (register_chrdev(121, "cdata", &cdata_fops) < 0) {
        printk(KERN_INFO "CDATA: can't register driver\n");
        return -1;
    }
    printk(KERN_INFO "CDATA: Init\n");
#if 1
    /// Try to write fb
    fb=ioremap(0x33f00000, 320*240*4);
    for(i=0;i<320*240;i++)
        writel(0x00ff0000, fb++); //RED
#endif
    return 0;
}

static void cdata_cleanup_module(void)
{
    unregister_chrdev(121, "cdata");
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
