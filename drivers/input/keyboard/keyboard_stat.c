#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <asm/io.h>


#define KSTAT_PROCF_NAME "keyboard_stat"
#define KSTAT_VERSION "0.0"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wang Xiao");
MODULE_DESCRIPTION("Keyboard press status. WXC");
MODULE_VERSION(KSTAT_VERSION);

#define KEY_STAT(_name, _value)                                                \
	{                                                                      \
		.name = _name, .code = _value, .pressed = 0, .unpressed = 0    \
	}

struct keyboard_scancode {
	unsigned char code;
	unsigned int pressed;
	unsigned int unpressed;
	const char *name;
};

static struct keyboard_scancode stat[] = {KEY_STAT("Esc", 0x01),
					  KEY_STAT("1!", 0x02),
					  KEY_STAT("2@", 0x03),
					  KEY_STAT("3#", 0x04),
					  KEY_STAT("4$", 0x05),
					  KEY_STAT("5%", 0x06),
					  KEY_STAT("6^", 0x07),
					  KEY_STAT("7&", 0x08),
					  KEY_STAT("8*", 0x09),
					  KEY_STAT("9(", 0x0a),
					  KEY_STAT("0)", 0x0b),
					  KEY_STAT("-_", 0x0c),
					  KEY_STAT("=+", 0x0d),
					  KEY_STAT("Backspace", 0x0e),
					  KEY_STAT("Tab", 0x0f),
					  KEY_STAT("Q", 0x10),
					  KEY_STAT("W", 0x11),
					  KEY_STAT("E", 0x12),
					  KEY_STAT("R", 0x13),
					  KEY_STAT("T", 0x14),
					  KEY_STAT("Y", 0x15),
					  KEY_STAT("U", 0x16),
					  KEY_STAT("I", 0x17),
					  KEY_STAT("O", 0x18),
					  KEY_STAT("P", 0x19),
					  KEY_STAT("[{", 0x1a),
					  KEY_STAT("]}", 0x1b),
					  KEY_STAT("Enter", 0x1c),
					  KEY_STAT("LCtrl", 0x1d),
					  KEY_STAT("A", 0x1e),
					  KEY_STAT("S", 0x1f),
					  KEY_STAT("D", 0x20),
					  KEY_STAT("F", 0x21),
					  KEY_STAT("G", 0x22),
					  KEY_STAT("H", 0x23),
					  KEY_STAT("J", 0x24),
					  KEY_STAT("K", 0x25),
					  KEY_STAT("L", 0x26),
					  KEY_STAT(";:", 0x27),
					  KEY_STAT("\'\"", 0x28),
					  KEY_STAT("`~", 0x29),
					  KEY_STAT("LShift", 0x2a),
					  KEY_STAT("\\|", 0x2b),
					  KEY_STAT("Z", 0x2c),
					  KEY_STAT("X", 0x2d),
					  KEY_STAT("C", 0x2e),
					  KEY_STAT("V", 0x2f),
					  KEY_STAT("B", 0x30),
					  KEY_STAT("N", 0x31),
					  KEY_STAT("M", 0x32),
					  KEY_STAT(",<", 0x33),
					  KEY_STAT(".>", 0x34),
					  KEY_STAT("/?", 0x35),
					  KEY_STAT("RShift", 0x36),
					  KEY_STAT("Keypad-*", 0x37),
					  KEY_STAT("LAlt", 0x38),
					  KEY_STAT("Space", 0x39),
					  KEY_STAT("CapsLock", 0x3a),
					  KEY_STAT("F1", 0x3b),
					  KEY_STAT("F2", 0x3c),
					  KEY_STAT("F3", 0x3d),
					  KEY_STAT("F4", 0x3e),
					  KEY_STAT("F5", 0x3f),
					  KEY_STAT("F6", 0x40),
					  KEY_STAT("F7", 0x41),
					  KEY_STAT("F8", 0x42),
					  KEY_STAT("F9", 0x43),
					  KEY_STAT("F10", 0x44),
					  KEY_STAT("NumLock", 0x45),
					  KEY_STAT("ScrollLock", 0x46),
					  KEY_STAT("Keypad-7/Home", 0x47),
					  KEY_STAT("Keypad-8/Up", 0x48),
					  KEY_STAT("Keypad-9/PgUp", 0x49),
					  KEY_STAT("Keypad--", 0x4a),
					  KEY_STAT("Keypad-4/Left", 0x4b),
					  KEY_STAT("Keypad-5", 0x4c),
					  KEY_STAT("Keypad-6/Right", 0x4d),
					  KEY_STAT("Keypad-+", 0x4e),
					  KEY_STAT("Keypad-1/End", 0x4f),
					  KEY_STAT("Keypad-2/Down", 0x50),
					  KEY_STAT("Keypad-3/PgDn", 0x51),
					  KEY_STAT("Keypad-0/Ins", 0x52),
					  KEY_STAT("Keypad-./Del", 0x53),
					  KEY_STAT("Alt-SysRq", 0x54),
					  KEY_STAT("Unknown", 0x80 - 1)};

