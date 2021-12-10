#!/bin/bash

WIDTH=1000
HEIGHT=1000
ROUND_BASE=10

for n in {1..64}; do
ROUNDS=$(($n * ${ROUND_BASE}))
export OMP_NUM_THREADS=$n
echo "Running (${WIDTH}x${HEIGHT}:${ROUNDS}) with ${OMP_NUM_THREADS} threads"
./heat-equation ${WIDTH} ${HEIGHT} ${ROUNDS}
done
