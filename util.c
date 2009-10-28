
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


// FIXME don't leave zeros ("1.0000....") of tail of ascii.
void
dtoa(Digit *digit, char *ascii, size_t max_size, int base)
{
    double val;
    UNUSED(base);

    val = digit2double(digit);
    if (snprintf(ascii, max_size, "%f", val) < 0)
        DIE2("can't convert '%f' to string", val);
}
