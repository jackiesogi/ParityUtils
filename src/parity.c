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
parity_fn strtofptr(const char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for (int i = 0; parity_map_table[i].name != NULL; ++i) {
        if (strcmp(parity_map_table[i].name, name) == 0) {
            return parity_map_table[i].function;
        }
    }
    return NULL;
}

struct file_metadata **new_parfile_header(int file_count) {
    struct file_metadata **table = (struct file_metadata **)malloc(file_count * sizeof(struct file_metadata *));
    if (!table) {
        perror("Failed to allocate file metadata table");
        return NULL;
    }

    for (int i = 0; i < file_count; i++) {
        table[i] = (struct file_metadata *)malloc(sizeof(struct file_metadata));
        if (!table[i]) {
            perror("Failed to allocate file metadata entry");
            for (int j = 0; j < i; j++) free(table[j]);
            free(table);
            return NULL;
        }
    }
    return table;
}

void free_parfile_header(struct file_metadata **table, int file_count) {
    for (int i = 0; i < file_count; i++) {
        free(table[i]);
    }
    free(table);
}

/* Record original files metadata to the parity file */
void write_parfile_header(FILE *fp, char **input_files, int file_count) {
    struct file_metadata **parfile_header = new_parfile_header(file_count);
    if (!parfile_header) return;

    /* Write each file size to the metadata table */
    for (int i = 0; i < file_count; i++) {
        FILE *file = fopen(input_files[i], "rb");
        if (!file) {
            perror("Failed to open input file for reading");
            free_parfile_header(parfile_header, file_count);
            return;
        }

        parfile_header[i]->size = get_fd_size(file);
        fclose(file);
    }

    /* Write the header and metadata table to the parity file */
    fprintf(fp, "<parfile s=%d>!", file_count);
    fprintf(stdout, "Writing tag: <parfile s=%d>!", file_count);
    for (int i = 0; i < file_count; i++) {
        fprintf(fp, "%zu!", parfile_header[i]->size);
    }

    fflush(fp);
    fseek(fp, 0, SEEK_END);

    free_parfile_header(parfile_header, file_count);
}

/* Read metadata from the parity file and store it in a parfile header */
void read_parfile_header(FILE *fp, struct file_metadata **parfile_header, int file_count) {
    /* Read the header */
    char header[256];
    fgets(header, sizeof(header), fp);

    /* Parse the header to get the file count */
    int count = 0;
    sscanf(header, "<parfile s=%d>!", &count);
    if (count != file_count) {
        fprintf(stderr, "File count mismatch: %d != %d\n", count, file_count);
        return;
    }

    /* Read each file size from the metadata */
    for (int i = 0; i < file_count; i++) {
        fscanf(fp, "%zu!", &parfile_header[i]->size);
    }
}

bool is_valid_parfile(char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        perror("Failed to open file");
        return false;
    }

    char header[12] = {0};
    fread(header, 1, 11, fp);
    if (strncmp(header, "<parfile s=", 11) != 0) {
        fclose(fp);
        return false;
    }

    /* Reset the file pointer to the beginning */
    fseek(fp, 0, SEEK_SET);
    size_t file_count;
    if (fscanf(fp, "<parfile s=%zu>!", &file_count) != 1) {
        fclose(fp);
        return false;
    }

    int delimiter_count = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '!') {
            delimiter_count++;
        }
    }

    fclose(fp);

    return (delimiter_count == file_count);
}

size_t get_missing_file_size (const char *parfile, char **input_files, size_t file_count) {
    
    size_t total_size = 0;
    FILE *fp = fopen(parfile, "rb");
    if (!fp) {
        perror("Failed to open parity file for reading");
        return 0;
    }
    fscanf(fp, "<parfile s=%zu>!", &file_count);
    for (size_t i = 0; i < file_count; i++) {
        size_t size;
        fscanf(fp, "%zu!", &size);
        total_size += size;
    }
    fclose(fp);
    
    size_t file_size = 0;
    for (size_t i = 0; i < file_count; i++) {
        if (strcmp(input_files[i], parfile) != 0) {
            file_size += get_fname_size(input_files[i]);
        }
    }

    return total_size - file_size;
}

