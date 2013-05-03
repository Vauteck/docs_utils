#! /bin/sh

DEVICE=/dev/velleman0

#stty -raw
while read a
do
	a=$((2#$a))
		echo 5 $a $a $a  0 0 0 0 > $DEVICE 
done
