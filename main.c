#include "operands.h"
#include "options.h"
#include <locale.h>

int main(int argc, char *argv[])
{
    Options options;
    int first_operand;

    (void)setlocale(LC_ALL, "");

    if (parse_options(argc, argv, &options, &first_operand) != 0) {
        return 1;
    }

    if (first_operand == argc) {
        char *default_paths[] = { "." };

        return list_operands(1, default_paths, &options);
    }

    return list_operands(argc - first_operand,
                         argv + first_operand, &options);
}
