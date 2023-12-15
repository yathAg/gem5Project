#!/bin/bash

# Define variables
SCONS_COMMAND="scons-3 USE_HDF5=0 -j $(nproc)"
BUILD_PATH="./build/ECE565-X86/gem5.opt"

# Execute the command
$SCONS_COMMAND $BUILD_PATH