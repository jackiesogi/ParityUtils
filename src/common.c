#include <string.h>
#include <stdio.h>
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

/* create file lock on POSIX system */
int
create_lock (char *name)
{
    FILE *file = fopen (name, "wx");
    if (file == NULL)
    {
        perror ("Failed to create lock file");
        return -1;
    }
    fprintf (file, "%d\n", getpid ());
    fclose (file);
    return 0;
}

/* release file lock on POSIX system */
void
release_lock (char *name)
{
    remove (name);
}
