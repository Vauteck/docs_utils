ifneq ($(KERNELRELEASE),)

obj-m := char.o

ifeq ($(PATCHLEVEL),4)
include $(TOPDIR)/Rules.make
endif

else

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	$(CROSS_COMPILE)gcc -O2 -Wall test_ioctl.c -o test_ioctl

endif

clean:
	rm -rf *.[oas] .*.flags *.ko .*.cmd .*.d .*.tmp *.mod.c .tmp_versions Module*.symvers
	rm -f ioctl
