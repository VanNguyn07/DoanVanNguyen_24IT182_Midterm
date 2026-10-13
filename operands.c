#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "traversal.h"
#include "display.h"
#include "sorting.h"
#include "names.h"
#include "operands.h"



static int read_operand_info(const char *path, struct stat *info,
                             const Options *options)
{
    struct stat target_info;

    /* Inspect the operand itself, including a dangling symbolic link. */
    if (lstat(path, info) == -1) {
        perror(path);
        return 1;
    }

    /* Without -d, follow an operand link that points to a directory.
       This policy will be extended when -l and -F are implemented. */
    if (!options->directory_itself && !options->long_format && !options->classify && S_ISLNK(info->st_mode)) {
        if (stat(path, &target_info) == 0 &&
            S_ISDIR(target_info.st_mode)) {
            *info = target_info;
        }
    }

    return 0;
}

int list_operands(int count, char *paths[], const Options *options)
{
    char **files;
    char **directories;
    size_t file_count = 0;
    size_t directory_count = 0;
    size_t i;
    int argument;
    int status = 0;
    int printed_section = 0;

    files = calloc((size_t)count, sizeof(*files));
    directories = calloc((size_t)count, sizeof(*directories));

    if (files == NULL || directories == NULL) {
        fprintf(stderr, "ls: cannot allocate memory\n");
        free(files);
        free(directories);
        return 1;
    }

    for (argument = 0; argument < count; argument++) {
        struct stat file_info;

        if (read_operand_info(paths[argument], &file_info, options) != 0) {
            status = 1;
            continue;
        }

        if (!options->directory_itself &&
            S_ISDIR(file_info.st_mode)) {
            directories[directory_count++] = paths[argument];
        } else {
            files[file_count++] = paths[argument];
        }
    }

    if (sort_names(files, file_count, NULL, options, 1) != 0) {
        status = 1;
    }
    if (sort_names(directories, directory_count, NULL, options, 1) != 0) {
        status = 1;
    }

    for (i = 0; i < file_count; i++) {
        size_t index = i;

        if (print_entry(files[index], files[index], options, !options->directory_itself) == EOF) {
            perror("ls: output");
            status = 1;
            break;
        }

        printed_section = 1;
    }

    for (i = 0; i < directory_count; i++) {
        size_t index = i;
        const char *path = directories[index];

        if (count > 1) {
            if (printed_section && putchar('\n') == EOF) {
                perror("ls: output");
                status = 1;
                break;
            }

            if (print_name(path, options) == EOF ||
                fputs(":\n", stdout) == EOF) {
                perror("ls: output");
                status = 1;
                break;
            }
        }

        if (list_tree(path, options) != 0) {
            status = 1;
        }

        printed_section = 1;
    }

    free(files);
    free(directories);

    return status;
}
