#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <libintl.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef USE_COMMON
#include <config.h>
#include "system.h"
#include <version.h>
#include "progname.h"
#endif

#ifdef USE_GUI
#include "backup_gui.h"
#else
#define case_GUI_OPTION do {} while (0) // No-op
#define GUI_HELP ""
#endif

#include "common.h"
#include "backup.h"
#include "parity.h"
#include "metadata.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "backup"

// #define AUTHORS proper_name ("Shou-Chi Chen")
#define AUTHORS "Shou-Chi Chen"

void
print_gui_help ()
{
    printf("  -g, --show-gui               use GUI to process the inputs rather than CLI\n");
}

void
usage (int status)
{
    if (status != EXIT_SUCCESS)
    {
        emit_try_help ();
    }
    else
    {
        printf ("\
Usage: %s [OPTION]... SOURCE... -o DEST\n\
Create a parity file from SOURCE files and write to DEST.\n\
\n\
  -d, --directory=DIR          automatically use all files from a specified directory as input\n\
  -o, --output=FILE            specify the output parity file (e.g., /path/to/output/P.bin)\n\
  -a, --algorithm=ALGORITHM    specify the parity algorithm (xor_encode, seedsolomon_encode ...)\n\
  -f, --force                  overwrite the destination file if it exists\n\
  -n, --no-clobber             do not overwrite an existing file (overrides a previous -f option)\n\
  -v, --verbose                explain what is being done\n\
  -q, --quiet                  suppress output of informational messages\n", program_name);
#ifdef USE_GUI
        print_gui_help ();
#endif
    printf ("\
  -D, --dry-run                show what would be done without making any changes\n\
  -h, --help                   display this help and exit\n\
  -V, --version                output version information and exit\n\
\n\
Examples:\n\
  %s A.bin B.bin C.bin D.bin -o /path/to/output/P.bin\n\
  %s -d test_dir -o /path/to/output/P.bin\n\
  %s test_dir/* --force --output /path/to/output/P.bin\n\
", program_name, program_name, program_name);
    }

    exit (status);
}

static char const short_options[] = "d:o:a:fpnvqgDhV";

static struct option const long_options[] =
{
    {"directory", no_argument, NULL, 'd'},
    {"output", required_argument, NULL, 'o'},
    {"force", no_argument, NULL, 'f'},
    {"no-clobber", no_argument, NULL, 'n'},
    {"verbose", no_argument, NULL, 'v'},
    {"quiet", no_argument, NULL, 'q'},
    {"show-gui", no_argument, NULL, 'g'},
    {"dry-run", no_argument, NULL, 'D'},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {NULL, 0, NULL, 0}
};


void
new_inputs (struct backup_options *x, size_t n)
{
    x->input = (char **) malloc (n * sizeof (char *));
    x->input_count = n;
}

void
free_inputs (struct backup_options *x, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        free (x->input[i]);
    }
    free (x->input);
}

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
        char *output)
{
    struct backup_options *x = (struct backup_options *) malloc (sizeof(struct backup_options));
    // x->algorithm = (char *)malloc(CLI_OPTION_DEFAULT_SIZE + 1);
    // x->directory = (char *)malloc(CLI_OPTION_DEFAULT_SIZE + 1);
    // x->input = NULL;
    // x->output = (char *)malloc(CLI_OPTION_DEFAULT_SIZE + 1);

    x->algorithm = (algorithm != NULL) ? strdup(algorithm) : NULL;
    x->directory = (directory != NULL) ? strdup(directory) : NULL;
    x->force = force;
    x->no_clobber = no_clobber;
    x->verbose = verbose;
    x->quiet = quiet;
    x->dry_run = dry_run;
    x->input = input;
    x->input_count = input_count;
    x->output = (output != NULL) ? strdup(output) : NULL;
    return x;
}

void
free_backup_options (struct backup_options *x)
{
    free (x->algorithm);
    free (x->directory);
    free_inputs (x, x->input_count);
    free (x->output);
    free (x);
}

void
display_backup_options (struct backup_options *x)
{
    printf ("Algorithm: %s\n", x->algorithm);
    printf ("Directory: %s\n", x->directory);
    printf ("Output: %s\n", x->output);
    printf ("Force: %s\n", x->force ? "true" : "false");
    printf ("No clobber: %s\n", x->no_clobber ? "true" : "false");
    printf ("Verbose: %s\n", x->verbose ? "true" : "false");
    printf ("Quiet: %s\n", x->quiet ? "true" : "false");
    printf ("Dry run: %s\n", x->dry_run ? "true" : "false");
    printf ("Input files:\n");
    for (int i = 0; i < x->input_count; ++i)
    {
        printf ("  %s", x->input[i]);
    }
    putchar('\n');
}