/* Parity algorithm function definitions */
void parity_encode_xor(char **input_files, int file_count, char *output_file) {
    FILE *out_fp = fopen(output_file, "wb");
    if (!out_fp) {
        perror("Failed to open output file for writing");
        return;
    }

    FILE **in_fps = (FILE **)malloc(file_count * sizeof(FILE *));
    if (!in_fps) {
        perror("Failed to allocate file pointers");
        fclose(out_fp);
        return;
    }

    // Open input files
    for (int i = 0; i < file_count; ++i) {
        in_fps[i] = fopen(input_files[i], "rb");
        if (!in_fps[i]) {
            perror("Failed to open input file");
            for (int j = 0; j < i; j++) {
                fclose(in_fps[j]);
            }
            fclose(out_fp);
            free(in_fps);
            return;
        }
    }

    size_t block_size = DEFAULT_BLOCK_SIZE; 
    unsigned char **buffers = (unsigned char **)malloc(file_count * sizeof(unsigned char *));
    for (int i = 0; i < file_count; i++) {
        buffers[i] = (unsigned char *)malloc(block_size);
    }
    unsigned char *output_buffer = (unsigned char *)malloc(block_size);

    size_t bytes_read = 0;
    while ((bytes_read = read_block(in_fps[0], buffers[0], block_size)) > 0) {
        for (int i = 1; i < file_count; i++) {
            read_block(in_fps[i], buffers[i], block_size);
        }

        // Perform XOR operation on the blocks
        xor_block((const unsigned char **)buffers, output_buffer, file_count, bytes_read);
        write_block(out_fp, output_buffer, bytes_read);
    }

    // Cleanup
    for (int i = 0; i < file_count; i++) {
        fclose(in_fps[i]);
        free(buffers[i]);
    }
    fclose(out_fp);
    free(buffers);
    free(output_buffer);
    free(in_fps);
}

// void parity_encode_xor(char **input_files, int file_count, char *output_file)
// {
//     FILE *out_fp = fopen(output_file, "wb");
//     if (!out_fp) {
//         perror("Failed to open output file for writing");
//         return;
//     }
//
//     FILE **in_fps = (FILE **)malloc(file_count * sizeof(FILE *));
//     if (!in_fps) {
//         perror("Failed to allocate file pointers");
//         fclose(out_fp);
//         return;
//     }
//
//     for (int i = 0; i < file_count; ++i) {
//         in_fps[i] = fopen(input_files[i], "rb");
//         if (!in_fps[i]) {
//             perror("Failed to open input file");
//             for (int j = 0; j < i; j++) {
//                 fclose(in_fps[j]);
//             }
//             fclose(out_fp);
//             free(in_fps);
//             return;
//         }
//     }
//
//     write_parfile_header(out_fp, input_files, file_count);
//
//     size_t block_size = DEFAULT_BLOCK_SIZE; 
//     unsigned char **buffers = (unsigned char **)malloc(file_count * sizeof(unsigned char *));
//     for (int i = 0; i < file_count; i++) {
//         buffers[i] = (unsigned char *)malloc(block_size);
//     }
//     unsigned char *output_buffer = (unsigned char *)malloc(block_size);
//
//     size_t bytes_read = 0;
//     while ((bytes_read = read_block(in_fps[0], buffers[0], block_size)) > 0) {
//         for (int i = 1; i < file_count; i++) {
//             read_block(in_fps[i], buffers[i], block_size);
//         }
//
//         xor_block((const unsigned char **)buffers, output_buffer, file_count, bytes_read);
//         write_block(out_fp, output_buffer, bytes_read);
//     }
//
//     for (int i = 0; i < file_count; i++) {
//         fclose(in_fps[i]);
//         free(buffers[i]);
//     }
//     fclose(out_fp);
//     free(buffers);
//     free(output_buffer);
//     free(in_fps);
// }

