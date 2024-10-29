#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <time.h>

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
