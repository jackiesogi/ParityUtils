#ifndef METADATA_H
#define METADATA_H
#include <stdio.h>
#include <time.h>

#define SHA256_DIGEST_LENGTH 32

struct file_metadata
{
    char *path;
    unsigned char sha256[SHA256_DIGEST_LENGTH];
    size_t size;
    time_t mtime;
};

/* Allocates a new file_metadata struct */
struct file_metadata *
new_file_metadata ();

/* Frees a file_metadata struct */
void
free_file_metadata (struct file_metadata *x);

size_t
get_fd_size (FILE *file);

size_t
get_fname_size (char *path);

time_t
get_file_mtime (FILE *file);

void
calculate_sha256 (FILE *file, unsigned char output[SHA256_DIGEST_LENGTH]);

#endif  // METADATA_H
