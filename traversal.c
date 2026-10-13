#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listing.h"
#include "traversal.h"
#include "sorting.h"
#include "names.h"

typedef struct DirectoryJob {
    char *path;
    struct DirectoryJob *next;
} DirectoryJob;


static char *join_path(const char *directory, const char *name)
{
    size_t directory_length = strlen(directory);
    size_t name_length = strlen(name);
    char *path;
    const char *separator;

    if (directory_length > SIZE_MAX - name_length - 2) {
        fprintf(stderr, "ls: path is too long\n");
        return NULL;
    }

    path = malloc(directory_length + name_length + 2);
    if (path == NULL) {
        fprintf(stderr, "ls: cannot allocate memory\n");
        return NULL;
    }

    separator = (directory_length > 0 &&
                 directory[directory_length - 1] == '/') ? "" : "/";

    snprintf(path, directory_length + name_length + 2,
             "%s%s%s", directory, separator, name);

    return path;
}

/* Push children in reverse display order: the stack is last-in-first-out. */
static int push_children(const char *directory,
                         const Options *options,
                         DirectoryJob **pending)
{
    struct dirent **entries = NULL;
    int count;
    int i;
    int status = 0;

    count = scan_sorted_directory(directory, &entries, options);
    if (count == -1) {
        perror(directory);
        return 1;
    }

    for (i = 0; i < count; i++) {
        int index;
        const char *name;
        char *path;
        struct stat info;
        DirectoryJob *job;

        index = count - 1 - i;
        name = entries[index]->d_name;

        /* Never descend into "." or "..", even with -a. */
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
            !should_show_name(name, options)) {
            continue;
        }

        path = join_path(directory, name);
        if (path == NULL) {
            status = 1;
            break;
        }

        if (lstat(path, &info) == -1) {
            perror(path);
            free(path);
            status = 1;
            continue;
        }

        /* Do not follow symbolic links encountered inside directories. */
        if (!S_ISDIR(info.st_mode)) {
            free(path);
            continue;
        }

        job = malloc(sizeof(*job));
        if (job == NULL) {
            fprintf(stderr, "ls: cannot allocate memory\n");
            free(path);
            status = 1;
            break;
        }

        job->path = path;
        job->next = *pending;
        *pending = job;
    }

    for (i = 0; i < count; i++) {
        free(entries[i]);
    }
    free(entries);

    return status;
}

int list_tree(const char *path, const Options *options)
{
    DirectoryJob *pending = NULL;
    int status;

    status = list_directory(path, options);
    if (status != 0 || !options->recursive) {
        return status;
    }

    if (push_children(path, options, &pending) != 0) {
        status = 1;
    }

    while (pending != NULL) {
        DirectoryJob *job = pending;
        int directory_status;

        pending = job->next;

        if (putchar('\n') == EOF ||
            print_name(job->path, options) == EOF ||
            fputs(":\n", stdout) == EOF) {
            perror("ls: output");
            free(job->path);
            free(job);
            status = 1;
            break;
        }

        directory_status = list_directory(job->path, options);

        if (directory_status != 0) {
            status = 1;
        } else if (push_children(job->path, options, &pending) != 0) {
            status = 1;
        }

        free(job->path);
        free(job);
    }

    /* Clean up remaining jobs if output failed. */
    while (pending != NULL) {
        DirectoryJob *job = pending;

        pending = job->next;
        free(job->path);
        free(job);
    }

    return status;
}
