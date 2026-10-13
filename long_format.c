#define _NETBSD_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "long_format.h"
#include "mode.h"
#include "sizes.h"
#include "timestamps.h"
#include "names.h"

/* Grow the buffer if the link target does not fit. */
static char *read_link_target(const char *path)
{
    size_t capacity = 128;

    for (;;) {
        char *target;
        ssize_t length;

        target = malloc(capacity + 1);
        if (target == NULL) {
            errno = ENOMEM;
            return NULL;
        }

        length = readlink(path, target, capacity);

        if (length == -1) {
            int saved_errno = errno;

            free(target);
            errno = saved_errno;
            return NULL;
        }

        if ((size_t)length < capacity) {
            target[length] = '\0';
            return target;
        }

        free(target);

        if (capacity > (SIZE_MAX - 1) / 2) {
            errno = ENAMETOOLONG;
            return NULL;
        }

        capacity *= 2;
    }
}

int print_long_entry(const char *path, const char *name,
                     const struct stat *info, const Options *options, const char *suffix)
{
    char permissions[11];
    char date[64];
    time_t timestamp;
    struct tm *local_time;
    struct passwd *owner;
    struct group *group;
    char *link_target = NULL;
    int result = EOF;
    int saved_errno;

    format_mode(info->st_mode, permissions);

    timestamp = selected_timestamp(info, options).tv_sec;
    local_time = localtime(&timestamp);
    if (local_time == NULL) {
        errno = EOVERFLOW;
        return EOF;
    }

    if (strftime(date, sizeof(date), "%b %e %H:%M", local_time) == 0) {
        errno = EOVERFLOW;
        return EOF;
    }

    /* Read the target before printing to avoid a partial line on failure. */
    if (S_ISLNK(info->st_mode)) {
        link_target = read_link_target(path);
        if (link_target == NULL) {
            return EOF;
        }
    }

    if (printf("%s %ju ", permissions,
               (uintmax_t)info->st_nlink) < 0) {
        goto cleanup;
    }

    owner = options->numeric_ids ? NULL : getpwuid(info->st_uid);

    if (owner != NULL) {
        if (printf("%s ", owner->pw_name) < 0) {
            goto cleanup;
        }
    } else {
        if (printf("%ju ", (uintmax_t)info->st_uid) < 0) {
            goto cleanup;
        }
    }

    group = options->numeric_ids ? NULL : getgrgid(info->st_gid);

    if (group != NULL) {
        if (printf("%s ", group->gr_name) < 0) {
            goto cleanup;
        }
    } else {
        if (printf("%ju ", (uintmax_t)info->st_gid) < 0) {
            goto cleanup;
        }
    }

    /* Device files display major/minor numbers instead of byte size. */
    if (S_ISCHR(info->st_mode) || S_ISBLK(info->st_mode)) {
        if (printf("%ju, %ju ",
                   (uintmax_t)major(info->st_rdev),
                   (uintmax_t)minor(info->st_rdev)) < 0) {
            goto cleanup;
        }
    } else {
        if (print_byte_size((intmax_t)info->st_size, options) == EOF ||
            putchar(' ') == EOF) {
            goto cleanup;
        }
    }

    if (printf("%s ", date) < 0 ||
        print_name(name, options) == EOF ||
        fputs(suffix, stdout) == EOF) {
        goto cleanup;
    }

    if (link_target != NULL) {
        if (fputs(" -> ", stdout) == EOF ||
            print_name(link_target, options) == EOF) {
            goto cleanup;
        }
    }

    if (putchar('\n') == EOF) {
        goto cleanup;
    }

    result = 0;

cleanup:
    saved_errno = errno;
    free(link_target);
    errno = saved_errno;

    return result;
}
