#!/bin/bash

INCLUDE="-I${PWD}/include"

RANGE="1 5"

if [ -n "${1+set}" ]; then
  RANGE="$1 $1"
fi

if [ -n "${2+set}" ]; then
  RANGE="$1 $2"
fi

WARNINGS="-Wextra -Wall -Wshadow -Wstrict-aliasing"
FLAGS="-std=c++11 -march=native -fno-exceptions -fomit-frame-pointer ${WARNINGS} -O2 -pedantic -pipe"
# -fverbose-asm -save-temps -DNDEBUG

mkdir -p out

for i in `seq $RANGE`;
do
    COMMAND="g++ ${FLAGS} ${INCLUDE} src/day$i.cpp -o out/day$i"
    echo "$COMMAND"
    ${COMMAND}
done  
