
obj-m:= drv_param.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	file drv_param.ko

install:
	make -C $(KDIR) M=$(PWD) modules_install
	depmod -a

info:
	modinfo drv_param.ko

clean:
	make -C $(KDIR) M=$(PWD) clean

