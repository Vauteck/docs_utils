/*
 * Module       : drv_ifconfig.o
 * Loading      : insmod   drv_ifconfig
 *                modprobe drv_ifconfig
 * Unloading    : rmmod    drv_ifconfig
 * Reading data : cat /proc/ifconfig
 * 
 * http://www.makelinux.net/ldd3/chp-17-sect-13
 * http://www.comp.nus.edu.sg/~cs4236/readings/out/html/structnet__device__stats.html
 * http://www.comp.nus.edu.sg/~cs4236/readings/out/html/netdevice_8h-source.html
 * /usr/src/linux-2.6.19/include/linux/netdevice.h
 * 
 * struct net_device_stats
 * {
 * 		unsigned long   rx_packets;     // total packets received   	
 * 		unsigned long   tx_packets;     // total packets transmitted    
 * 		unsigned long   rx_bytes;       // total bytes received     	
 * 		unsigned long   tx_bytes;       // total bytes transmitted 		
 * 		unsigned long   rx_errors;      // bad packets received     	
 * 		unsigned long   tx_errors;      // packet transmit problems 	
 * 		unsigned long   rx_dropped;     // no space in linux buffers   
 * 		unsigned long   tx_dropped;     // no space available in linux 	
 * 		unsigned long   multicast;      // multicast packets received   
 * 		unsigned long   collisions;
 * 		
 * 		// detailed rx_errors:
 * 		unsigned long   rx_length_errors;
 * 		unsigned long   rx_over_errors;     // receiver ring buff overflow 
 * 		unsigned long   rx_crc_errors;      // recved pkt with crc error    
 * 		unsigned long   rx_frame_errors;    // recv'd frame alignment error 
 * 		unsigned long   rx_fifo_errors;     // recv'r fifo overrun      
 * 		unsigned long   rx_missed_errors;   // receiver missed packet   
 * 		
 * 		// detailed tx_errors 
 * 		unsigned long   tx_aborted_errors;
 * 		unsigned long   tx_carrier_errors;
 * 		unsigned long   tx_fifo_errors;
 * 		unsigned long   tx_heartbeat_errors;
 * 		unsigned long   tx_window_errors;
 * 		
 * 		// for cslip etc
 * 		unsigned long   rx_compressed;
 * 		unsigned long   tx_compressed;
 * 	};
 * 
 * __u32  ipv4address;
 * printk(KERN_INFO "my ipv4 address is : %d.%d.%d.%d\n", NIPQUAD(ipv4address));
 * 
 */

//#include <linux/config.h>
#include <linux/module.h>		/*									*/
#include <linux/can.h>			/*									*/
#include <linux/can/core.h>		/*									*/
#include <linux/can/raw.h>		/*									*/
#include <asm/checksum.h>		/*									*/
#include <linux/errno.h>  		/* error codes 						*/
#include <linux/etherdevice.h> 	/* eth_type_trans 					*/
#include <linux/fs.h>			/*									*/
#include <linux/in.h>			/*									*/
#include <linux/in6.h>			/*									*/
#include <linux/init.h>			/*									*/
#include <linux/interrupt.h> 	/* mark_bh 							*/
#include <linux/ip.h>          	/* struct iphdr 					*/
#include <linux/if_arp.h>		/*									*/
#include <linux/kernel.h>		/* printk level             		*/
#include <linux/moduleparam.h>	/*									*/
#include <linux/net.h>			/*									*/
#include <linux/netdevice.h>   	/* struct device, and other headers */
#include <net/net_namespace.h>	/*									*/
#include <linux/proc_fs.h>     	/* all the proc stuff       		*/
#include <linux/sched.h>		/* For current              		*/
#include <linux/skbuff.h>		/*									*/
#include <linux/slab.h>			/* kmalloc() 						*/
#include <net/sock.h>			/*									*/
#include <linux/socket.h>		/*									*/
#include <linux/tcp.h>         	/* struct tcphdr 					*/
#include <linux/tty.h>         	/* For the tty declarations 		*/
#include <linux/types.h>  		/* size_t 							*/
#include <linux/uio.h>			/*									*/

/* Driver metadata */
#define PROC_NAME_ENTRY  "ifconfig"
#define PROC_RIGHTS      0644
#define DRV_LICENCE      "GPL"
#define DRV_ALIAS        "ifconfig"
#define DRV_AUTHOR       "GAYET Thierry"
#define DRV_DESC         "Embedded ifconfig"

/* External function */
//extern struct net_device*		dev_get_by_index(int idx);
extern struct net_device*		dev_get_by_index(struct net *net, int ifindex);

/* Global variables */
static struct proc_dir_entry*	proc_ifconfig = NULL;

/*
 * This structure is private to each device. It is used to pass
 * packets in and out, so there is place for a packet
 */
struct proc_dir_priv 
{
	struct net_device_stats stats;
	int                  	status;
	struct snull_packet* 	ppool;
	struct snull_packet* 	rx_queue;  		/* List of incoming packets */
	int 					rx_int_enabled;
	int 					tx_packetlen;
	u8*						tx_packetdata;
	struct sk_buff*			skb;
	spinlock_t 				lock;
}; /* proc_dir_priv */

