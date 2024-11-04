#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include "metadata.h"

/* Allocates a new file_metadata struct */
struct file_metadata *
new_file_metadata ()
{
    struct file_metadata *x = (struct file_metadata *) malloc (sizeof (struct file_metadata));
    x->path = NULL;
    x->size = 0;
    x->mtime = 0;
    // x->sha256 = (unsigned char *)malloc (SHA256_DIGEST_LENGTH);
    return x;
}

void
free_file_metadata (struct file_metadata *x)
{
    free (x->path);
    // free (x->sha256);
    free (x);
}

/* Calculate SHA-256 hash of the file */
void
calculate_sha256 (FILE *file, unsigned char output[SHA256_DIGEST_LENGTH])
{
    if (!file) return;

    SHA256_CTX sha256;
    SHA256_Init (&sha256);
    unsigned char buffer[4096];
    int bytes_read;
    while ((bytes_read = fread (buffer, 1, sizeof (buffer), file)) > 0) {
        SHA256_Update (&sha256, buffer, bytes_read);
    }
    SHA256_Final (output, &sha256);
    fclose (file);
}

/* Get file's size */
size_t
get_fname_size (char *path)
{
    struct stat st;
    if (stat (path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

size_t
get_fd_size (FILE *file)
{
    struct stat st;
    if (fstat (fileno (file), &st) == 0) {
        return st.st_size;
    }
    return 0;
}

/* Get file's last modification time (mtime) */
time_t
get_file_mtime (FILE *file)
{
    struct stat st;
    if (fstat (fileno (file), &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

/* This function loop through the directory and save all filename and store them 
 * into input_files and also update file_count. Notice that this function is 
 * platform specific and only support POSIX system. */
void
get_files_from_dir (char *dir,
                    char **input_files,
                    size_t file_count)
{
    DIR *dp;
    char *slash;
    char *lastchar;
    struct dirent *entry;
    struct stat statbuf;
    size_t count = 0;

    if ((dp = opendir(dir)) == NULL)
    {
        perror("Failed to open directory");
        return;
    }

    /* Remove trailing slash */  
    lastchar = dir + (strlen (dir) - 1);
    slash = strrchr (dir, '/');
    if (lastchar == slash)
    {
        *slash = '\0';
    }

    /* Loop over each entry in the directory */
    while ((entry = readdir(dp)) != NULL)
    {
        char path[PATH_MAX];
        snprintf (path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (stat(path, &statbuf) == -1)
        {
            perror("Failed to get file status");
            continue;
        }

        if (S_ISREG(statbuf.st_mode))
        {
            if (count < file_count)
            {
                input_files[count] = (char *) malloc (strlen(path) + 1);
                if (input_files[count] == NULL)
                {
                    perror("Failed to allocate memory for file path");
                    closedir(dp);
                    return;
                }
                strcpy(input_files[count], path);
                count++;
            }
            else
            {
                break;
            }
        }
    }

    closedir(dp);
}

/* Count the number of files of a directory */
size_t
get_filecount_from_dir (const char *dir)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    size_t count = 0;

    if ((dp = opendir(dir)) == NULL)
    {
        perror("Failed to open directory");
        return 0;
    }

    /* Loop over each entry in the directory */
    while ((entry = readdir(dp)) != NULL)
    {
        char path[PATH_MAX];
        snprintf (path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (stat(path, &statbuf) == -1)
        {
            perror("Failed to get file status");
            continue;
        }

        if (S_ISREG(statbuf.st_mode))
        {
            count++;
        }
    }

    closedir(dp);
    return count;
}
