
#include "util.h"

#include <stdarg.h>




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


// Note that this does not call dentaku_destroy().
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




/*
 * Conversion Functions
 */

// on success, return true.
bool
double2digit(double val, Digit *digit)
{
    digit->i = (int)val;
    digit->d = val - (double)digit->i;
    return true;
}

double
digit2double(Digit *digit)
{
    return (double)digit->i + digit->d;
}


void
atod(const char *digit_str, Digit *digit, int base)
{
    char *end_ptr;
    double val;

    UNUSED(base);

    errno = 0;
    val = strtod(digit_str, &end_ptr);

    if (errno == ERANGE || (errno != 0 && val == 0)) {
        DIE2("can't convert '%s' to digit", digit_str);
    }
    if (end_ptr == digit_str) {
        DIE2("can't convert '%s' to digit", digit_str);
    }
    if (*end_ptr != '\0') {
        DIE2("can't convert '%s' to digit", digit_str);
    }

    if (! double2digit(val, digit))
        DIE2("can't convert '%s' to digit", digit_str);
}


void
dtoa(Digit *digit, char *ascii, size_t max_size, int base)
{
    double val;
    UNUSED(base);

    val = digit2double(digit);
    if (snprintf(ascii, max_size, "%f", val) < 0)
        DIE2("can't convert '%f' to string", val);

    // 1.00000 -> 1
    char *head_zero = ascii;
    // fprintf(stderr, "begin: ascii [%s]\n", ascii);
    while ((head_zero = strchr(head_zero, '0')) != NULL) {
        // fprintf(stderr, "head_zero [%s]\n", head_zero);
        char *non_zero_pos = strchr_not(head_zero, '0');
        // fprintf(stderr, "non_zero_pos [%s]\n", non_zero_pos);
        if (non_zero_pos == NULL) {
            *head_zero = '\0';
            break;
        }
        else {
            if (non_zero_pos[1] == '\0')    // last is non zero
                break;
            else
                head_zero = non_zero_pos + 1;
            // fprintf(stderr, "next head_zero [%s]\n", head_zero);
        }
    }
    char *last_pos;
    if ((last_pos = last_chr(ascii)) != NULL && *last_pos == '.')
        *last_pos = '\0';
}




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
