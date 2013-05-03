#/* **************** "":1.0 s_33/lab1_testloop.sh **************** */
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

# 2/2008 J. Cooperstein (coop@linuxfoundation.org) License:GPLv2

file="/dev/mycdrv" && [ "$1" != "" ] && file=$1
reps=10 && [ "$2" != "" ] && reps=$2

echo DOING $reps iterations on $file with ../lab1_aio_test:
time  ( n=0 ; \
while [ $n -lt $reps ]
  do ./lab1_aio_test $file > /dev/null
  n=$(($n+1))
  sync
done )

echo DOING $reps iterations on $file with ../lab1_posix_test:
time  ( n=0; \
while [ $n -lt $reps ]
  do ./lab1_posix_test $file > /dev/null
  n=$(($n+1))
  sync
done )