int
backup_internal (struct backup_options *x)
{
    /* Check if there is a lock file */
    char lock_file[PATH_MAX];
    snprintf(lock_file, sizeof(lock_file), ".~lock.%s#", x->output);

    /* If the lock file exists, print its content (the pid that lock this file) */
    if (access(lock_file, F_OK) != -1) 
    {
        FILE *file = fopen(lock_file, "r");
        if (file == NULL) 
        {
            perror("Failed to open lock file");
            return 1;
        }
        int pid;
        fscanf(file, "%d", &pid);
        fclose(file);
        printf("File '%s' is currently locked by process %d. Continue? [y/N]: ", x->output, pid);
        char response = getchar();
        if (response != 'n' && response != 'N') 
        {
            printf("Aborted by user.\n");
            return 1;
        }
        while (getchar() != '\n');
    }

    /* Check if the output file exists and is non-empty */
    if (!x->force) 
    {
        struct stat st;
        if (stat(x->output, &st) == 0 && st.st_size > 0) 
        {
            printf("File '%s' already exists and is not empty. Overwrite? [y/N]: ", x->output);
            char response = getchar();
            if (response != 'y' && response != 'Y') 
            {
                printf("Aborted by user.\n");
                return 1;  // Abort operation
            }
            while (getchar() != '\n');
        }
    }

    parity_options options;
    options.input_files = x->input;
    options.file_count = x->input_count;
    options.output_file = x->output;
    options.algorithm = strtofptr (x->algorithm);  // Convert algorithm name to function pointer

    if (!options.algorithm) 
    {
        options.algorithm = DEFAULT_XOR_ENCODE;  // Default to XOR encoding
    }

    /* Create the lock file */
    create_lock (lock_file);

    options.algorithm (options.input_files, options.file_count, options.output_file);

    /* Release the lock file */
    release_lock (lock_file);

    return 0;
}

int
main (int argc, char **argv)
{
    int optc;
    int ret;
    size_t filecount;

    /* Set default values */
    struct backup_options *x = new_backup_options(NULL,
            NULL, false, true, false, true, false, NULL, 0, NULL);

    // TODO: align these functions with GNU ones
    set_program_name (argv[0]);

    setlocale (LC_ALL, "");
    // bindtextdomain (PACKAGE, LOCALEDIR);
    // textdomain (PACKAGE);
    // atexit (close_stdout);
    
    // Parse the arguments and save it to `backup_options x`
    while ((optc = getopt_long (argc, argv, short_options, long_options, NULL))
            != -1)
    {
        switch (optc)
        {
            case 'a':
                x->algorithm = optarg;
                break;
            case 'o':
                x->output = optarg;
                break;
            case 'f':
                x->force = true;
                break;
            case 'n':
                x->no_clobber = true;
                break;
            case 'v':
                x->verbose = true;
                break;
            case 'q':
                x->quiet = true;
                break;
            case 'd':
/*                 fprintf (stderr, "'--directory' option is not stable now, please \ */
/* specify the input files by your own!\n"); */
/*                 return EXIT_FAILURE; */
                x->directory = optarg;
                break;
            case 'D':
                x->dry_run = true;
                break;

            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);

#ifdef USE_GUI
            case_GUI_OPTION;
#endif
            
            default:
                usage (EXIT_FAILURE);
        }
    }

    /* If non-option arguments is not enough and -d option is not set
     * meaning that possibly there is no input files given */
    if (!x->directory && optind >= argc)
    {
        fprintf (stderr, "%s: missing file operand\n", program_name);
        usage (EXIT_FAILURE);
    }

    if (x->directory)
    {
        filecount = get_filecount_from_dir (x->directory);
        new_inputs (x, filecount);
        get_files_from_dir (x->directory, x->input, filecount);
    }
    else
    {
        new_inputs (x, argc - optind);

        /* store the input files ito char **input_files */
        for (int i = optind; i < argc; ++i)
        {
           x->input[i - optind] = argv[i];
        }
    }

    /* Show options when --verbose is set */
    if (x->verbose || !x->quiet)
    {
        display_backup_options (x);
    }

    /* Call the internal function to generate the parity file */
    ret = backup_internal (x);

    if (ret == 0)
    {
        printf ("Parity file '%s' is created successfully.\n", x->output);
    }

    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
