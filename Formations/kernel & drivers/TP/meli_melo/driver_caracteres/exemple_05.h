/************************************************************************\

  Fonction d'ioctl permettant d'afficher ou non le PPID

\************************************************************************/

#ifndef EXEMPLE_05_H
#define EXEMPLE_05_H
	#include <asm/ioctl.h>
	
	#define TYPE_IOCTL_EXEMPLE	'x'
	
	#define EX_GET_PPID  1
	#define EX_SET_PPID  2
	
	#define EX_IOCGPPID _IOR(TYPE_IOCTL_EXEMPLE, EX_GET_PPID, int)
	#define EX_IOCSPPID _IOW(TYPE_IOCTL_EXEMPLE, EX_SET_PPID, int)
	
	
#endif
