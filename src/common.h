#ifndef COMMON_H
#define COMMON_H

#ifdef USE_COMMON
#include <limits.h>
#define CLI_OPTION_DEFAULT_SIZE 1024

#define DISCLAIMER "\
This is free software: you are free to \
change and redistribute it.\nThere is NO \
WARRANTY, to the extent permitted by law.\n"

extern char *program_name;

// void initialize_main (int argc, char **argv);

void set_program_name (char *argv);

#define nullptr NULL 

/* Factor out some of the common --help and --version processing code.  */

#define case_GETOPT_HELP_CHAR			\
  case 'h':			\
    usage (EXIT_SUCCESS);			\
    break;

/* Program_name must be a literal string.
   Usually it is just PROGRAM_NAME.  */
#define USAGE_BUILTIN_WARNING \
  _("\n" \
"Your shell may have its own version of %s, which usually supersedes\n" \
"the version described here.  Please refer to your shell's documentation\n" \
"for details about the options it supports.\n")

#define HELP_OPTION_DESCRIPTION \
  _("      --help        display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
  _("      --version     output version information and exit\n")

// #include "propername.h"
/* Define away proper_name, since it's not worth the cost of adding ~17KB to
   the x86_64 text size of every single program.  This avoids a 40%
   (almost ~2MB) increase in the file system space utilization for the set
   of the 100 binaries. */
// #define proper_name(x) proper_name_lite (x, x)
//
// #include "progname.h"

#define PACKAGE_NAME "ParityUtils"
#define Version "0.1.1"
#define case_GETOPT_VERSION_CHAR(Program_name, Authors)			\
  case 'V':						\
    fprintf (stdout, "%s (%s) %s beta\nCopyright (C) 2024 %s\n%s\n", Program_name, PACKAGE_NAME, Version, Authors, DISCLAIMER); \
    exit (EXIT_SUCCESS);						\
    break;

/* Use a macro rather than an inline function, as this references
   the global program_name, which causes dynamic linking issues
   in libstdbuf.so on some systems where unused functions
   are not removed by the linker.  */
#define emit_try_help() \
  do \
    { \
      fprintf (stderr, "Try '%s --help' for more information.\n", \
               program_name); \
    } \
  while (0)

#endif  // USE_COMMON
#endif  // COMMON_H
