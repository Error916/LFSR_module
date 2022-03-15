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
static void lfsr_fill(char*, size_t);

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
	size_t n, remaining = len;
	char tmp[128];

	while (remaining) {
		n = min(remaining, sizeof(tmp));

		lfsr_fill(tmp, n);

		if (copy_to_user(buffer, tmp, n))
			return -EFAULT;

		buffer += n;
		remaining -= n;
	}

	return len;
}

static int lfsr_state(void) {
	char out = ((state >> 127) ^ (state >> 126) ^ (state >> 101) ^ (state >> 99) ^ (state >> 0)) & 1;
	state = (state >> 1) | ( state << 127);
	return out;
}

static void lfsr_fill(char* buffer, size_t size) {
	size_t i, j;

	for (i = 0; i < size; ++i)
		for (j = 0; j < 8; ++j)
			buffer[i] |= lfsr_state() << j;
}

module_init(lfsr_init);
module_exit(lfsr_exit);

MODULE_LICENSE ("GPL v2");
