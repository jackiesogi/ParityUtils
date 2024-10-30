#!/bin/bash

CWD=$(dirname $BASH_SOURCE)
BIN="$CWD/../bin"
PATH=$PATH:$BIN

echo ""
echo "===================================== Requirement 2 ====================================="
echo ""
echo "Develop a program in C/C++, named “backup”, to create a parity file (P.bin) from A.bin,"
echo "B.bin, C.bin, and D.bin. Please design the program arguments/options to take the input"
echo "and output files."
echo ""
echo "========================================================================================="
echo ""

echo "The 'backup' program will be available after using 'make' command to build this project,"
echo "or if you are on x86_64 system, it will use the pre-built binaries inside 'bin/' directory"
echo "but it's better to build it yourself because you might encounter shared library issues."

echo ""
echo "RUN: backup --verbose A.bin B.bin C.bin D.bin --output P.bin"
backup --verbose A.bin B.bin C.bin D.bin --output P.bin
echo ""
