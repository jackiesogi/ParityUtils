#!/bin/bash

CWD=$(dirname $BASH_SOURCE)
BIN="$CWD/../bin"
PATH=$PATH:$BIN

backup --verbose A.bin B.bin C.bin D.bin -o P.bin
