#ifndef MODE_H
#define MODE_H

#include <sys/types.h>

void format_mode(mode_t mode, char output[11]);
char file_indicator(mode_t mode);

#endif
