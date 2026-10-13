#define _POSIX_C_SOURCE 200809L

#include "timestamps.h"

struct timespec selected_timestamp(const struct stat *info,
                                   const Options *options)
{
    switch (options->time_kind) {
    case LS_TIME_ACCESSED:
        return info->st_atim;
    case LS_TIME_CHANGED:
        return info->st_ctim;
    case LS_TIME_MODIFIED:
    default:
        return info->st_mtim;
    }
}
