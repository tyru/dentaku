
#include "util.h"
#include "token.h"
#include "digit.h"

#include <stdarg.h>
#include <assert.h>
#include <gmp.h>




/*
 * Show the warning message.
 */
void
warn(const char *fmt, ...)
{
    va_list ap;

    fputs("[warning]::", stderr);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    fflush(stderr);
}


/*
 * Internal Error.
 */
NORETURN void
die(const char *filename, int line, const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr,
            "\n\ninternal error at %s, line %d.\n",
            filename, line);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}




/*******************************
 * Conversion Functions        *
 *******************************/

void
token2digit(Token *tok, Digit *digit, int base)
{
    mpf_init_set_str(*digit, tok->str, base);
}


void
digit2token(Digit *digit, Token *tok, size_t max_size, int base)
{
    mp_exp_t exp;
    mpf_get_str(tok->str, &exp, base, max_size, *digit);
    if (exp == 0) {
        // From gmplib document:
        // When digit is zero, an empty string is produced
        // and the exponent returned is 0.
        assert(max_size >= 2);
        strcpy(tok->str, "0");
    }
}




/*******************************
 * Util Functions              *
 *******************************/

/*
 * Return pointer to first character which is NOT c of str.
 * Return NULL if all characters are c or strlen(str) == 0.
 */
char*
strchr_not(const char *str, int c)
{
    for (; *str; str++)
        if (*str != c)
            return (char*)str;
    return NULL;
}


/*
 * Return pointer to last character of str.
 * Return NULL if strlen(str) == 0.
 */
char*
last_chr(const char *str)
{
    for (; *str; str++)
        if (str[1] == '\0')
            return (char*)str;
    return NULL;
}
