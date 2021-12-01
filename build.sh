#!/bin/bash

INCLUDE="-I${PWD}/include"

RANGE="1 1"

if [ -n "${1+set}" ]; then
  RANGE="$1 $1"
fi

FLAGS="-std=c++11 -march=native -fno-exceptions -Wextra -Wall -Wshadow -O2 -pedantic -pipe"
# -fverbose-asm -save-temps -DNDEBUG

mkdir -p out

for i in `seq $RANGE`;
do
    g++ ${FLAGS} ${INCLUDE} src/day$i.cpp -o out/day$i
done  