/*
 * Implementation of the read callback
 */
static int proc_ifconfig_list_netdev(char* page, char** start, off_t off, int count, int* eof, void* data)
{
	int                size       = 0;
	int                i          = 1;	 /* Net devices start counting at 1 */
	char*              hw_types[] = { "Ethernet", "Local Loopback", "Other" };
	struct net_device* dev        = NULL;
	
	printk( KERN_INFO "--> proc_ifconfig_list_netdev() " );
	
	/* 
	 * Search for an interface by index. 
	 * Returns NULL if the device is not found or a pointer to the device. 
	 * The device returned has had a reference added and the pointer is safe 
	 * until the user calls dev_put to indicate they have finished with it.  
	 */
	while (NULL != (dev = (struct net_device*)dev_get_by_index(&init_net, i)))
	{
		printk( KERN_INFO "Current device #%2d ", i );
		
		/* Declare and initialize local data */
		unsigned long   			addr       = 0;
		unsigned long   			bcast      = 0;
		unsigned long				mask       = 0;
		unsigned long 				rx_packets = 0;
		unsigned long 				tx_packets = 0;
		unsigned long 				rx_bytes   = 0;
		unsigned long 				tx_bytes   = 0;
		unsigned long 				rx_errors  = 0;
		unsigned long 				tx_errors  = 0;
		unsigned long 				rx_dropped = 0;
		unsigned long 				tx_dropped = 0;
		unsigned long 				collisions = 0;
		unsigned long 				multicast  = 0;
		struct proc_dir_priv*       priv       = NULL;
		struct net_device_stats* 	stats      = NULL;
		struct in_device*			in_dev 	   = NULL;
		
		/* 
		 * Get the data related to net devices
		 */
		priv  = (struct proc_dir_priv*)netdev_priv(dev);
		//stats = (struct net_device_stats*)priv->stats;
		
		/*if (NULL != dev->get_stats) 
		{
			stats  = dev->get_stats(dev);
		}*/
		if (NULL != (in_dev = dev->ip_ptr))
		{
			//addr  = in_dev->ifa_list->ifa_address;
			//bcast = in_dev->ifa_list->ifa_broadcast;
			//mask  = in_dev->ifa_list->ifa_mask;
		} /* IF */
		
		/* Only check Ethernet and loopback for now */
		printk( KERN_INFO "Type found : %d ", dev->type );
		
		/* 
		 * Display the data 
		 */
		size += sprintf( page+size, "%s\tLink encap:%s\tHWaddr:%s", dev->name, hw_types[dev->type], dev->dev_addr );
		//	size += sprintf( page+size, "inet addr:%s Bcast:%s Mask:%s", addr, bcast, mask );
		//size += sprintf( page+size, "MTU:%s", "" );
		//
		//
		//
		//size += sprintf( page+size, "collisions:%d txqueuelen:%d ", , );

		/* Add a line for real device, not for loopback */
		/*if (type == 1)
		{
			size += sprintf( page+size, "Interrupt: %d  Base address", dev->irq, dev->base_addr&0x0ffff );
		}*/ /* IF */
		
		/* Increase the net device */
		i++;
	} /* WHILE */
	
	printk( KERN_INFO "<-- proc_ifconfig_list_netdev() - size=%d ", size );
	
	return(size);
} /* proc_ifconfig_list_netdev */

/* Chargement du module */
static int init_proc_ifconfig_module(void)
{
	int rc_init = 0;
	
	printk( KERN_INFO "--> proc_ifconfig_list_netdev() " );
	
	/* Creating the /proc/ifconfig entry that will as a read-only callback */
	if (NULL == (proc_ifconfig = create_proc_read_entry(PROC_NAME_ENTRY, PROC_RIGHTS, NULL, proc_ifconfig_list_netdev, NULL)))
	{
		printk( KERN_INFO "Creating the /proc/%s entry : NOK ",  PROC_NAME_ENTRY );
		rc_init = -ENOMEM;
	} else {
		printk( KERN_INFO "Creating the /proc/%s entry : OK ",   PROC_NAME_ENTRY );
		printk( KERN_INFO "For reading the data : cat /proc/%s (read-only) ", PROC_NAME_ENTRY );
	} /* IF */
	
	/* Final return code */
	printk( KERN_INFO "<-- init_proc_ifconfig_module() - rc_init=%d ", rc_init );
	return(rc_init);
} /* init_proc_ifconfig_module */

/* D�chargement du module */
static void cleanup_proc_ifconfig_module(void)
{
	printk( KERN_INFO "--> cleanup_proc_ifconfig_module() " );
	
	remove_proc_entry(PROC_NAME_ENTRY, NULL);
	printk( KERN_INFO "The '%s' module is now unloaded.", THIS_MODULE->name );
	
	printk( KERN_INFO "<-- cleanup_proc_ifconfig_module() " );
} /* cleanup_proc_ifconfig_module */

module_init(init_proc_ifconfig_module);
module_exit(cleanup_proc_ifconfig_module);

MODULE_LICENSE(DRV_LICENCE);
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_ALIAS(DRV_ALIAS);
MODULE_DESCRIPTION(DRV_DESC);

/* EOF */
