#ifndef DISPLAY_H
#define DISPLAY_H

#include <stddef.h>

#include "options.h"

/* Return EOF on failure, a nonnegative value on success. */
int print_entry(const char *path, const char *name,
                const Options *options, int follow_links);

int print_directory_entry(const char *directory, const char *name,
                          const Options *options);

int print_directory_total(const char *directory,
                          char *names[], size_t count, const Options *options);

int should_print_total(const Options *options);
 
#endif
