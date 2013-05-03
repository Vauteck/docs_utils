#/* **************** CLDD:1.0 s_04/lab4_loadit.sh **************** */
#/*
# * The code herein is: Copyright 1998-2009 by Jerry Cooperstein
# *
# * This Copyright is retained for the purpose of protecting free
# * redistribution of source.
# *
# * URL:    http://www.coopj.com
# * email:  coop@coopj.com 
# *
# * The primary maintainer for this code is Jerry Cooperstein
# * The CONTRIBUTORS file (distributed with this
# * file) lists those known to have contributed to the source.
# *
# * This code is distributed under Version 2 of the GNU General Public
# * License, which you should have received with the source.
# *
# */
#
#!/bin/sh

# 1/2005 J. Cooperstein (coop@coopj.com) License:GPLv2

module=lab3_dynamic.ko
node=/dev/mycdrv
minor=0

[ "$1" != "" ] && module="$1"
[ "$2" != "" ] && node="$2"

echo loading $module
insmod $module

major=$(awk "\$2==\"mycdrv\" {print \$1}" /proc/devices)
echo major number is: $major

echo creating the device node at $node with minor number=0

mknod $node c $major $minor






