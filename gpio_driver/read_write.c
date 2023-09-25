#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("G. Daneels");
MODULE_DESCRIPTION("Registers a device number and implements some cb functions.");

static char buffer[255];
static int buffer_pointer;

#define DRIVER_NAME "mydummydriver"
#define DRIVER_CLASS "MyModuleClass2"

/* Variables for device and device class. */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;

static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
    int to_copy;
    int not_copied;
    int delta;

    /* Get amount of data to copy. */
    to_copy = min(buffer_pointer, count); 

    /* copy data to user */
    // returns the number of bytes it hasn't copied
    not_copied = copy_to_user(user_buffer, buffer, to_copy);

    /* calculate data to copy */
    delta = to_copy - not_copied;

    return delta;
}

/**
 * write data to buffer.
 */
static ssize_t driver_write(struct file* File, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy;
    int not_copied;
    int delta;

    /* Get amount of data to copy. */
    to_copy = min(count, sizeof(buffer)); 

    /* copy data from user */
    // returns the number of bytes it hasn't copied
    not_copied = copy_from_user(buffer, user_buffer, to_copy);
    buffer_pointer = to_copy;

    /* calculate data to copy */
    delta = to_copy - not_copied;

    return delta;
}

/**
 * @brief This function is called when the device file is opened
 */
static int driver_open(struct inode* device_file, struct file* instance) {
    printk("dev_nr - open was called!\n");
    return 0;
}

/**
 * @brief This function is called when the device file is closed
 */
static int driver_close(struct inode* device_file, struct file* instance) {
    printk("dev_nr - close was called!\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .read = driver_read,
    .write = driver_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel.
 */
static int __init ModuleInit(void) {
    int retval;
	printk("Hello, Kernel!\n");
    
    /* Allocat device number. */
    if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
        printk("Device number could not be allocated.\n");
        return -1;
    }
    printk("read_write - Device nr Major: %d, Minor: %d was registered.\n", my_device_nr >> 20, my_device_nr && 0xFFFFF);

    /* Create a device class */
    if ((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
        printk("Device class can not be created!\n");
        goto ClassError;
    }
    
    /* Create device file */
    if (device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
        printk("Can not create device file.\n");
        goto FileError;
    }

    /* Initialize device file */
    cdev_init(&my_device, &fops);

    /* Registering device to kernel. */
    // make character device
    // this call will add the device to the /dev directory
    if (cdev_add(&my_device, my_device_nr, 1) == -1) {
        printk("Registering of device to kernel failed.\n");
        goto AddError;
    }
	return 0;
AddError:
    device_destroy(my_class, my_device_nr);
FileError:
    class_destroy(my_class);
ClassError:
    unregister_chrdev(my_device_nr, DRIVER_NAME);
    return -1;

}

/**
 * @brief This function is called, when the module is removed from the kernel.
 */
static void __exit ModuleExit(void) {
    cdev_del(&my_device);
    device_destroy(my_class, my_device_nr);
    class_destroy(my_class);
    unregister_chrdev(my_device_nr, DRIVER_NAME);
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

