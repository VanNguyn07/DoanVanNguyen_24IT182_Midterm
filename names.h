#ifndef NAMES_H
#define NAMES_H

#include "options.h"

/* Return EOF on failure, zero on success. */
int print_name(const char *name, const Options *options);
int print_name_line(const char *name, const Options *options);

#endif
