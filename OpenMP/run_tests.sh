#!/bin/bash

THREADS="1 2 4 8"
INPUTS="4 5 6 7 8 9 10"

for input in $INPUTS; do
    echo "=============================="
    echo "Input: energy${input}"
    echo "=============================="
    for p in $THREADS; do
        echo "--- Threads: $p ---"
        ./route -p $p -i input/energy${input}
        echo ""
    done
done