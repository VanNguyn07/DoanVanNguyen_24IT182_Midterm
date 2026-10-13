#define _NETBSD_SOURCE 1

#include <sys/stat.h>

#include "mode.h"

static char file_type(mode_t mode)
{
    if (S_ISREG(mode)) {
        return '-';
    }
    if (S_ISDIR(mode)) {
        return 'd';
    }
    if (S_ISLNK(mode)) {
        return 'l';
    }
    if (S_ISCHR(mode)) {
        return 'c';
    }
    if (S_ISBLK(mode)) {
        return 'b';
    }
    if (S_ISFIFO(mode)) {
        return 'p';
    }
    if (S_ISSOCK(mode)) {
        return 's';
    }

#ifdef S_ISWHT
    if (S_ISWHT(mode)) {
        return 'w';
    }
#endif

    return '?';
}

void format_mode(mode_t mode, char output[11])
{
    output[0] = file_type(mode);

    output[1] = (mode & S_IRUSR) ? 'r' : '-';
    output[2] = (mode & S_IWUSR) ? 'w' : '-';
    output[3] = (mode & S_IXUSR) ? 'x' : '-';

    output[4] = (mode & S_IRGRP) ? 'r' : '-';
    output[5] = (mode & S_IWGRP) ? 'w' : '-';
    output[6] = (mode & S_IXGRP) ? 'x' : '-';

    output[7] = (mode & S_IROTH) ? 'r' : '-';
    output[8] = (mode & S_IWOTH) ? 'w' : '-';
    output[9] = (mode & S_IXOTH) ? 'x' : '-';

    if (mode & S_ISUID) {
        output[3] = (mode & S_IXUSR) ? 's' : 'S';
    }

    if (mode & S_ISGID) {
        output[6] = (mode & S_IXGRP) ? 's' : 'S';
    }

    if (mode & S_ISVTX) {
        output[9] = (mode & S_IXOTH) ? 't' : 'T';
    }

    output[10] = '\0';
}

char file_indicator(mode_t mode)
{
    if (S_ISDIR(mode)) {
        return '/';
    }
    if (S_ISLNK(mode)) {
        return '@';
    }
    if (S_ISSOCK(mode)) {
        return '=';
    }
    if (S_ISFIFO(mode)) {
        return '|';
    }

#ifdef S_ISWHT
    if (S_ISWHT(mode)) {
        return '%';
    }
#endif

    if (S_ISREG(mode) &&
        (mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
        return '*';
    }

    return '\0';
}
