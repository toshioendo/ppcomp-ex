#!/bin/sh
if [ $# -lt 4 ]; then
    echo "Options required: initial-core-id, core-stride, #proces, command (args)"
    echo "$0 0 8 3 ./mm 2000 2000 2000"
    exit 0
fi

command="$4 $5 $6 $7 $8 $9"

# initial core id
START=$1
# stride
STRIDE=$2
# # of processes
N=$3

LAST=`expr $START + $N \* $STRIDE - $STRIDE`

trap "kill 0" EXIT
set -x

for C in `seq $START $STRIDE $LAST`; do
    numactl -C $C $command &
done
wait




