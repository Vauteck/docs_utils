/************************************************************************\
  Chapitre "Peripheriques et protocoles reseau"
  exemple_05

  Collecte des statistiques reseau

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

  Librement inspire d'un exemple du livre "Linux Device Driver"
  d'Alessandro Rubini et Jonathan Corbet.

\************************************************************************/

#include <linux/module.h>
#include <linux/version.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/skbuff.h>

	struct net_device * net_dev_ex_0 = NULL;
	struct net_device * net_dev_ex_1 = NULL;

	struct net_dev_priv_exemple {

		struct sk_buff * sk_b;

		unsigned char data[ETH_DATA_LEN];
		int data_len;
		
		/* statistiques */
		struct net_device_stats net_dev_stats;
	};

static irqreturn_t irq_tx_handler_exemple(int irq, void * irq_id, struct pt_regs * regs);
static irqreturn_t irq_rx_handler_exemple(int irq, void * irq_id, struct pt_regs * regs);


static int open_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: open_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	net_dev->dev_addr[0] = 0x00;
	net_dev->dev_addr[1] = 0x20;
	net_dev->dev_addr[2] = 0x07;
	net_dev->dev_addr[3] = 0x20;
	net_dev->dev_addr[4] = 0x07;
	if (net_dev == net_dev_ex_0)
		net_dev->dev_addr[5] = 0x00;
	else
		net_dev->dev_addr[5] = 0x01;
		
	netif_start_queue(net_dev);
	return 0;
}


static int stop_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: stop_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	netif_stop_queue(net_dev);
	return 0;
}


static int hard_start_xmit_exemple(struct sk_buff * sk_b, struct net_device * src)
{
	struct net_dev_priv_exemple * dst_priv;
	struct net_dev_priv_exemple * src_priv;
	
	struct net_device * dst;
	struct iphdr * ip_header;
	
	unsigned char * ptr_src;
	unsigned char * ptr_dst;

	char * data;
	int    len;
	char   short_packet[ETH_ZLEN];

	printk(KERN_INFO "%s: hard_start_xmit_exemple(%p, %p)\n",
	       THIS_MODULE->name, sk_b, src);

	if (src == net_dev_ex_0)
		dst = net_dev_ex_1;
	else
		dst = net_dev_ex_0;

	src_priv = netdev_priv(src);
	dst_priv = netdev_priv(dst);

	data = sk_b->data;
	len  = sk_b->len;
	
	if (len < ETH_ZLEN) {
		memset(short_packet, 0, ETH_ZLEN);
		memcpy(short_packet, data, len);
		len = ETH_ZLEN;
		data = short_packet;
	}
	
	if (len > ETH_DATA_LEN)
		return -ENOMEM;

	src->trans_start = jiffies;
	
	src_priv->sk_b = sk_b;

	ip_header = (struct iphdr *) (data + sizeof(struct ethhdr));

	ptr_src = (unsigned char *) &(ip_header -> saddr);
	ptr_dst = (unsigned char *) &(ip_header -> daddr);
		
	ptr_src += 2;
	* ptr_src = 255 - *ptr_src;
	ptr_dst += 2;
	* ptr_dst = 255 - *ptr_dst;
	
	ip_header->check = 0;
	ip_header->check = ip_fast_csum((unsigned char *)ip_header, ip_header->ihl);

	memcpy(dst_priv->data, data, len);
	dst_priv->data_len = len;

	irq_rx_handler_exemple (0, (void *) dst, NULL);

	src_priv->net_dev_stats.tx_packets ++;
	src_priv->net_dev_stats.tx_bytes += len;
	irq_tx_handler_exemple (0, (void *) src, NULL);
	
	return 0;
}


static irqreturn_t irq_rx_handler_exemple(int irq, void * irq_id, struct pt_regs * regs)
{
	unsigned char * data;
	struct sk_buff * sk_b;
	struct net_device * net_dev;
	struct net_dev_priv_exemple * priv;

	printk(KERN_INFO "%s: irq_rx_handler(%d, %p, %p)\n",
	       THIS_MODULE->name, irq, irq_id, regs);


	net_dev = (struct net_device *) irq_id;
	priv = netdev_priv(net_dev);
	if (priv == NULL)
		return IRQ_NONE;
		
	sk_b = dev_alloc_skb(priv->data_len);
	if (! sk_b)
		return IRQ_HANDLED;

	data = skb_put(sk_b, priv->data_len);
	memcpy(data, priv->data, priv->data_len);

	priv->net_dev_stats.rx_packets ++;
	priv->net_dev_stats.rx_bytes += priv->data_len;

	sk_b->dev = net_dev;
	sk_b->protocol = eth_type_trans(sk_b, net_dev);
	sk_b->ip_summed = CHECKSUM_UNNECESSARY;
	netif_rx(sk_b);

	return IRQ_HANDLED;
}


