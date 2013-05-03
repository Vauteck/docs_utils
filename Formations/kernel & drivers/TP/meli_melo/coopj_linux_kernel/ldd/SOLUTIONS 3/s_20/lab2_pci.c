/* **************** LDD:1.0 s_20/lab2_pci.c **************** */
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
 * PCI Devices
 *
 * Write a module that scans your PCI devices, and gathers information
 * about them.
 *
 * For each found device, read some information from its configuration
 * register. (Make sure you read /usr/src/linux/include/linux/pci.h to
 * get symbolic names.)  Fields you may wish to obtain could include:
 * PCI_VENDOR_ID, PCI_DEVICE_ID, PCI_REVISION_ID, PCI_INTERRUPT_LINE,
 * PCI_LATENCY_TIMER, PCI_COMMAND.
 *
 * The information you obtain should agree with that in /proc/pci.
 @*/

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/init.h>

static int __init my_init (void)
{
    u16 dval;
    char byte;
    int j = 0;
    struct pci_dev *pdev = NULL;

    printk (KERN_INFO "LOADING THE PCI_DEVICE_FINDER\n");

    /* either of the following looping constructs will work */

    for_each_pci_dev (pdev) {

        /*    while ((pdev = pci_get_device
           (PCI_ANY_ID, PCI_ANY_ID, pdev))) { */

        printk (KERN_INFO "\nFOUND PCI DEVICE # j = %d, ", j++);
        printk (KERN_INFO "READING CONFIGURATION REGISTER:\n");

        printk (KERN_INFO "Bus,Device,Function=%s", pci_name (pdev));

        pci_read_config_word (pdev, PCI_VENDOR_ID, &dval);
        printk (KERN_INFO " PCI_VENDOR_ID=%x", dval);

        pci_read_config_word (pdev, PCI_DEVICE_ID, &dval);
        printk (KERN_INFO " PCI_DEVICE_ID=%x", dval);

        pci_read_config_byte (pdev, PCI_REVISION_ID, &byte);
        printk (KERN_INFO " PCI_REVISION_ID=%d", byte);

        pci_read_config_byte (pdev, PCI_INTERRUPT_LINE, &byte);
        printk (KERN_INFO " PCI_INTERRUPT_LINE=%d", byte);

        pci_read_config_byte (pdev, PCI_LATENCY_TIMER, &byte);
        printk (KERN_INFO " PCI_LATENCY_TIMER=%d", byte);

        pci_read_config_word (pdev, PCI_COMMAND, &dval);
        printk (KERN_INFO " PCI_COMMAND=%d\n", dval);

        /* decrement the reference count and release */
        pci_dev_put (pdev);

    }
    return 0;
}

static void __exit my_exit (void)
{
    printk (KERN_INFO "UNLOADING THE PCI DEVICE FINDER\n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_20/lab2_pci.c");
MODULE_LICENSE ("GPL v2");
