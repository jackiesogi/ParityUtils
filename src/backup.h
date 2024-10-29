#ifndef BACKUP_H
#define BACKUP_H

#include "parity.h"

struct backup_options
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

struct backup_options *
new_backup_options (
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

#endif  // BACKUP_H
