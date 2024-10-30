#!/bin/bash

CWD=$(dirname $BASH_SOURCE)
PATH=$CWD:$PATH

echo ""
echo "===================================== Requirement 3 ====================================="
echo ""
echo "Delete any one of the files generated in step 1."
echo ""
echo "========================================================================================="
echo ""

echo ""
echo "We will rename the 'A.bin' to 'A.bin.original' for simulating file loss while at the same"
echo "time, preserving the original file for future checking (with the restored file)."
echo ""

echo "RUN: cp A.bin A.bin.original"
cp A.bin A.bin.original

echo "RUN: rm -f A.bin"
rm -f A.bin

echo ""
