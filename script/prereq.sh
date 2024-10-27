### Script Name : prereq.sh (Prerequisites for this project)
### Author Name : Jackie Chen (jck.tux@proton.me)
### Description : This script generates four binary files that are required for further processing.

#!/bin/bash

# Exit on error
set -e  

# Variables
RANDOM_SOURCE=/dev/urandom
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(dirname "$0")


# Check if the platform is supported
function check_platform() {
    if [[ "$OSTYPE" != "linux-gnu" ]]; then
        echo "Warning: This script is only tested on Linux, it may not work on other platforms."
    fi
}

# Check if the required dependencies are installed
function check_dependencies() {
    if ! command -v dd &> /dev/null; then
        echo "Error: dd is not installed."
        exit 1
    fi
}

# Generate random binary files with given size and name
function generate_random_binary_file() {
    local name=$1  # Name of the file
    local size=$2  # Size of the file
    echo "Generating $name with size $size MiB..."
    dd if=/dev/urandom of="$name" bs=$((1024 * 1024)) count="$size"
}

# Test function for generate_random_binary_file()
function test_generate_random_binary_file() {
    generate_random_binary_file 5 test.bin
    if [[ ! -f test.bin ]]; then
        echo "Error: test.bin is not generated."
        exit 1
    fi
    file_size=$(wc -c < "$file_name")
    if [[ $file_size -ne 5 ]]; then
        echo "Error: test.bin size is not correct."
        exit 1
    fi
    rm test.bin
}

# Get file size in MiB
function get_file_size() {
    local file=$1  # Name of the file
    local byte=$(wc -c < "$file" | awk '{print $1}')
    echo $((byte / 1024 / 1024))
}

# Check if the file is generated and size is correct after generating
function check_generate_random_binary_file() {
    local name=$1  # Name of the file
    local size=$2  # Size of the file
    if [[ ! -f $name || $(get_file_size "$name") -ne $size ]]; then
        echo "Error: $name is not generated correctly."
        exit 1
    fi
}
