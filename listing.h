#ifndef LISTING_H
#define LISTING_H

#include "options.h"

/* List directory contents using the selected options. */
int list_directory(const char *path, const Options *options);

#endif
