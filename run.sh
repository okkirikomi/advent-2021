#!/bin/bash

RANGE="1 1"

if [ -n "${1+set}" ]; then
  RANGE="$1 $1"
fi

for i in `seq $RANGE`;
do
    echo "Running day$i"
    out/day$i input/day$i
    echo ""
done  
