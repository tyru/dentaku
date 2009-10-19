
#include "util.h"

#include <stdarg.h>




void
warn(int line, const char *fmt, ...)
{
    va_list ap;

    UNUSED(line);

    fputs("[warning]::", stderr);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    fflush(stderr);
}


void
die(int line, const char *msg)
{
    fprintf(stderr, "[error]::[%s] at %d\n", msg, line);
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


// on success, return true.
bool
atod(const char *digit_str, Digit *digit, int base)
{
    char *end_ptr;
    double val;

    UNUSED(base);

    errno = 0;
    val = strtod(digit_str, &end_ptr);

    if (errno == ERANGE || (errno != 0 && val == 0)) {
        return false;
    }
    if (end_ptr == digit_str) {
        return false;
    }
    if (*end_ptr != '\0') {
        return false;
    }

    return double2digit(val, digit);
}


// on success, return true.
// FIXME don't leave zeros ("1.0000....") of tail of ascii.
bool
dtoa(Digit *digit, char *ascii, size_t max_size, int base)
{
    double val;
    UNUSED(base);

    val = digit2double(digit);
    snprintf(ascii, max_size, "%f", val);
    return double2digit(val, digit);
}
