/* **************** LDD:1.0 s_16/lab1_copytofrom.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein and the Linux Foundation, 2009
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@linuxfoundation.org
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/*
 * Using get_user() and put_user(). (copy_to,from_user solution)
 *
 * Adapt your character driver to use get_user() and put_user().
 *
 @*/

#include <linux/module.h>
#include "lab_char.h"

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_generic_read,
    .write = mycdrv_generic_write,
    .open = mycdrv_generic_open,
    .release = mycdrv_generic_release,
};

module_init (my_generic_init);
module_exit (my_generic_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_16/lab1_copytofrom.c");
MODULE_LICENSE ("GPL v2");
