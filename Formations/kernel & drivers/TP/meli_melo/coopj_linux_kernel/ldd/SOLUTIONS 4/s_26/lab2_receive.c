/* **************** "":1.0 s_26/lab2_receive.c **************** */
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
/*
 * Adding Reception
 *
 * Extend your transmitting device driver to include a reception
 * function.
 *
 * You can do a loopback method in which any packet sent out is
 * received.
 *
 * Be careful not to create memory leaks!
 *
 @*/
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/init.h>

static struct net_device *dev;
static struct net_device_stats *stats;

static void my_rx(struct sk_buff *skb, struct net_device *dev)
{
	/* just a loopback, already has the skb */
	printk(KERN_INFO "I'm receiving a packet\n");
	++stats->rx_packets;
	netif_rx(skb);
}
static int my_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int i;
	printk(KERN_INFO "my_hard_start_xmit(%s)\n", dev->name);

	dev->trans_start = jiffies;
	printk(KERN_INFO "Sending packet :\n");

	/* print out 16 bytes per line */
	for (i = 0; i < skb->len; ++i) {
		if ((i & 0xf) == 0)
			printk(KERN_INFO "\n  ");
		printk(KERN_INFO "%02x ", skb->data[i]);
	}
	printk(KERN_INFO "\n");

	++stats->tx_packets;

	/* loopback it */
	my_rx(skb, dev);

	return 0;
}
static int my_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	printk(KERN_INFO "my_do_ioctl(%s)\n", dev->name);
	return -1;
}
static struct net_device_stats *my_get_stats(struct net_device *dev)
{
	printk(KERN_INFO "my_get_stats(%s)\n", dev->name);
	return stats;
}

/*
 * This is where ifconfig comes down and tells us who we are, etc.
 * We can just ignore this.
 */
static int my_config(struct net_device *dev, struct ifmap *map)
{
	printk(KERN_INFO "my_config(%s)\n", dev->name);
	if (dev->flags & IFF_UP) {
		return -EBUSY;
	}
	return 0;
}
static int my_change_mtu(struct net_device *dev, int new_mtu)
{
	printk(KERN_INFO "my_change_mtu(%s)\n", dev->name);
	return -1;
}
static int my_open(struct net_device *dev)
{
	printk(KERN_INFO "Hit: my_open(%s)\n", dev->name);

	/* start up the transmission queue */

	netif_start_queue(dev);
	return 0;
}
static int my_close(struct net_device *dev)
{
	printk(KERN_INFO "Hit: my_close(%s)\n", dev->name);

	/* shutdown the transmission queue */

	netif_stop_queue(dev);
	return 0;
}

#ifdef HAVE_NET_DEVICE_OPS
static struct net_device_ops ndo = {
	.ndo_open = my_open,
	.ndo_stop = my_close,
	.ndo_start_xmit = my_hard_start_xmit,
	.ndo_do_ioctl = my_do_ioctl,
	.ndo_get_stats = my_get_stats,
	.ndo_set_config = my_config,
	.ndo_change_mtu = my_change_mtu,
};
#endif
static void my_setup(struct net_device *dev)
{
	int j;
	printk(KERN_INFO "my_setup(%s)\n", dev->name);

	/* Fill in the MAC address with a phoney */

	for (j = 0; j < ETH_ALEN; ++j) {
		dev->dev_addr[j] = (char)j;
	}

	ether_setup(dev);

#ifdef HAVE_NET_DEVICE_OPS
	dev->netdev_ops = &ndo;
#else
	dev->open = my_open;
	dev->stop = my_close;
	dev->hard_start_xmit = my_hard_start_xmit;
	dev->do_ioctl = my_do_ioctl;
	dev->get_stats = my_get_stats;
	dev->set_config = my_config;
	dev->change_mtu = my_change_mtu;
#endif
	dev->flags |= IFF_NOARP;
	stats = &dev->stats;

	/*
	 * Just for laughs, let's claim that we've seen 50 collisions.
	 */
	stats->collisions = 50;

}
static int __init my_init(void)
{
	printk(KERN_INFO "Loading transmitting network module:....");
	dev = alloc_netdev(0, "mynet%d", my_setup);
	if (register_netdev(dev)) {
		printk(KERN_INFO " Failed to register\n");
		free_netdev(dev);
		return -1;
	}
	printk(KERN_INFO "Succeeded in loading %s!\n\n", dev_name(&dev->dev));
	return 0;
}
static void __exit my_exit(void)
{
	printk(KERN_INFO "Unloading transmitting network module\n\n");
	unregister_netdev(dev);
	free_netdev(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Bill Shubert");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION(""":1.0 s_26/lab2_receive.c");
MODULE_LICENSE("GPL v2");
