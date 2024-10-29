#ifndef PARITY_CORE_H
#define PARITY_CORE_H

#include <stddef.h>
#include <stdio.h>

#define BLOCK_SIZE 4096  // 4 KiB blocks for efficiency

// XORs multiple blocks and stores the result in the output block
void
xor_block (const unsigned char *blocks[], unsigned char *output, size_t num_blocks, size_t block_size);

// Reads a block from a file at the current position
size_t
read_block(FILE *file, unsigned char *buffer, size_t block_size);

// Writes a block to a file at the current position
size_t
write_block(FILE *file, const unsigned char *buffer, size_t block_size);

#endif
