#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/random.h>

#define DEVICE_NAME "lfsr"

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
static int lfsr_state(void);

static const struct file_operations fops = {
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int major;
static __uint128_t state;

static int __init lfsr_init(void) {
	uint64_t seed = get_random_u64();
	state = ((__uint128_t)1 << 127) | (__uint128_t)seed;
	major = register_chrdev(0, DEVICE_NAME, &fops);

	if (major < 0) {
		printk(KERN_INFO "lfsr load failed\n");
		return major;
	}

	printk(KERN_INFO "lfsr module has been loaded: %d\n", major);
	return 0;
}

static void __exit lfsr_exit(void) {
	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "lfsr module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "lfsr device opened\n");
	return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
	printk(KERN_INFO "Sorry, lfsr is read only\n");
	return -EFAULT;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "lfsr device closed\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
	int i, errors = 0;
	char c = 0;
	for(i = 0; i < 8; ++i)
		c |= lfsr_state() << i;

	errors = copy_to_user(buffer, &c, 1);

	return errors == 0 ? 1 : -EFAULT;
}

static int lfsr_state(void) {
	uint8_t out = state & 1;
	state = (state >> 1) | ((state ^ (state >> 2) ^ (state >> 27) ^ (state >> 29)) << 127);
	return out;
}

module_init(lfsr_init);
module_exit(lfsr_exit);

MODULE_LICENSE ("GPL v2");
