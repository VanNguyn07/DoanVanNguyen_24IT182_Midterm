#ifndef SORTING_H
#define SORTING_H

#include <dirent.h>
#include <stddef.h>

#include "options.h"

int sort_names(char *names[], size_t count, const char *directory,
               const Options *options, int operands);

int scan_sorted_directory(const char *directory,
                          struct dirent ***entries,
                          const Options *options);

#endif
