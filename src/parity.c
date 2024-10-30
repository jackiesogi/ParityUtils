#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parity.h"
#include "parity_core.h"
#include "metadata.h"

/* Parity function map table */
const parity_map parity_map_table[] =
{
    {"xor_encode", DEFAULT_XOR_ENCODE},
    {"xor_decode", DEFAULT_XOR_DECODE},
    {"reedsolomon_encode", DEFAULT_REEDSOLOMON_ENCODE},
    {"reedsolomon_decode", DEFAULT_REEDSOLOMON_DECODE},
    {NULL, NULL}  // End marker
};

/* Function to convert algorithm name to function pointer */
parity_fn
strtofptr (const char *name)
{
    if (name == NULL)
    {
        return NULL;
    }

    for (int i = 0; parity_map_table[i].name != NULL; ++i)
    {
        if (strcmp (parity_map_table[i].name, name) == 0)
        {
            return parity_map_table[i].function;
        }
    }
    return NULL;
}

struct file_metadata **
new_parfile_header (int file_count)
{
    struct file_metadata **table = (struct file_metadata **) malloc (file_count * sizeof (struct file_metadata *));
    if (!table)
    {
        perror ("Failed to allocate file metadata table");
        return NULL;
    }

    for (int i = 0; i < file_count; i++)
    {
        table[i] = (struct file_metadata *) malloc (sizeof (struct file_metadata));
        if (!table[i]) 
        {
            perror ("Failed to allocate file metadata entry");
            for (int j = 0; j < i; j++) free (table[j]);
            free (table);
            return NULL;
        }
    }
    return table;
}

void
free_parfile_header (struct file_metadata **table, int file_count)
{
    for (int i = 0; i < file_count; i++) 
    {
        free (table[i]);
    }
    free (table);
}

/* Record original files metadata to the parity file */
void
write_parfile_header (FILE *fp, char **input_files, int file_count) 
{
    struct file_metadata **parfile_header = new_parfile_header (file_count);
    if (!parfile_header) return;

    /* Write each file size to the metadata table */
    for (int i = 0; i < file_count; i++) 
    {
        FILE *file = fopen (input_files[i], "rb");
        if (!file) 
        {
            perror ("Failed to open input file for reading");
            free_parfile_header (parfile_header, file_count);
            return;
        }

        parfile_header[i]->size = get_fd_size (file);
        fclose (file);
    }

    /* Write the header and metadata table to the parity file */
    fprintf (fp, "<parfile s=%d>!", file_count);
    /* fprintf (stdout, "Writing tag: <parfile s=%d>!\n", file_count); */
    for (int i = 0; i < file_count; ++i) 
    {
        fprintf (fp, "%zu!", parfile_header[i]->size);
    }

    fflush (fp);
    fseek (fp, 0, SEEK_END);

    free_parfile_header (parfile_header, file_count);
}

/* Read metadata from the parity file and store it in a parfile header */
void
read_parfile_header (FILE *fp, struct file_metadata **parfile_header, int file_count) 
{
    /* Read the header */
    char header[256];
    fgets (header, sizeof (header), fp);

    /* Parse the header to get the file count */
    int count = 0;
    sscanf (header, "<parfile s=%d>!", &count);
    if (count != file_count) 
    {
        fprintf (stderr, "File count mismatch: %d != %d\n", count, file_count);
        return;
    }

    /* Read each file size from the metadata */
    for (int i = 0; i < file_count; i++) 
    {
        fscanf (fp, "%zu!", &parfile_header[i]->size);
    }
}

/* Currently only check if the first 11 bytes contains parity file tag */
bool
is_valid_parfile (char *path) 
{
    FILE *fp = fopen (path, "rb");
    if (!fp) 
    {
        perror ("Failed to open file");
        return false;
    }

    char header[12] = {0};
    fread (header, 1, 11, fp);
    if (strncmp (header, "<parfile s=", 11) != 0) 
    {
        fclose (fp);
        return false;
    }

    return true;
    /* TODO: Implement a thorough check on parity file validity */
  
    /* Reset the file pointer to the beginning */
    /* fseek (fp, 0, SEEK_SET); */
    /* size_t file_count; */
    /* if  (fscanf (fp, "<parfile s=%zu>!", &file_count) != 1)  */
    /* { */
    /*     fclose (fp); */
    /*     return false; */
    /* } */

    /* int delimiter_count = 0; */
    /* int ch; */
    /* while ((ch = fgetc (fp)) != EOF)  */
    /* { */
    /*     if (ch == '!')  */
    /*     { */
    /*         delimiter_count++; */
    /*     } */
    /* } */

    /* fclose (fp); */

    /* return (delimiter_count == file_count); */
}

