#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage: $0 <busnum>"
	exit 1
fi

if [ ! -e /sys/module/i2c_dev ]; then
	echo "load the i2c_dev driver first"
	exit 2
fi

if [ ! -c /dev/i2c-$1 ]; then
	echo "invalid bus number"
	exit 3
fi

for i in `seq 1 63`; do
	i2c-ctl /dev/i2c-$1 rb `printf "%x" $i` 0 1 &>/dev/null
	if [ "$?" = "0" ]; then
		printf "device at 0x%x\n" $i
	fi
done
