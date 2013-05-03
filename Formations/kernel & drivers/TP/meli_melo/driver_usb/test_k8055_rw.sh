#! /bin/sh

DEVICE=/dev/velleman0

while read digital_in unknow analog1_in analog2_in rest
do
	analog1_out=$((255 & (analog1_in * 10)))
	analog2_out=$((analog1_out))
#	digital_out=$((digital_in))
	digital_out=$((analog1_in))
	echo 5 $digital_out $analog1_out $analog2_out 0 0 0 0 >&0
done <> $DEVICE

