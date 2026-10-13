#ifndef TIMESTAMPS_H
#define TIMESTAMPS_H

#include <sys/stat.h>
#include <time.h>

#include "options.h"

struct timespec selected_timestamp(const struct stat *info,
                                   const Options *options);

#endif