static irqreturn_t irq_tx_handler_exemple(int irq, void * irq_id, struct pt_regs * regs)
{
	struct net_device * net_dev;
	struct net_dev_priv_exemple * priv;

	printk(KERN_INFO "%s: irq_tx_handler(%d, %p, %p)\n",
	       THIS_MODULE->name, irq, irq_id, regs);

	net_dev = (struct net_device *) irq_id;
	priv = netdev_priv(net_dev);
	if (priv == NULL)
		return IRQ_NONE;
		
	dev_kfree_skb(priv->sk_b);
	return IRQ_HANDLED;
}

static int hard_header_exemple(struct sk_buff * sk_b, struct net_device * net_dev,
                        unsigned short type, void * dst_addr, void * src_addr,
                        unsigned int len)
{
	struct ethhdr * eth_hdr = NULL;

	printk(KERN_INFO "%s: hard_header(%p, %p, %d, %p, %p, %d)\n",
	       THIS_MODULE->name, sk_b, net_dev, type, dst_addr, src_addr, len);
	
	eth_hdr = (struct ethhdr *) skb_push(sk_b, ETH_HLEN);
	eth_hdr->h_proto = htons(type);
	
	if (src_addr == NULL)
		src_addr = net_dev->dev_addr;
	if (dst_addr == NULL)
		dst_addr = net_dev->dev_addr;
		
	memcpy(eth_hdr->h_source, src_addr, net_dev->addr_len);
	memcpy(eth_hdr->h_dest,   dst_addr, net_dev->addr_len);

	if (eth_hdr->h_dest[ETH_ALEN-1] == 0)
		eth_hdr->h_dest[ETH_ALEN-1] = 1;
	else
		eth_hdr->h_dest[ETH_ALEN-1] = 0;
		
	return net_dev->hard_header_len;
}


static struct net_device_stats * get_stats_exemple(struct net_device * net_dev)
{
	struct net_dev_priv_exemple * priv = netdev_priv(net_dev);

	printk(KERN_INFO "%s: get_stats(%p)\n",
	       THIS_MODULE->name, net_dev);

	return & priv->net_dev_stats;
}


static void setup_exemple (struct net_device * net_dev)
{
	struct net_dev_priv_exemple * private = NULL;

	printk(KERN_INFO "%s: setup_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	/* Objet de base : ethernet */
	ether_setup(net_dev);

	/* Surcharge de certaines methodes */
	net_dev->open            = open_exemple;
	net_dev->stop            = stop_exemple;
	net_dev->hard_start_xmit = hard_start_xmit_exemple;
	net_dev->hard_header     = hard_header_exemple;
	net_dev->get_stats       = get_stats_exemple;
	
	/* Complements des champs de base */
	net_dev->flags    |= IFF_NOARP;
	net_dev->hard_header_cache = NULL;
	
	/* Initialisation des champs prives */
	private = netdev_priv(net_dev);
	memset(private, 0, sizeof(struct net_dev_priv_exemple));
}


static void __exit dechargement(void);

static int __init chargement(void)
{
	
	printk(KERN_INFO "%s: chargement\n", THIS_MODULE->name);

	net_dev_ex_0 = alloc_netdev(sizeof(struct net_dev_priv_exemple),
	                           "ex%d", setup_exemple);
	if (net_dev_ex_0 == NULL)
		return -ENOMEM;

	if (register_netdev(net_dev_ex_0) != 0) {
		dechargement();
		return -ENODEV;
	}

	net_dev_ex_1 = alloc_netdev(sizeof(struct net_dev_priv_exemple),
	              "ex%d", setup_exemple);
	if (net_dev_ex_1 == NULL) {
		dechargement();
		return -ENOMEM;
	}

	if (register_netdev(net_dev_ex_1) != 0) {
		dechargement();
		return -ENODEV;
	}
	return 0;
}


static void __exit dechargement(void)
{
	printk(KERN_INFO "%s: dechargement\n", THIS_MODULE->name);

	if (net_dev_ex_1 != NULL) {
		unregister_netdev(net_dev_ex_1);		
		free_netdev(net_dev_ex_1);
		net_dev_ex_1 = NULL;
	}

	if (net_dev_ex_0 != NULL) {
		unregister_netdev(net_dev_ex_0);
		free_netdev(net_dev_ex_0);
		net_dev_ex_0 = NULL;
	}
}

module_init(chargement)
module_exit(dechargement)
MODULE_LICENSE("GPL");

