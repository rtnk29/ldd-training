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
#include <asm-arm/arch-s3c2410/S3C2410.h>
#include "cdata_ioctl.h"

#define CDATA_TS_MINOR 50 // FIXME: define in miscdevice.h

void cdata_bh(unsigned long);
DECLARE_TASKLET(my_tasklet,  cdata_bh, NULL);

struct cdata_ts {
    struct input_dev ts_input;
    int x;
    int y;
};

void cdata_ts_handler(int irq, void* priv, struct pt_reg* reg)
{
    struct cdata_ts *cdata = (struct cdata_ts *)priv;

    printk(KERN_INFO "TS TOP interrupt\n");
    //while(1); --> HANG
    //FIXME: read (x,y) from ADC
    cdata->x = 100;
    cdata->y = 100;
    
    my_tasklet.data = (unsigned long)cdata;

    tasklet_schedule(&my_tasklet);
}

void cdata_bh(unsigned long priv)
{
    struct cdata_ts *cdata = (struct cdata_ts *)priv;
    struct input_dev *dev = &cdata->ts_input;

    printk(KERN_INFO "TS BH \n");
    //while(1); // ----> only TOP interrupt allow
    input_report_abs(dev, ABS_X, cdata->x);
    input_report_abs(dev, ABS_Y, cdata->y);
}

static int ts_input_open(struct input_dev* dev)
{
}

static int ts_input_close(struct input_dev* dev)
{
}

static int cdata_ts_open(struct inode *inode, struct file *filp)
{
    struct cdata_ts *cdata;

    cdata = kmalloc(sizeof(struct cdata_ts), GFP_KERNEL);

    set_gpio_ctrl(GPIO_YPON);
    set_gpio_ctrl(GPIO_YPON);
    set_gpio_ctrl(GPIO_YPON);
    set_gpio_ctrl(GPIO_YPON);
    
    ADCTSC = DOWN_INT | XP_PULL_UP_EN |\
             XP_AIN | XM_HIZ | YP_AIN | YM_GND |\
             XP_PST(WAIT_INT_MODE);

    if (request_irq(IRQ_TC, cdata_ts_handler, 0, "cdata_ts", (void *)cdata))
    {
        printk(KERN_INFO "request irq failed\n");
        return -1;
    }
    
    /** handling input device ***/
    cdata->ts_input.name = "cdata-ts";
    cdata->ts_input.open = ts_input_open;
    cdata->ts_input.close = ts_input_close;

    // capabilties
    cdata->ts_input.absbit[0] = BIT(ABS_X) | BIT(ABS_Y);

    input_register_device(&cdata->ts_input);

    cdata->x = 0;
    cdata->y = 0;

    filp->private_data = (void *)cdata;

    return 0;
}


static ssize_t cdata_ts_write(struct file *filp, const char *buf, size_t size,
    loff_t *off)
{
    return 0;
}

static int cdata_ts_close(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t cdata_ts_read(struct file *filp, char *buf, size_t size, loff_t *off)
{
    return 0;
}

static int cdata_ts_ioctl(struct inode *inode, struct file *filp,
unsigned int cmd, unsigned long arg)
{
    return 0;
}

static struct file_operations cdata_ts_fops = {
    owner: THIS_MODULE,
    open: cdata_ts_open,
    release: cdata_ts_close,
    write: cdata_ts_write,
    read: cdata_ts_read,
    ioctl: cdata_ts_ioctl,
};

static struct miscdevice cdata_ts_misc = {
    minor: CDATA_TS_MINOR,
    name: "cdata-ts",
    fops:  &cdata_ts_fops,
};


static int cdata_ts_init_module(void)
{
    unsigned long *fb;
    int i;

    if (misc_register(&cdata_ts_misc) < 0) {
        printk(KERN_INFO "CDATA-TS: can't register driver\n");
        return -1;
    }
    printk(KERN_INFO "CDATA-TS: Init\n");

    return 0;
}

static void cdata_ts_cleanup_module(void)
{
}

module_init(cdata_ts_init_module);
module_exit(cdata_ts_cleanup_module);

MODULE_LICENSE("GPL");
