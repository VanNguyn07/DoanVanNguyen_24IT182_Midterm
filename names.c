#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "names.h"

int print_name(const char *name, const Options *options)
{
    const char *cursor = name;
    size_t remaining = strlen(name);
    mbstate_t state;

    if (!options->quote_nonprintable) {
        return fputs(name, stdout) == EOF ? EOF : 0;
    }

    memset(&state, 0, sizeof(state));

    while (remaining > 0) {
        wchar_t character;
        size_t length;
        int printable;

        length = mbrtowc(&character, cursor, remaining, &state);

        if (length == (size_t)-1 || length == (size_t)-2) {
            /* Replace an invalid byte and restart character decoding. */
            length = 1;
            printable = 0;
            memset(&state, 0, sizeof(state));
        } else if (length == 0) {
            break;
        } else {
            printable = iswprint(character);
        }

        if (printable) {
            if (fwrite(cursor, 1, length, stdout) != length) {
                return EOF;
            }
        } else if (putchar('?') == EOF) {
            return EOF;
        }

        cursor += length;
        remaining -= length;
    }

    return 0;
}

int print_name_line(const char *name, const Options *options)
{
    if (print_name(name, options) == EOF ||
        putchar('\n') == EOF) {
        return EOF;
    }

    return 0;
}