#define IS_PRESS(code) (((code)&0x80) == 0)

const int keys_count = sizeof(stat) / sizeof(stat[0]);

static void get_scancode(unsigned char code)
{
	int i = 0;
	unsigned char set_to_press = code & 0x7f;

	for (i = 0; i < keys_count - 1; i++) {
		if (stat[i].code == set_to_press) {
			break;
		}
	}
	if (IS_PRESS(code)) {
		stat[i].pressed++;
	} else {
		stat[i].unpressed++;
	}
	return;
}

irq_handler_t keyboard_stat_irq_handler(int _irq, void *_dev_id,
					struct pt_regs *_reg)
{
	static unsigned char scancode;
	scancode = inb(0x60);
	get_scancode(scancode);
	return (irq_handler_t)IRQ_HANDLED;
}

static int kstat_show(struct seq_file *p, void *v)
{
	int i = 0;
	seq_printf(p, "%-15s\t\t %-9s\t\t %-9s\n", "Name", "Pressed",
		   "UnPressed");
	for (; i < keys_count; i++) {
		seq_printf(p, "%-15s\t\t %-9u\t\t %-9u\n", stat[i].name,
			   stat[i].pressed, stat[i].unpressed);
	}

	seq_putc(p, '\n');
	return 0;
}

static int keystat_open(struct inode *inode, struct file *file)
{
	int size = keys_count * 256 + 1024;
	return single_open_size(file, kstat_show, NULL, size);
}

static const struct file_operations key_stat_proc_file_operator = {
    .owner = THIS_MODULE,
    .open = keystat_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static struct proc_dir_entry *kstat_procf = NULL;
struct proc_dir_entry *parent = NULL;

static int __init init_keyboard_stat(void)
{
	int req_irq;
	pr_info("KEYBOARD_STAT: KEYBOARD_STAT Module Initializing. " "Version:" KSTAT_VERSION);
	parent = wxcl_get_proc_root();
	if (parent == NULL) {
		parent = proc_mkdir("wxc", NULL);
	}
	kstat_procf = proc_create(KSTAT_PROCF_NAME, 0644, parent,
				  &key_stat_proc_file_operator);
	if (unlikely(kstat_procf == NULL)) {
		pr_err("KEYBOARD_STAT: Create proc "
		       "file " KSTAT_PROCF_NAME " Failed.");
		return -ENOMEM;
	}

	req_irq = request_irq(1, (irq_handler_t)keyboard_stat_irq_handler,
			      IRQF_SHARED, KSTAT_PROCF_NAME,
			      (irq_handler_t)keyboard_stat_irq_handler);
	if (unlikely(req_irq != 0)) {
		pr_err("KEYBOARD_STAT: request_irq failed.");
		return -req_irq;
	}
	pr_info("KEYBOARD_STAT: Initialized");
	return 0;
}

static void __exit exit_keyboard_stat(void)
{
	remove_proc_entry(KSTAT_PROCF_NAME, parent);
	free_irq(1, (irq_handler_t)keyboard_stat_irq_handler);
	pr_info(KERN_INFO "KEYBOARD_STAT: Keyboard_Stat Module Unloaded.");
}

module_init(init_keyboard_stat);
module_exit(exit_keyboard_stat);
