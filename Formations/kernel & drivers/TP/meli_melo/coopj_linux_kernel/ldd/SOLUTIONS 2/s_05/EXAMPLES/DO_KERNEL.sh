#/* **************** LDD:1.0 s_05/DO_KERNEL.sh **************** */
#/*
# * The code herein is: Copyright Jerry Cooperstein, 2009
# *
# * This Copyright is retained for the purpose of protecting free
# * redistribution of source.
# *
# *     URL:    http://www.coopj.com
# *     email:  coop@coopj.com
# *
# * The primary maintainer for this code is Jerry Cooperstein
# * The CONTRIBUTORS file (distributed with this
# * file) lists those known to have contributed to the source.
# *
# * This code is distributed under Version 2 of the GNU General Public
# * License, which you should have received with the source.
# *
# */
#!/bin/bash

# Script to compile and install the Linux kernel and modules.
# written by Jerry Cooperstein (2002-2009)
# Copyright GPL blah blah blah

function get_SOURCE(){
    KERNEL=$1
    TARFILE=$2
    CONFIGFILE=$3
    LKV=linux-$KERNEL
    KCONFIG=$LKV/.config
    [ ! -f "$TARFILE" ] && echo no  "$TARFILE, aborting" && exit
    [ ! -f "$CONFIGFILE" ] && echo no "$CONFIGFILE, aborting" && exit
    echo -e "\nbuilding: Linux $KERNEL kernel from $TARFILE
               Using: $CONFIGFILE as the configuration file\n"
    set -x
# Decompress kernel, assumed in bz2 form
    tar jxf $TARFILE
# Copy over the .config file
    cp $CONFIGFILE $KCONFIG
# Change to the main source directory
    cd $LKV
    set +x
}

# determine which distribution we are on
function get_SYSTEM(){
    [ "$SYSTEM" != "" ] && return
    SYSTEM=
    [ "$(grep -i Red\ Hat /proc/version)" != "" ] && SYSTEM=REDHAT && return
    [ "$(grep -i Ubuntu   /proc/version)" != "" ] && SYSTEM=UBUNTU && return
    [ "$(grep -i debian   /proc/version)" != "" ] && SYSTEM=DEBIAN && return
    [ "$(grep -i suse     /proc/version)" != "" ] && SYSTEM=SUSE   && return
    [ "$(grep -i gentoo   /proc/version)" != "" ] && SYSTEM=GENTOO && return
}

# find out what kernel version this is
function get_KERNELVERSION(){
    for FIELD in VERSION PATCHLEVEL SUBLEVEL EXTRAVERSION ; do
	eval $(sed -ne "/^$FIELD/s/ //gp" Makefile)
    done
# is there a local version file?
    [ -f ./localversion-tip ] && \
	EXTRAVERSION="$EXTRAVERSION$(cat localversion-tip)"
    KERNEL=$VERSION.$PATCHLEVEL.$SUBLEVEL$EXTRAVERSION
}

# determine where to place vmlinuz, System.map, initrd image, config file
function get_BOOT(){
    if [ "$BOOT" == "" ] ; then
	PLAT=$(uname -i)
	BOOT=/boot
	[ "$PLAT" == "i386" ]   && [ -d /boot/32 ] && BOOT=/boot/32
	[ "$PLAT" == "x86_64" ] && [ -d /boot/64 ] && BOOT=/boot/64
    fi
}

# parallelize, speed up for multiple CPU's
function get_MAKE(){
    NCPUS=$(grep ^processor /proc/cpuinfo | wc -l)
    MAKE="make -j $(( $NCPUS * 2))"
}

# if it is a NVIDIA'able  kernel, compile nvidia.ko
function  dealwith_NVIDIA(){
     [ "$(lsmod | grep nvidia)" ] && \
     [ "$(which --skip-alias dealwith_nvidia)" ] && \
	 dealwith_nvidia $KERNEL
}

function makeinitrd_REDHAT(){
# Construct mkinitrd image
# RHEL5.3 2.6.18 kernels mucked this up
    if [ "$(grep CONFIG_MD_RAID45 ./.config)" != "" ] \
	|| [ "$(grep without-dmraid /sbin/mkinitrd)" == "" ] ; then
	mkinitrd -v -f             $BOOT/initrd-$KERNEL.img $KERNEL 
    else
	mkinitrd -v -f --without-dmraid   $BOOT/initrd-$KERNEL.img $KERNEL 
    fi
# Update /boot/grub/grub.conf
    cp /boot/grub/grub.conf /boot/grub/grub.conf.BACKUP
    grubby --copy-default  \
	--make-default \
	--remove-kernel=$BOOT/vmlinuz-$KERNEL \
	--add-kernel=$BOOT/vmlinuz-$KERNEL \
	--initrd=$BOOT/initrd-$KERNEL.img \
	--title=$BOOT/$KERNEL
# if it is a NVIDIA'able  kernel, compile nvidia.ko
    dealwith_NVIDIA 
}

function makeinitrd_DEBIAN(){
    make install
    if [ -f $BOOT/initrd.img-$KERNEL ] ; then
	update-initramfs -u -k $KERNEL
    else
	update-initramfs -c -k $KERNEL
    fi
    update-grub
}

function makeinitrd_UBUNTU(){
    makeinitrd_DEBIAN
}

function makeinitrd_SUSE(){
    make install
}

function makeinitrd_GENTOO(){
    make install
    genkernel 	ramdisk --kerneldir=$PWD   
}
function makeinitrd_(){
    echo System $SYSTEM is not something I understand, can not finish
    exit
}

##########################################################################
# Start of the work

NARGS="$#"
[ "$NARGS" == "3" ]  && get_SOURCE $1 $2 $3

get_KERNELVERSION
get_BOOT
get_SYSTEM
get_MAKE

echo building: Linux $KERNEL kernel,  and placing in: $BOOT on a $SYSTEM system

# set shell to abort on any failure and echo commands
set -e -x

# Do the main compilation work, kernel and modules
$MAKE

# Install the modules
$MAKE modules_install 

# Install the compressed kernel, System.map file, config file, 
cp arch/x86/boot/bzImage   $BOOT/vmlinuz-$KERNEL 
cp System.map              $BOOT/System.map-$KERNEL 
cp .config                 $BOOT/config-$KERNEL 

# making initrd and updating grub is very distribution dependendent:

echo I am building the initrd image and modifying grub config on $SYSTEM

makeinitrd_"$SYSTEM"