/* Note: It recieve parity file's path as an argument so it will not affect the 
 * `_IO_read_ptr` for the future reading operations */
size_t
get_missing_file_size (const char *parfile, char **input_files, size_t file_count) 
{
    size_t total_size = 0;
    FILE *fp = fopen (parfile, "rb");
    if (!fp) 
    {
        perror ("Failed to open parity file for reading");
        return 0;
    }
    fscanf (fp, "<parfile s=%zu>!", &file_count);
    for (size_t i = 0; i < file_count; i++) 
    {
        size_t size;
        fscanf (fp, "%zu!", &size);
        total_size += size;
    }
    fclose (fp);
    
    size_t file_size = 0;
    for (size_t i = 0; i < file_count; i++) 
    {
        if (strcmp (input_files[i], parfile) != 0) 
        {
            file_size += get_fname_size (input_files[i]);
        }
    }

    /* fprintf (stdout, "Missing file size: %zu\n", total_size - file_size); */
    return total_size - file_size;
}

/* TODO: A function that check if the input file number is valid to generate
 * restored file*/
/* bool */
/* is_valid_inputfiles () */

/* Parity algorithm function definitions */
void
parity_encode_xor (char **input_files, int file_count, char *output_file) 
{
    FILE *out_fp = fopen (output_file, "wb");
    if (!out_fp) 
    {
        perror ("Failed to open output file for writing");
        return;
    }

    FILE **in_fps = (FILE **) malloc (file_count * sizeof (FILE *));
    if (!in_fps) 
    {
        perror("Failed to allocate file pointers");
        fclose(out_fp);
        return;
    }
  
    size_t maxsize = 0;
    /* size_t maxind = 0; */
    for (int i = 0; i < file_count; ++i) 
    {
        in_fps[i] = fopen (input_files[i], "rb");
        if (!in_fps[i]) 
        {
            perror ("Failed to open input file");
            for (int j = 0; j < i; j++) 
            {
                fclose (in_fps[j]);
            }
            fclose (out_fp);
            free (in_fps);
            return;
        }
        size_t size = get_fd_size (in_fps[i]);
        if (size > maxsize)
        {
            maxsize = size;
            /* maxind = i; */
        }
    }
  
    /* Write the header into the top of the parity file */
    write_parfile_header (out_fp, input_files, file_count);

    size_t block_size = DEFAULT_BLOCK_SIZE; 
    unsigned char **buffers = (unsigned char **) malloc (file_count * sizeof (unsigned char *));
    for (int i = 0; i < file_count; i++) 
    {
        buffers[i] = (unsigned char *) malloc (block_size);
    }
    unsigned char *output_buffer = (unsigned char *) malloc (block_size);

    /* Define a 4096-byte zeroed buffer for padding */
    unsigned char padding_buffer[4096] = {0};

    size_t bytes_read = 0;
    size_t total_read = 0;
    while (total_read < maxsize)
    {
        /* Process each input file, padding with the zeroed buffer if needed */
        for (int i = 0; i < file_count; i++) 
        {
            bytes_read = read_block (in_fps[i], buffers[i], block_size);
            if (bytes_read < block_size) 
            {
                /* Fill remaining buffer with padding if the file is shorter */
                memcpy(buffers[i] + bytes_read, padding_buffer, block_size - bytes_read);
            }
        }

        /* Perform XOR operation on the blocks */
        xor_block ((const unsigned char **)buffers, output_buffer, file_count, block_size);
        write_block (out_fp, output_buffer, block_size);

        total_read += block_size;
    }

    /* Cleanup */
    for (int i = 0; i < file_count; i++) 
    {
        fclose (in_fps[i]);
        free (buffers[i]);
    }
    fclose (out_fp);
    free (buffers);
    free (output_buffer);
    free (in_fps);
}

