#!/bin/bash

make clean
make

LIMIT=$1 
echo "running $LIMIT instances"

for((a=1; a <= LIMIT; a++))
do
  ./client &
done
