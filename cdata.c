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
    struct timer_list	flush_timer;
    struct timer_list	sched_timer;

    wait_queue_head_t	wq;
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

    init_timer(&cdata->flush_timer);
    init_timer(&cdata->sched_timer);

    init_waitqueue_head(&cdata->wq);

    filp->private_data = (void*)cdata;
    return 0;
}

void flush_lcd(unsigned long priv)
{
    struct cdata_t *cdata = (struct cdata *)priv;
    unsigned char *fb;
    unsigned char *pixel;
    int index;
    int offset;
    int i;
    int j;

    fb = cdata->fb;
    index = cdata->index;
    pixel = cdata->buf;
    offset = cdata->offset;

    for (i = 0; i < index; i++) {
        writeb(pixel[i], fb+offset);
        
        offset++;
        if (offset >= LCD_SIZE)
            offset = 0;
        for(j=0;j<100000;j++); // in order to emulate a low speed IO
    }

    cdata->index = 0;
    cdata->offset = offset;
}

void cdata_wake_up(unsigned long priv)
{	
    struct cdata_t *cdata = (struct cdata *)priv;
    struct timer_list *sched;
    wait_queue_head_t	*wq;

    sched = &cdata->sched_timer;
    wq = &cdata->wq;

    wake_up(wq);

    sched->expires = jiffies + 10;
    add_timer(sched);
}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size,
    loff_t *off)
{
    int i;
    struct cdata_t *cdata = (struct cdata*)filp->private_data;
    //unsigned long *fb;
    struct timer_list *timer;
    struct timer_list *sched;
    unsigned char* pixel;
    unsigned int index;
    wait_queue_head_t *wq;
    wait_queue_t wait;

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
    timer = &cdata->flush_timer;
    sched = &cdata->sched_timer;
    wq = &cdata->wq;
    // unlock

     for (i = 0; i < size; i++) {
        if (index >= BUF_SIZE) {
            // buffer full handle
            cdata->index = index;
            timer->expires = jiffies + 5*HZ;
            timer->function = flush_lcd;
            timer->data = (unsigned long)cdata;
            add_timer(timer);

            sched->expires = jiffies + 10;
            sched->function = cdata_wake_up;
            sched->data = (unsigned long)cdata;
            add_timer(sched);

            wait.flags = 0;
            wait.task = current;
            add_wait_queue(wq, &wait);
repeat:
           current->state = TASK_INTERRUPTIBLE;
           schedule();
 
           index = cdata->index;

           if (index != 0)
               goto repeat;

           remove_wait_queue(wq, &wait);
           del_timer(sched);
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
    del_timer(&cdata->flush_timer);

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
            //get_user(n, arg);
            printk(KERN_INFO "CDATA_CLEAR: %d pixel\n", n);

            //FIXME: lock
            fb = cdata->fb;
            //FIXME unlock

            for(i=0; i<n; i++)
                writel(0x00ff00ff, fb++); //clear to pink
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

int cdata_mmap(struct file* filp, struct vm_area_struct *vma)
{
    unsigned long from;
    unsigned long to;
    unsigned long size;

    from = vma->vm_start;
    to = 0x33f00000;
    size = vma->vm_end - vma->vm_start;
    
    //remap_page_range(from, to, size, PAGE_SHARED);
    while(size) {
        remap_page_range(from, to, PAGE_SIZE, PAGE_SHARED);
        from += PAGE_SIZE;
        to += PAGE_SIZE;
        size -= PAGE_SIZE;
    }

    printk(KERN_INFO "CDATA vm start: %08x\n", vma->vm_start);
    printk(KERN_INFO "CDATA vm end: %08x\n", vma->vm_end);

    return 0;
}

static struct file_operations cdata_fops = {
    owner: THIS_MODULE,
    open: cdata_open,
    release: cdata_close,
    write: cdata_write,
    read: cdata_read,
    ioctl: cdata_ioctl,
    mmap: cdata_mmap,
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
