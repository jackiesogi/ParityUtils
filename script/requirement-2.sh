#!/bin/bash

CWD=$(dirname $BASH_SOURCE)
SRC="$CWD/../src"
PATH=$PATH:$SRC

backup --verbose A.bin B.bin C.bin D.bin -o P.bin
