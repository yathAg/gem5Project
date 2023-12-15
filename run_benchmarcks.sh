#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <compression_type>"
    exit 1
fi

compression_type=$1

# Array of benchmarks
benchmarks=("fotonik3d_s" "cactuBSSN_s" "imagick_s" "wrf_s" "xalancbmk_s" "x264_s" "exchange2_s" "gcc_s" "mcf_s" "astar")
# You can uncomment one of the other benchmark lists based on your needs.

# Output file for storing miss rates
miss_rate_file="miss_rates_${compression_type}.txt"

# Remove the existing miss rate file
rm -f "$miss_rate_file"

# Loop through each benchmark
for bench in "${benchmarks[@]}"
do
    # Build the output filename
    output_file="${bench}_${compression_type}.txt"
    result_file="m5out/${bench}_${compression_type}.txt"

    # Run the command with the current benchmark and compression type
    ./build/ECE565-X86/gem5.opt --stats-file="$output_file" \
    configs/spec/spec_se.py --cpu-type=O3CPU -F 10000000 --maxinsts=10000000 \
    --l1i_size=64kB --caches --l2cache --l2_size=1MB --cacheline_size 128 \
    -b "$bench" 

    echo "Finished running $bench with $compression_type, results saved in $output_file"

    # Extract the line starting with "system.l2.overallMissRate::total" and save it
    miss_rate=$(grep "system.l2.overallMissRate::total" "$result_file")
    echo "$bench: $miss_rate" >> "$miss_rate_file"
done

echo "Miss rates saved in $miss_rate_file"
