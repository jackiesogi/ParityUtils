#ifndef COMMON_H
#define COMMON_H

#define CLI_OPTION_DEFAULT_SIZE 1024
#define DISCLAIMER "\
This is free software: you are free to \
change and redistribute it.\nThere is NO \
WARRANTY, to the extent permitted by law.\n"

extern char *program_name;

void initialize_main (int argc, char **argv);

void set_program_name (char *argv);
    
#endif  // COMMON_H
