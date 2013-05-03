#! /bin/sh

DEVICE=/dev/velleman0


while true; do
		analog1=$(($RANDOM & 0xFF))
		analog2=$(($RANDOM & 0xFF))
		digital=$(($RANDOM & 0xFF))
		echo 5 $digital $analog1 $analog2 0 0 0 0 > $DEVICE
		usleep 50000
done