void
jump_parfile_header(FILE *parfile)
{
    if (!parfile)
    {
        fprintf(stderr, "Invalid parity file pointer\n");
        return;
    }

    /* Move the pointer to the beginning of the file */
    fseek(parfile, 0, SEEK_SET);

    /* Skip the initial tag "<parfile s=X>!" */
    int file_count;
    if (fscanf(parfile, "<parfile s=%d>!", &file_count) != 1)
    {
        fprintf(stderr, "Failed to read the file count from parity file header\n");
        return;
    }

    /* Skip each file size value in the metadata section */
    size_t size;
    for (int i = 0; i < file_count; i++)
    {
        if (fscanf(parfile, "%zu!", &size) != 1)
        {
            fprintf(stderr, "Failed to read file size from parity file header\n");
            return;
        }
    }
}

void
parity_decode_xor (char **input_files, int file_count, char *output_file) 
{
    FILE *out_fp = fopen (output_file, "wb");
    if (!out_fp) 
    {
        perror ("Failed to open output file for writing");
        return;
    }

    FILE **in_fps = (FILE **) malloc (file_count * sizeof (FILE *));
    if (!in_fps) 
    {
        perror ("Failed to allocate file pointers");
        fclose (out_fp);
        return;
    }

    char *parfile = NULL;
    int parind = 0;
    for (int i = 0; i < file_count; i++) 
    {
        in_fps[i] = fopen (input_files[i], "rb");
        if (!in_fps[i]) 
        {
            perror ("Failed to open input file");
            for (int j = 0; j < i; j++) 
            {
                fclose (in_fps[j]);
            }
            fclose (out_fp);
            free (in_fps);
            return;
        }
      
        /* Find parity file inside input_files */
        if (is_valid_parfile (input_files[i]))
        {
            parfile = input_files[i];
            parind = i;
            /* fprintf (stdout, "input_files parity file: %s", input_files[i]); */
        }
    }

    size_t block_size = DEFAULT_BLOCK_SIZE;
    unsigned char **buffers = (unsigned char **) malloc (file_count * sizeof (unsigned char *));
    for (int i = 0; i < file_count; i++) 
    {
        buffers[i] = (unsigned char *) malloc (block_size);
    }
    unsigned char *output_buffer = (unsigned char *) malloc (block_size);

    /* Define a 4096-byte zeroed buffer for padding */
    unsigned char padding_buffer[4096] = {0};

    /* Get the missing file size */
    size_t missing_size = get_missing_file_size (parfile, input_files, file_count);
            
    /* Calculate the number of rounds that need to write a whole block */
    size_t round = missing_size / block_size;
    size_t count = 0;
    
/*     fprintf (stdout, "The write operation will take %zu rounds (each round %zu bytes),\n\ */
/* and in the last round, it will write %zu bytes instead of a whole block.\n", round + 1, block_size, missing_size % block_size); */
    
    /* Before reading parity file, jump over the header section */
    jump_parfile_header (in_fps[parind]); 

    // Main decoding loop
    while (count <= round) 
    {
        size_t bytes_read = block_size;
        if (count == round) 
        {
            /* If it’s the last round, read only the remaining bytes */
            bytes_read = missing_size % block_size;
        }

        /* Read blocks for each file or use padding if a file has ended */
        for (int i = 0; i < file_count; i++) 
        {
            size_t read_bytes = read_block (in_fps[i], buffers[i], bytes_read);
            if (read_bytes < bytes_read) 
            {
                /* Fill the remaining part of the buffer with zero padding */
                memcpy(buffers[i] + read_bytes, padding_buffer, bytes_read - read_bytes);
            }
        }

        /* fprintf (stdout, "The bytes_read in round %zu: %zu\n", count + 1, bytes_read);     */

        /* XOR the blocks and write to output */
        xor_block ((const unsigned char **)buffers, output_buffer, file_count, bytes_read);
        write_block (out_fp, output_buffer, bytes_read);

        ++count;
    }

    /* Cleanup */
    for (int i = 0; i < file_count; i++) 
    {
        fclose (in_fps[i]);
        free (buffers[i]);
    }
    fclose (out_fp);
    free (buffers);
    free (output_buffer);
    free (in_fps);
}

void
parity_encode_reedsolomon (char **input_files, int file_count, char *output_file)
{
    // TODO: Implement Reed-Solomon encoding
    printf ("Reed-Solomon encoding implementation is still under development\n");
    
    if (input_files && file_count && output_file)
    {
      
    }
}

void
parity_decode_reedsolomon (char **input_files, int file_count, char *output_file)
{
    // TODO: Implement Reed-Solomon decoding
    printf("Reed-Solomon decoding implementation is still under development\n");
  
    if (input_files && file_count && output_file)
    {
      
    }
}
