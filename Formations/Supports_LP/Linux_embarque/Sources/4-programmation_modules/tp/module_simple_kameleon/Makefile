obj-m += module_simple.o

all:
        make -C /usr/src/linux-2.6.30/headers SUBDIRS=`pwd` modules
clean: 
        make -C /opt/eldk_arm/arm/usr/src/linux M=`pwd` clean

