#!/bin/sh

if [ $# -lt 1 ]; then
	echo "usage: $0 <busnum>"
	exit 1
fi

if [ ! -c /dev/i2c-$1 ]; then
	echo "invalid bus number"
	exit 2
fi

for i in `seq 1 63`; do
	printf "%x\n" $i
	i2c-ctl /dev/i2c-$1 rb `printf "%x" $i` 0 1
done
