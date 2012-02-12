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

static int cdata_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "CDATA: in open\n");
    if(MINOR(inode->i_rdev) != 0) {
        printk(KERN_INFO "CDATA: open error number\n");
    }
    printk(KERN_INFO "CDATA: minor %d\n", MINOR(inode->i_rdev));
    return 0;
}

ssize_t cdata_write(struct file *filp, const char *buf, size_t size,
    loff_t *off)
{
    return 0;
}

int cdata_close(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t cdata_read(struct file *filp, char *buf, size_t size,
    loff_t *off)
{
    return 0;
}

int cdata_ioctl(struct file *filp, int cmd, struct file_lock* lock)
{
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

int cdata_init_module(void)
{
    if (register_chrdev(121, "cdata", &cdata_fops) < 0) {
        printk(KERN_INFO "CDATA: can't register driver\n");
        return -1;
    }
    printk(KERN_INFO "CDATA: Init\n");
    return 0;
}

void cdata_cleanup_module(void)
{
    unregister_chrdev(121, "cdata");
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
