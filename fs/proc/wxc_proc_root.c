#include <linux/proc_fs.h>
#include "internal.h"

struct proc_dir_entry *wxcl_get_proc_root(void)
{
	struct file *fp = filp_open("/proc/wxc", O_RDONLY | O_DIRECTORY, 0);
	struct proc_dir_entry *ent;
	if (fp == NULL) {
		return NULL;
	}
	ent = PDE(fp->f_path.dentry->d_inode);
	filp_close(fp, NULL);
	return ent;
}
EXPORT_SYMBOL(wxcl_get_proc_root);
