#!/bin/bash

# Iterate over the ranges
for h in $(seq 1000 200 10000); do
    for w in $(seq 1000 200 10000); do
        echo "Running command with -h $h -w $w"
        sudo ./app --json ../fractal/fractal_simple -h "$h" -w "$w"
    done
done

echo "All commands executed."
