/*
 $ make

 # insmod ./hello_version.ko qui="Linus"
 Hello Linus. You are currently using Linux 2.6.20.21.
 
 # cat /proc/hello_version_elapsed
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/proc_fs.h>
static char *qui = "Master";
module_param(qui, charp, 0);
static struct timeval start;
static int get_elapsed(void)
{
	struct timeval end;
	do_gettimeofday(&end);
	return end.tv_secstart.
	tv_sec;
}
static int read_proc_elapsed(char *buffer, char **start, off_t fpos, int
							 length)
{
	sprintf(buffer, "%d\n", get_elapsed());
	return strlen(buffer);
}
static int hello_version_init(void)
{
	int error;
	error = 0;
	printk(KERN_ALERT "Hello %s. You are currently using Linux %s.\n",
		   qui, init_uts_ns.name.release);
	do_gettimeofday(&start);
	if (create_proc_info_entry("hello_version_elapsed", 0, NULL,
							   read_proc_elapsed) == NULL) {
		error=ENOMEM;
	}
	return error;
}
static void hello_version_exit(void)
{
	printk(KERN_ALERT "Goodbye, %s.\n", qui);
	printk(KERN_ALERT "This module has been loaded for %d seconds.\n",
		   get_elapsed());
	remove_proc_entry("hello_version_elapsed", NULL);
}
module_init(hello_version_init);
module_exit(hello_version_exit);
MODULE_LICENSE("GPL");
