#ifndef LONG_FORMAT_H
#define LONG_FORMAT_H

#include <sys/stat.h>

#include "options.h"

int print_long_entry(const char *path, const char *name,
                     const struct stat *info, const Options *options,
                     const char *suffix);

#endif
