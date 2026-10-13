#ifndef SIZES_H
#define SIZES_H

#include <stdint.h>

#include "options.h"

int initialize_sizes(Options *options);
uintmax_t block_count(uintmax_t blocks, const Options *options);

/* Print without a trailing space or newline.
   Return EOF on failure, zero on success. */
int print_blocks(uintmax_t blocks, const Options *options);
int print_byte_size(intmax_t bytes, const Options *options);

#endif
