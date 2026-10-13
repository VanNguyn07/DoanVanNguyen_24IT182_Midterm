#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listing.h"
#include "display.h"
#include "sorting.h"

/* qsort passes pointers to elements of the array. */

int list_directory(const char *path, const Options *options)
{
    DIR *directory;
    struct dirent *entry;
    char **names = NULL;
    size_t count = 0;
    size_t capacity = 0;
    size_t i;
    int status = 0;

    directory = opendir(path);
    if (directory == NULL) {
        perror(path);
        return 1;
    }

    for (;;) {
        char *name_copy;

        errno = 0;
        entry = readdir(directory);

        if (entry == NULL) {
            if (errno != 0) {
                perror(path);
                status = 1;
            }
            break;
        }

        if (!should_show_name(entry->d_name, options)) {
            continue;
        }

        /* Grow the array when all available slots are occupied. */
        if (count == capacity) {
            size_t new_capacity;
            char **new_names;

            if (capacity > SIZE_MAX / sizeof(*names) / 2) {
                fprintf(stderr, "ls: too many directory entries\n");
                status = 1;
                break;
            }

            new_capacity = (capacity == 0) ? 16 : capacity * 2;
            new_names = realloc(names, new_capacity * sizeof(*names));

            if (new_names == NULL) {
                fprintf(stderr, "ls: cannot allocate memory\n");
                status = 1;
                break;
            }

            names = new_names;
            capacity = new_capacity;
        }

        /* Copy the name because readdir may reuse its internal buffer. */
        name_copy = malloc(strlen(entry->d_name) + 1);
        if (name_copy == NULL) {
            fprintf(stderr, "ls: cannot allocate memory\n");
            status = 1;
            break;
        }

        strcpy(name_copy, entry->d_name);
        names[count] = name_copy;
        count++;
    }

    if (closedir(directory) == -1) {
        perror(path);
        status = 1;
    }

    if (count > 0 && should_print_total(options) &&
        print_directory_total(path, names, count, options) == EOF) {
        perror(path);
        status = 1;
    }
    if (sort_names(names, count, path, options, 0) != 0) {
        status = 1;
    }

    for (i = 0; i < count; i++) {
        if (print_directory_entry(path, names[i], options) == EOF) {
            perror("ls: output");
            status = 1;
            break;
        }
    }

    /* Release every copied name, then release the array itself. */
    for (i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);

    return status;
}
