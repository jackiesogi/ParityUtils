#ifndef PARITY_H
#define PARITY_H

#include <stdbool.h>

/* Aliases to algorithm names */
#define  DEFAULT_XOR_ENCODE          parity_encode_xor
#define  DEFAULT_XOR_DECODE          parity_decode_xor
#define  DEFAULT_REEDSOLOMON_ENCODE  parity_encode_reedsolomon
#define  DEFAULT_REEDSOLOMON_DECODE  parity_decode_reedsolomon

/* Default block size for encoding */
#define  DEFAULT_BLOCK_SIZE   4 << 10  // 4KB
#define  DEFAULT_BUFFER_SIZE  4 << 10  // 4KB

/* Declare `parity_fn` as a type for function pointers which takes an array of strings
 * INPUT_FILES, an integer FILE_COUNT, a string OUTPUT_FILE and returns void. */
typedef void (*parity_fn) (char **input_files, int file_count, char *output_file);

/* A map to store the parity function name and its pointer */
typedef struct parity_map
{
    const char *name;
    parity_fn function;
} parity_map;

/* `parity_options` is a struct that contains info that a parity function might need. */
typedef struct
{
    char **input_files;    // List of input files
    int file_count;        // Number of input files
    char *output_file;     // Output parity file path
    parity_fn algorithm;   // Chosen algorithm function pointer
} parity_options;

/* `parity_map_table[]` maps the alogrithm aliases and corresponding function names,
 * for converting from command line arguments that the user set. For example, if the
 * user set the parity algorithm to Reed-Solomon, the program would try to find the 
 * corresponding function in this map using `strtofptr()`. (string to function pointer)*/
extern const parity_map parity_map_table[];

/* Get the parity function pointer by a given algorithm NAME. Specifically, it tries to
 * iterate through `parity_map_table[]` to find if there is a matched function pointer. */
parity_fn
strtofptr (const char *name);

/* Default parity encoding algorithm */
void
parity_encode_xor (char **input_files, int file_count, char *output_file);

/* Default parity decoding algorithm */
void
parity_decode_xor (char **input_files, int file_count, char *output_file);

/* TODO: Finish the implementations */
void
parity_encode_reedsolomon (char **input_files, int file_count, char *output_file);
void
parity_decode_reedsolomon (char **input_files, int file_count, char *output_file);

/* Set the parity algorithm for encoding */
bool
set_parity_encode (parity_fn algorithm);

/* Set the parity algorithm for decoding */
bool
set_parity_decode (parity_fn algorithm);

#endif  // PARITY_H

/*------------------------- End of parity.h -------------------------*/
