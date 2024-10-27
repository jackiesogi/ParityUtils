#!/bin/bash

CWD=$(dirname $BASH_SOURCE)
PATH=$CWD:$PATH

cp A.bin A.bin.original

rm -f A.bin
