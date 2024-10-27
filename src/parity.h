#ifndef PARITY_H
#define PARITY_H

#include <stdbool.h>

/* Macro to define the function pointer of the parity algorithm */
#define  DEFAULT_XOR_ENCODE    parity_encode_xor
#define  DEFAULT_XOR_DECODE    parity_decode_xor
#define  DEFAULT_RAID4_ENCODE  parity_encode_raid4
#define  DEFAULT_RAID4_DECODE  parity_decode_raid4
#define  DEFAULT_RAID5_ENCODE  parity_encode_raid5
#define  DEFAULT_RAID5_DECODE  parity_decode_raid5

void parity_encode_xor(char **input_files, int file_count, char *output_file);
void parity_decode_xor(char **input_files, int file_count, char *output_file);
void parity_encode_raid4(char **input_files, int file_count, char *output_file);
void parity_decode_raid4(char **input_files, int file_count, char *output_file);
void parity_encode_raid5(char **input_files, int file_count, char *output_file);
void parity_decode_raid5(char **input_files, int file_count, char *output_file);

/* Declare `parity_fn` as a type for function pointers */
typedef void (*parity_fn)(char **input_files, int file_count, char *output_file);

/* Structure to pass options to parity functions */
typedef struct
{
    char **input_files;    // List of input files
    int file_count;        // Number of input files
    char *output_file;     // Output parity file path
    parity_fn algorithm;   // Chosen algorithm function pointer
} parity_options;

/* Set the parity algorithm for encoding */
bool set_parity_encode(parity_fn algorithm);

/* Set the parity algorithm for decoding */
bool set_parity_decode(parity_fn algorithm);

/* Get the parity function pointer by a given algorithm name */
parity_fn strtofptr(const char *name);

/* A map to store the parity function name and its pointer */
typedef struct parity_map
{
    const char *name;
    parity_fn function;
} parity_map;

extern const parity_map parity_map_table[];

#endif  // PARITY_H

