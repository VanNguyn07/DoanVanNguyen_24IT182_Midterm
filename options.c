#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "options.h"
#include "sizes.h"

int parse_options(int argc, char *argv[], Options *options,
                  int *first_operand)
{
    int option;

    options->show_all = 0;
    options->show_almost_all = (geteuid() == 0);
    options->reverse = 0;
    options->no_sort = 0;
    options->directory_itself = 0;
    options->recursive = 0;
    options->show_inode = 0;
    options->long_format = 0;
    options->numeric_ids = 0;
    options->classify = 0;
    options->show_blocks = 0;
    options->size_format = LS_SIZE_DEFAULT;
    options->time_kind = LS_TIME_MODIFIED;
    options->sort_kind = LS_SORT_NAME;
    options->quote_nonprintable = isatty(STDOUT_FILENO);

    opterr = 0;

    while ((option = getopt(argc, argv, "aArfdRilnFskhuctSqw")) != -1) {
        switch (option) {
        case 'a':
            options->show_all = 1;
            break;
        case 'A':
            options->show_almost_all = 1;
            break;
        case 'r':
            options->reverse = 1;
            break;
        case 'f':
            options->no_sort = 1;
            break;
        case 'd':
            options->directory_itself = 1;
            options->recursive = 0;
            break;
        case 'R':
            options->recursive = 1;
            options->directory_itself = 0;
            break;
        case 'i':
            options->show_inode = 1;
            break;
        case 'l':
            options->long_format = 1;
            options->numeric_ids = 0;
            break;
        case 'n':
            options->long_format = 1;
            options->numeric_ids = 1;
            break;
        case 'F':
            options->classify = 1;
            break;
        case 's':
            options->show_blocks = 1;
            break;
        case 'k':
            options->size_format = LS_SIZE_KILOBYTES;
            break;
        case 'h':
            options->size_format = LS_SIZE_HUMAN;
            break;
        case 'u':
            options->time_kind = LS_TIME_ACCESSED;
            break;
        case 'c':
            options->time_kind = LS_TIME_CHANGED;
            break;
        case 't':
            options->sort_kind = LS_SORT_TIME;
            break;
        case 'S':
            options->sort_kind = LS_SORT_SIZE;
            break;
        case 'q':
            options->quote_nonprintable = 1;
            break;
        case 'w':
            options->quote_nonprintable = 0;
            break;
        default:
            fprintf(stderr, "%s: invalid option -- %c\n",
                    argv[0], optopt);
            return 1;
        }
    }

    *first_operand = optind;
    return initialize_sizes(options);
}

int should_show_name(const char *name, const Options *options)
{
    if (options->show_all) {
        return 1;
    }

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return 0;
    }

    return name[0] != '.' || options->show_almost_all;
}
