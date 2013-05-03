/* **************** "":1.0 s_21/lab2_resource.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2009
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/* Accessing I/O Ports
 *
 * Look at /proc/ioports to find a free I/O port region.
 *
 * Write a simple module that checks if the region is available, and
 * requests it.
 *
 * Check and see if the region is properly registered in /proc/ioports
 *
 * Make sure you release the region when done.
 *
 * The module should send some data to the region, and read some data
 * from it.  Do the values agree?  If not, why?
 @*/

/* IOPORT FROM 0x200 to 0x240 is free on my system (64 bytes) */
#define IOSTART  0x200
#define IOEXTEND 0x40
#define IOEND    IOSTART+IOEXTEND

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/init.h>

static unsigned long iostart = IOSTART, ioextend = IOEXTEND, ioend;
module_param(iostart, ulong, S_IRUGO);
module_param(ioextend, ulong, S_IRUGO);

static struct resource my_resource;

static int __init my_init(void)
{
	unsigned long ultest = (unsigned long)100;
	ioend = iostart + ioextend;

	my_resource.name = "my_ioport";
	my_resource.start = iostart;
	my_resource.end = ioend;

	printk(KERN_INFO " requesting the IO region from 0x%lx to 0x%lx\n",
	       iostart, ioend);

	if (request_resource(&ioport_resource, &my_resource)) {
		printk(KERN_INFO "the IO REGION is busy, quitting\n");
		return -EBUSY;
	}

	printk(KERN_INFO " writing a long=%ld\n", ultest);
	outl(ultest, iostart);

	ultest = inl(iostart);
	printk(KERN_INFO " reading a long=%ld\n", ultest);

	return 0;
}

static void __exit my_exit(void)
{
	int rc;
	printk(KERN_INFO " releasing  the IO region from 0x%lx to 0x%lx\n",
	       iostart, ioend);
	rc = release_resource(&my_resource);
	printk(KERN_INFO "return value from release_resource is %d\n", rc);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION(""":1.0 s_21/lab2_resource.c");
MODULE_LICENSE("GPL v2");
