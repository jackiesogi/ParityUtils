#!/bin/bash

CWD=$(dirname BASH_SOURCE)
PRG=$(basename BASH_SOURCE)

source $CWD/prereq.sh

echo "Checking requirements..."
check_platform

echo "Checking dependencies..."
check_dependencies

echo "Generating random binary file: A.bin (7 MiB)"
generate_random_binary_file 'A.bin' 7 

echo "Checking generated file: A.bin"
check_generate_random_binary_file 'A.bin' 7

echo "Generating random binary file: B.bin (8 MiB)"
generate_random_binary_file 'B.bin' 8

echo "Checking generated file: B.bin"
check_generate_random_binary_file 'B.bin' 8

echo "Generating random binary file: C.bin (9 MiB)"
generate_random_binary_file 'C.bin' 9

echo "Checking generated file: C.bin"
check_generate_random_binary_file 'C.bin' 9

echo "Generating random binary file: D.bin (10 MiB)"
generate_random_binary_file 'D.bin' 10

echo "Checking generated file: D.bin"
check_generate_random_binary_file 'D.bin' 10

echo "All files are generated successfully!"
