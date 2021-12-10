#!/bin/bash

WIDTH=1024
HEIGHT=1024
ROUNDS=1000

for n in {1..64}; do
export OMP_NUM_THREADS=$n
echo "Running (${WIDTH}x${HEIGHT}:${ROUNDS}) with ${OMP_NUM_THREADS} threads"
./heat-equation ${WIDTH} ${HEIGHT} ${ROUNDS}
done