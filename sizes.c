#define _NETBSD_SOURCE 1

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sizes.h"

int initialize_sizes(Options *options)
{
    options->block_unit = 512;

    if (options->size_format == LS_SIZE_KILOBYTES) {
        options->block_unit = 1024;
    } else if (options->size_format == LS_SIZE_DEFAULT) {
        int header_length;
        long environment_unit;

        getbsize(&header_length, &environment_unit);

        if (environment_unit < 512) {
            fprintf(stderr, "ls: invalid block unit\n");
            return 1;
        }

        options->block_unit = (uintmax_t)environment_unit;
    }

    return 0;
}

/* The caller checks that blocks * 512 cannot overflow. */
uintmax_t block_count(uintmax_t blocks, const Options *options)
{
    uintmax_t bytes = blocks * 512;

    return bytes / options->block_unit +
           (bytes % options->block_unit != 0);
}

static int print_human_size(int64_t bytes)
{
    char buffer[5];

    if (humanize_number(buffer, sizeof(buffer), bytes, "",
                        HN_AUTOSCALE,
                        HN_DECIMAL | HN_NOSPACE | HN_B) == -1) {
        errno = EOVERFLOW;
        return EOF;
    }

    return printf("%s", buffer) < 0 ? EOF : 0;
}

int print_blocks(uintmax_t blocks, const Options *options)
{
    if (options->size_format == LS_SIZE_HUMAN) {
        if (blocks > (uintmax_t)INT64_MAX / 512) {
            errno = EOVERFLOW;
            return EOF;
        }

        return print_human_size((int64_t)(blocks * 512));
    }

    if (blocks > UINTMAX_MAX / 512) {
        errno = EOVERFLOW;
        return EOF;
    }

    return printf("%ju", block_count(blocks, options)) < 0 ? EOF : 0;
}

int print_byte_size(intmax_t bytes, const Options *options)
{
    if (options->size_format == LS_SIZE_HUMAN) {
        if (bytes > INT64_MAX || bytes < INT64_MIN) {
            errno = EOVERFLOW;
            return EOF;
        }

        return print_human_size((int64_t)bytes);
    }

    return printf("%jd", bytes) < 0 ? EOF : 0;
}
