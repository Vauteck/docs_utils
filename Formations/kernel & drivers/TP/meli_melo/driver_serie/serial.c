#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/serial_reg.h>
#include <linux/version.h>
#include <asm/termbits.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

MODULE_DESCRIPTION("A simple serial driver");
MODULE_LICENSE("GPL");

int irq = 4;
int port = 0x3f8;
int bps = 9600;

static int k_getc(void)
{
	if (inb(port + UART_LSR) & UART_LSR_DR)
		return (inb(port + UART_RX));
	return (-1);
}

static void k_putc(int chr)
{
	while (!(inb(port + UART_LSR) & UART_LSR_THRE)) ;
	outb(chr, port + UART_TX);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
static irqreturn_t k_interrupt(int irq, void *dev_id)
#else
static irqreturn_t k_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	int chr;
	int iir;

	do {
		chr = k_getc();
		iir = inb(port + UART_IIR);

		printk("Received %c\n", chr);

		k_putc(chr);
		k_putc(chr);
	} while (iir & UART_IIR_RDI) ;
	return IRQ_HANDLED;
}

int __init k_init(void)
{
	unsigned cval;
	int cflag = CREAD | HUPCL | CLOCAL | B9600 | CS8;
	int quot = 115200 / bps;

	cval = cflag & (CSIZE | CSTOPB);
	cval >>= 4;
	if (cflag & PARENB)
		cval |= UART_LCR_PARITY;
	if (!(cflag & PARODD))
		cval |= UART_LCR_EPAR;

	/*
	 *      Disable UART interrupts, set DTR and RTS high
	 *      and set speed.
	 */
	cval = 0x3;
	outb(cval | UART_LCR_DLAB, port + UART_LCR);	/* set DLAB */
	outb(quot & 0xff, port + UART_DLL);		/* LS of divisor */
	outb(quot >> 8, port + UART_DLM);		/* MS of divisor */
	outb(cval, port + UART_LCR);			/* reset DLAB */
	outb(UART_IER_RDI, port + UART_IER);		/* turn on interrupts */
	outb(UART_MCR_OUT2 | UART_MCR_DTR | UART_MCR_RTS, port + UART_MCR);

	/*
	 *      If we read 0xff from the LSR, there is no UART here.
	 */
	if (inb(port + UART_LSR) == 0xff)
		return -ENODEV;

	printk("Found a serial port\n");
	if (request_irq(irq, k_interrupt, SA_INTERRUPT, "my-serial", NULL) < 0)
		return -EIO;
	return 0;
}

void __exit k_exit(void)
{
	outb(0, port + UART_IER);		/* turn off interrupts */
	free_irq(irq, NULL);
}

module_init(k_init);
module_exit(k_exit);
