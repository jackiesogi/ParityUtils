### Script Name : clean.sh (Clean up the binary files)
### Author Name : Jackie Chen (jck.tux@proton.me)
### Description : This script remove binary files that were created by `prereq.sh`.

#!/bin/bash

# Remove the binary files
# rm -rf *.bin

# Remove the files generated by autotools
# rm -f configure
# rm -f Makefile.in
# rm -f aclocal.m4
# rm -rf autom4te.cache/
# rm -f config.guess config.sub install-sh missing depcomp compile config.status
# rm -f config.log
# rm -f Makefile