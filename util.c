
#include "util.h"
#include "token.h"
#include "digit.h"
#include "alloc-list.h"

#include <stdarg.h>
#include <assert.h>
#include <alloca.h>
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
token2digit(Digit *digit, Token *tok, int base)
{
    mpf_init_set_str(*digit, tok->str, base);
}


void
digit2token(Token *tok, Digit *digit, size_t max_size, int base)
{
    mp_exp_t exp;
    mpf_get_str(tok->str, &exp, base, max_size, *digit);
    size_t unsigned_len;

    if (tok->str[0] == '+' || tok->str[0] == '-') {
        unsigned_len = strlen(&tok->str[1]);
    }
    else {
        unsigned_len = strlen(tok->str);
    }

    if (tok->str[0] == '\0') {
        // From gmplib document:
        // When digit is zero, an empty string is produced
        // and the exponent returned is 0.
        assert(max_size >= 2);
        strcpy(tok->str, "0");
    }
    else if ((size_t)exp > unsigned_len) {
        // Add zeroes of exp to tail of string.
        for (; unsigned_len < exp; unsigned_len++) {
            tok->str[unsigned_len] = '0';
        }
        tok->str[exp] = '\0';
    }
    else if ((size_t)exp < unsigned_len) {
        // Insert '.' to head/middle of string.
        char *lower = ALLOCA(&tok->str[unsigned_len] - &tok->str[exp] + 1);
        strcpy(lower, &tok->str[exp]);
        if (exp == 0) {
            tok->str[exp] = '0';
            exp++;
        }
        tok->str[exp] = '.';
        tok->str[exp + 1] = '\0';
        strcpy(&tok->str[exp + 1], lower);
    }
}


void
double2digit(Digit *dest, double src)
{
    mpf_set_d(*dest, src);
}


double
digit2double(Digit *src)
{
    return mpf_get_d(*src);
}


unsigned long
digit2ulong(Digit *src)
{
    return mpf_get_ui(*src);
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
