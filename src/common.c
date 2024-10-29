#include <string.h>
#include "common.h"

char *program_name = NULL;

/* strip the program name from the actual path */
void
set_program_name (char *argv)
{
    program_name = argv;

    char *p = strrchr (program_name, '/');

    if (p)
    {
        program_name = p + 1;
    }
}
