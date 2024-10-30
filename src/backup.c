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
#include <dirent.h>

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
", program_name);
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

/* This function loop through the directory and save all filename and store them 
 * into input_files and also update file_count. Notice that this function is 
 * platform specific and only support POSIX system. */
size_t
get_files_from_dir (const char *dir, char **input_files)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    int count = 0;

    if ((dp = opendir(dir)) == NULL) 
    {
        perror("Failed to open directory");
        return 0;
    }

    /* Loop over each entry in the directory */
    while ((entry = readdir(dp)) != NULL) 
    {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (stat(path, &statbuf) == -1) 
        {
            perror("Failed to get file status");
            continue;
        }

        // Regular file
        if (S_ISREG(statbuf.st_mode)) 
        {
            input_files[count] = (char *) malloc (strlen (path) + 1);
            if (input_files[count] == NULL) 
            {
                perror("Failed to allocate memory for file path");
                closedir(dp);
                return 0;
            }
            strcpy(input_files[count], path);
            count++;
        }
    }

    closedir(dp);

    return count;
}

void
new_inputs (struct backup_options *x, size_t n)
{
    if (x->directory)
    {
        x->input_count = get_files_from_dir (x->directory, x->input);
        return;
    }
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

    x->algorithm = algorithm;
    x->directory = directory;
    x->force = force;
    x->no_clobber = no_clobber;
    x->verbose = verbose;
    x->quiet = quiet;
    x->dry_run = dry_run;
    x->input = input;
    x->input_count = input_count;
    x->output = output;
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
        printf ("  %s\n", x->input[i]);
    }
}

int
backup_internal (struct backup_options *x)
{
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
    options.algorithm (options.input_files, options.file_count, options.output_file);

    return 0;
}

int
main (int argc, char **argv)
{
    int optc;

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
                fprintf (stderr, "'--directory' option is not stable now, please \
specify the input files by your own!\n");
                return EXIT_FAILURE;
                /* x->directory = optarg; */
                /* break; */
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

    new_inputs (x, argc - optind);

    /* store the input files ito char **input_files */
    for (int i = optind; i < argc; ++i)
    {
        x->input[i - optind] = argv[i];
    }

    /* Show options when --verbose is set */
    if (x->verbose || !x->quiet)
    {
        display_backup_options (x);
    }

    return backup_internal (x) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
