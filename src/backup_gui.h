#ifndef BACKUP_GUI_H
#define BACKUP_GUI_H

#include <stdbool.h>
#include "backup.h"  // To access `backup_options`

// Runs the GUI, accepting the backup options from the command line
bool
run_gui (struct backup_options *x);

#ifdef USE_GUI
#define case_GUI_OPTION \
            case 'g': \
                if (!run_gui(x)) \
                { \
                    fprintf(stderr, "%s: failed to run GUI\n", program_name); \
                    return EXIT_FAILURE; \
                } \
                return EXIT_SUCCESS;

#define GUI_HELP "printf(\"   -g, --show-gui                  use GUI to process the inputs rather than CLI\n\")"
#else
#define case_GUI_OPTION
#endif

#endif // BACKUP_GUI_H
