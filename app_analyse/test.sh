#!/bin/bash

# Iterate over the ranges
for h in $(seq 4500 500 10000); do
    for w in $(seq 4500 500 10000); do
        echo "Running command with -h $h -w $w"
        sudo ./app --json ../fractal/fractal -h "$h" -w "$w" -f
    done
done

echo "All commands executed."
