#ifndef RESTORE_H
#define RESTORE_H

#include "parity.h"

struct restore_options
{
    char *algorithm;
    char *directory;
    bool force;
    bool no_clobber;
    bool verbose;
    bool quiet;
    bool dry_run;
    char **input;
    int  input_count;
    char *output;
};

struct restore_options *
new_restore_options (
        char *algorithm,
        char *directory,
        bool force,
        bool no_clobber,
        bool verbose,
        bool quiet,
        bool dry_run,
        char **input,
        int  input_count,
        char *output);

#endif  // RESTORE_H
