#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <compression_type>"
    exit 1
fi

compression_type=$1

# Array of benchmarks
benchmarks=("astar")

# Output file for storing miss rates
miss_rate_file="miss_rates_${compression_type}.txt"

# Remove the existing miss rate file
rm -f "$miss_rate_file"

# Loop through each benchmark
for bench in "${benchmarks[@]}"
do
    # Loop through l1i_size values
    for l1i_size in  "64kB"
    do
        # Loop through l2_size values
        for l2_size in "256kB" 
        do
            # Build the output filename
            output_file="${bench}_${compression_type}_${l1i_size}_${l2_size}.txt"
            result_file="m5out/${bench}_${compression_type}_${l1i_size}_${l2_size}.txt"

            # Run the command with the current benchmark and parameters
            ./build/ECE565-X86/gem5.opt --stats-file="$output_file" \
            configs/spec/spec_se.py --cpu-type=O3CPU --maxinsts=10000000 \
            --l1i_size="$l1i_size" --caches --l2cache --l2_size="$l2_size" --cacheline_size 64 \
            -b "$bench" 

            echo "Finished running $bench with $compression_type, l1i_size=$l1i_size, l2_size=$l2_size, results saved in $output_file"

            # Extract the line starting with "system.l2.overallMissRate::total" and save it
            miss_rate=$(grep "system.l2.overallMissRate::total" "$result_file")
            echo "$bench: $miss_rate, l1i_size=$l1i_size, l2_size=$l2_size" >> "$miss_rate_file"
        done
    done
done

echo "Miss rates saved in $miss_rate_file"
