#include "parity_core.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void
xor_block (const unsigned char *blocks[],
        unsigned char *output,
        size_t num_blocks,
        size_t block_size)
{
    memset (output, 0, block_size);
    for (size_t i = 0; i < num_blocks; i++)
    {
        for (size_t j = 0; j < block_size; j++)
        {
            output[j] ^= blocks[i][j];
        }
    }
}

size_t
read_block (FILE *file, unsigned char *buffer, size_t block_size)
{
    size_t bytes = fread (buffer, 1, block_size, file);
    fprintf (stderr, "Read %zu bytes from fileno %d\n", bytes, file->_fileno);
    return bytes;
}

size_t
write_block (FILE *file, const unsigned char *buffer, size_t block_size)
{
    size_t bytes = fwrite (buffer, 1, block_size, file);
    fprintf (stderr, "Wrote %zu bytes to fileno %d\n", bytes, file->_fileno);
    return bytes;
}

