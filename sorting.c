#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sorting.h"
#include "timestamps.h"

typedef struct {
    char *name;
    void *original;
    struct stat info;
    const Options *options;
} SortEntry;

static int compare_entries(const void *left, const void *right)
{
    const SortEntry *a = left;
    const SortEntry *b = right;
    const Options *options = a->options;
    int result = 0;

    if (options->sort_kind == LS_SORT_SIZE) {
        if (a->info.st_size != b->info.st_size) {
            result = (a->info.st_size > b->info.st_size) ? -1 : 1;
        }
    } else if (options->sort_kind == LS_SORT_TIME) {
        struct timespec ta = selected_timestamp(&a->info, options);
        struct timespec tb = selected_timestamp(&b->info, options);

        if (ta.tv_sec != tb.tv_sec) {
            result = (ta.tv_sec > tb.tv_sec) ? -1 : 1;
        } else if (ta.tv_nsec != tb.tv_nsec) {
            result = (ta.tv_nsec > tb.tv_nsec) ? -1 : 1;
        }
    }

    /* Equal primary keys are ordered by name. */
    if (result == 0) {
        result = strcmp(a->name, b->name);
    }

    if (options->reverse) {
        result = (result > 0) ? -1 : (result < 0 ? 1 : 0);
    }

    return result;
}

static char *make_path(const char *directory, const char *name)
{
    size_t directory_length = directory ? strlen(directory) : 0;
    size_t name_length = strlen(name);
    const char *separator = "";
    char *path;

    if (directory_length > SIZE_MAX - name_length - 2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    if (directory_length > 0 &&
        directory[directory_length - 1] != '/') {
        separator = "/";
    }

    path = malloc(directory_length + name_length + 2);
    if (path == NULL) {
        return NULL;
    }

    snprintf(path, directory_length + name_length + 2,
             "%s%s%s", directory ? directory : "", separator, name);

    return path;
}

static int sort_records(SortEntry records[], size_t count,
                        const char *directory,
                        const Options *options, int operands)
{
    size_t i;

    for (i = 0; i < count; i++) {
        records[i].options = options;

        if (options->sort_kind != LS_SORT_NAME) {
            char *path = make_path(directory, records[i].name);
            int result;
            int saved_errno;

            if (path == NULL) {
                perror("ls: sorting");
                return 1;
            }

            result = lstat(path, &records[i].info);

            if (result == 0 && operands &&
                !options->directory_itself &&
                !options->long_format && !options->classify &&
                S_ISLNK(records[i].info.st_mode)) {
                struct stat target_info;

                if (stat(path, &target_info) == 0) {
                    records[i].info = target_info;
                }
            }

            saved_errno = errno;

            if (result == -1) {
                perror(path);
                free(path);
                errno = saved_errno;
                return 1;
            }

            free(path);
        }
    }

    qsort(records, count, sizeof(*records), compare_entries);
    return 0;
}

int sort_names(char *names[], size_t count, const char *directory,
               const Options *options, int operands)
{
    SortEntry *records;
    size_t i;
    int status;

    if (options->no_sort || count < 2) {
        return 0;
    }

    records = calloc(count, sizeof(*records));
    if (records == NULL) {
        perror("ls: sorting");
        return 1;
    }

    for (i = 0; i < count; i++) {
        records[i].name = names[i];
        records[i].original = names[i];
    }

    status = sort_records(records, count, directory, options, operands);

    if (status == 0) {
        for (i = 0; i < count; i++) {
            names[i] = records[i].original;
        }
    }

    free(records);
    return status;
}

int scan_sorted_directory(const char *directory,
                          struct dirent ***entries,
                          const Options *options)
{
    SortEntry *records;
    int count;
    int i;

    count = scandir(directory, entries, NULL, NULL);
    if (count < 2 || options->no_sort) {
        return count;
    }

    records = calloc((size_t)count, sizeof(*records));
    if (records == NULL) {
        goto failure;
    }

    for (i = 0; i < count; i++) {
        records[i].name = (*entries)[i]->d_name;
        records[i].original = (*entries)[i];
    }

    if (sort_records(records, (size_t)count,
                     directory, options, 0) != 0) {
        free(records);
        goto failure;
    }

    for (i = 0; i < count; i++) {
        (*entries)[i] = records[i].original;
    }

    free(records);
    return count;

failure:
    {
        int saved_errno = errno;

        for (i = 0; i < count; i++) {
            free((*entries)[i]);
        }
        free(*entries);
        *entries = NULL;
        errno = saved_errno;
    }
    return -1;
}