// void parity_decode_xor(char **input_files, int file_count, char *output_file) {
//     FILE *out_fp = fopen(output_file, "wb");
//     if (!out_fp) {
//         perror("Failed to open output file for writing");
//         return;
//     }
//
//     FILE **in_fps = (FILE **)malloc(file_count * sizeof(FILE *));
//     if (!in_fps) {
//         perror("Failed to allocate file pointers");
//         fclose(out_fp);
//         return;
//     }
//
//     for (int i = 0; i < file_count; i++) {
//         in_fps[i] = fopen(input_files[i], "rb");
//         if (!in_fps[i]) {
//             perror("Failed to open input file");
//             for (int j = 0; j < i; j++) {
//                 fclose(in_fps[j]);
//             }
//             fclose(out_fp);
//             free(in_fps);
//             return;
//         }
//     }
//
//     FILE *par_fp = NULL;
//     for (int i = 0; i < file_count; i++) {
//         if (is_valid_parfile(input_files[i])) {
//             par_fp = in_fps[i];
//             fprintf(stdout, "Parity file found: %s\n", input_files[i]);
//
//             fseek(par_fp, 0, SEEK_SET);
//             int ch;
//             while ((ch = fgetc(par_fp)) != EOF) {
//                 if (ch == '!') {
//                     break;
//                 }
//             }
//             if (ch == EOF) {
//                 fprintf(stderr, "Failed to find end of header in parity file\n");
//                 for (int j = 0; j < file_count; j++) {
//                     fclose(in_fps[j]);
//                 }
//                 fclose(out_fp);
//                 free(in_fps);
//                 return;
//             }
//
//             while ((ch = fgetc(par_fp)) != EOF && ch != '\n') {
//                 if (ch == '!') {
//                     break;
//                 }
//             }
//             if (ch == EOF) {
//                 fprintf(stderr, "Failed to find end of metadata in parity file\n");
//                 for (int j = 0; j < file_count; j++) {
//                     fclose(in_fps[j]);
//                 }
//                 fclose(out_fp);
//                 free(in_fps);
//                 return;
//             }
//             break;
//         }
//     }
//
//     size_t block_size = DEFAULT_BLOCK_SIZE;
//     unsigned char **buffers = (unsigned char **)malloc(file_count * sizeof(unsigned char *));
//     for (int i = 0; i < file_count; i++) {
//         buffers[i] = (unsigned char *)malloc(block_size);
//     }
//     unsigned char *output_buffer = (unsigned char *)malloc(block_size);
//
//     size_t bytes_read;
//     size_t bytes_to_write = get_missing_file_size(input_files[0], input_files, file_count);
//     fprintf(stdout, "Missing file size: %zu\n", bytes_to_write);
//     size_t total_written = 0;
//
//     while (total_written < bytes_to_write && (bytes_read = read_block(in_fps[0], buffers[0], block_size)) > 0) {
//         for (int i = 1; i < file_count; i++) {
//             read_block(in_fps[i], buffers[i], block_size);
//         }
//
//         if (total_written + bytes_read > bytes_to_write) {
//             bytes_read = bytes_to_write - total_written;
//         }
//
//         xor_block((const unsigned char **)buffers, output_buffer, file_count, bytes_read);
//         write_block(out_fp, output_buffer, bytes_read);
//
//         total_written += bytes_read;
//     }
//
//     for (int i = 0; i < file_count; i++) {
//         fclose(in_fps[i]);
//         free(buffers[i]);
//     }
//     fclose(out_fp);
//     free(buffers);
//     free(output_buffer);
//     free(in_fps);
// }

void parity_decode_xor(char **input_files, int file_count, char *output_file) {
    FILE *out_fp = fopen(output_file, "wb");
    if (!out_fp) {
        perror("Failed to open output file for writing");
        return;
    }

    FILE **in_fps = (FILE **)malloc(file_count * sizeof(FILE *));
    if (!in_fps) {
        perror("Failed to allocate file pointers");
        fclose(out_fp);
        return;
    }

    for (int i = 0; i < file_count; i++) {
        in_fps[i] = fopen(input_files[i], "rb");
        if (!in_fps[i]) {
            perror("Failed to open input file");
            for (int j = 0; j < i; j++) {
                fclose(in_fps[j]);
            }
            fclose(out_fp);
            free(in_fps);
            return;
        }
    }

    size_t block_size = DEFAULT_BLOCK_SIZE;
    unsigned char **buffers = (unsigned char **)malloc(file_count * sizeof(unsigned char *));
    for (int i = 0; i < file_count; i++) {
        buffers[i] = (unsigned char *)malloc(block_size);
    }
    unsigned char *output_buffer = (unsigned char *)malloc(block_size);

    size_t bytes_read;

    while ((bytes_read = read_block(in_fps[0], buffers[0], block_size)) > 0) {
        for (int i = 1; i < file_count; i++) {
            read_block(in_fps[i], buffers[i], block_size);
        }

        xor_block((const unsigned char **)buffers, output_buffer, file_count, bytes_read);
        write_block(out_fp, output_buffer, bytes_read);
    }

    for (int i = 0; i < file_count; i++) {
        fclose(in_fps[i]);
        free(buffers[i]);
    }
    fclose(out_fp);
    free(buffers);
    free(output_buffer);
    free(in_fps);
}

// void parity_encode_xor(char **input_files, int file_count, char *output_file)
// {
//     // XOR encoding implementation
//     printf("XOR encoding\n");
//     for (int i = 0; i < file_count; ++i) {
//         printf("Input file: %s\n", input_files[i]);
//     }
//     printf("Output file: %s\n", output_file);
// }
//
// void parity_decode_xor(char **input_files, int file_count, char *output_file)
// {
//     // XOR decoding implementation
//     printf("XOR decoding\n");
// }

void parity_encode_reedsolomon(char **input_files, int file_count, char *output_file)
{
    // RAID4 encoding implementation
    printf("RAID4 encoding\n");
}

void parity_decode_reedsolomon(char **input_files, int file_count, char *output_file)
{
    // RAID4 decoding implementation
    printf("RAID4 decoding\n");
}
