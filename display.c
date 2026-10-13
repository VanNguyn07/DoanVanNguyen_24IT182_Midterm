#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "display.h"
#include "long_format.h"
#include "sizes.h"
#include "names.h"
#include "mode.h"

static char *join_path(const char *directory, const char *name)
{
    size_t directory_length = strlen(directory);
    size_t name_length = strlen(name);
    const char *separator;
    char *path;

    if (directory_length > SIZE_MAX - name_length - 2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    path = malloc(directory_length + name_length + 2);
    if (path == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    separator = (directory_length > 0 &&
                 directory[directory_length - 1] == '/') ? "" : "/";

    snprintf(path, directory_length + name_length + 2,
             "%s%s%s", directory, separator, name);

    return path;
}

int print_entry(const char *path, const char *name,
                const Options *options, int follow_links)
{
    struct stat info;
    char suffix[2] = { 0, 0 };

    if (!options->show_inode && !options->long_format && !options->classify && !options->show_blocks) {
        return print_name_line(name, options);
    }

    if (lstat(path, &info) == -1) {
        return EOF;
    }

    /* Long format displays the symbolic link itself. */
    if (follow_links && !options->long_format && !options->classify &&
        S_ISLNK(info.st_mode)) {
        struct stat target_info;

        if (stat(path, &target_info) == 0) {
            info = target_info;
        }
    }

    if (options->classify) {
        suffix[0] = file_indicator(info.st_mode);
    }
 
    if (options->show_inode) {
        if (printf("%ju ", (uintmax_t)info.st_ino) < 0) {
            return EOF;
        }
    }

    if (options->show_blocks) {
        if (info.st_blocks < 0) {
            errno = EOVERFLOW;
            return EOF;
        }
        if (print_blocks((uintmax_t)info.st_blocks, options) == EOF ||
            putchar(' ') == EOF) {
            return EOF;
        }
    }
 
    if (options->long_format) {
        return print_long_entry(path, name, &info,
                                options, suffix);
    }

    if (print_name(name, options) == EOF ||
        fputs(suffix, stdout) == EOF || putchar('\n') == EOF) {
        return EOF;
    }
    return 0;
}

int print_directory_entry(const char *directory, const char *name,
                          const Options *options)
{
    char *path;
    int result;
    int saved_errno;

    if (!options->show_inode && !options->long_format && !options->classify && !options->show_blocks) {
        return print_name_line(name, options);
    }

    path = join_path(directory, name);
    if (path == NULL) {
        return EOF;
    }

    result = print_entry(path, name, options, 0);

    saved_errno = errno;
    free(path);
    errno = saved_errno;

    return result;
}

int print_directory_total(const char *directory,
                          char *names[], size_t count, const Options *options)
{
    uintmax_t total = 0;
    uintmax_t logical_total = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        struct stat info;
        char *path;
        int result;
        int saved_errno;

        path = join_path(directory, names[i]);
        if (path == NULL) {
            return EOF;
        }

        result = lstat(path, &info);
        saved_errno = errno;
        free(path);
        errno = saved_errno;

        if (result == -1) {
            return EOF;
        }

        if (info.st_blocks < 0 ||
            (uintmax_t)info.st_blocks > UINTMAX_MAX - total) {
            errno = EOVERFLOW;
            return EOF;
        }

        total += (uintmax_t)info.st_blocks;

        if (options->size_format == LS_SIZE_HUMAN) {
            if (info.st_size < 0 ||
                (uintmax_t)info.st_size >
                (uintmax_t)INTMAX_MAX - logical_total) {
                errno = EOVERFLOW;
                return EOF;
            }
            logical_total += (uintmax_t)info.st_size;
        }
    }

    if (printf("total ") < 0 ||
        (options->size_format == LS_SIZE_HUMAN
         ? print_byte_size((intmax_t)logical_total, options)
         : print_blocks(total, options)) == EOF ||
        putchar('\n') == EOF) {
        return EOF;
    }
    return 0;
}

int should_print_total(const Options *options)
{
    return options->long_format ||
           (options->show_blocks && isatty(STDOUT_FILENO));
}
