#ifndef OPTIONS_H
#define OPTIONS_H
#include <stdint.h>

typedef enum {
    LS_SIZE_DEFAULT,
    LS_SIZE_KILOBYTES, LS_SIZE_HUMAN
} SizeFormat;

typedef enum {
    LS_TIME_MODIFIED,
    LS_TIME_ACCESSED,
    LS_TIME_CHANGED
} FileTimeKind;
 
typedef enum {
    LS_SORT_NAME,
    LS_SORT_TIME,
    LS_SORT_SIZE
} SortKind;
 
typedef struct {
    int show_all;
    int show_almost_all;
    int reverse;
    int no_sort;
    int directory_itself;
    int recursive;
    int show_inode;
    int long_format;
    int numeric_ids;
    int classify;
    int show_blocks;
    SizeFormat size_format;
    uintmax_t block_unit;
    FileTimeKind time_kind;
    SortKind sort_kind;
    int quote_nonprintable;
} Options;

int parse_options(int argc, char *argv[], Options *options,
                  int *first_operand);
int should_show_name(const char *name, const Options *options);

#endif
