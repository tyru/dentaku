
#include "util.h"

#include <stdarg.h>




void
d_printf(const char *fmt, ...)
{
#if NDEBUG
    va_list ap;

    fputs("[debug]::", stderr);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    fflush(stderr);
#else
    UNUSED(fmt);
#endif
}


void
warn(int line, const char *fmt, ...)
{
    va_list ap;

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


// almost code from 'man 3 strtol'.
// if failed, *failed is not NULL.
digit_t
atod(const char *digit_str, int base, char **failed)
{
    char *end_ptr;
    digit_t val;

    UNUSED(base);

    errno = 0;
    val = strtod(digit_str, &end_ptr);

    if (errno == ERANGE || (errno != 0 && val == 0)) {
        *failed = (char*)digit_str;
        return (digit_t)0;
    }
    if (end_ptr == digit_str) {
        *failed = (char*)digit_str;
        return (digit_t)0;
    }
    if (*end_ptr != '\0') {
        *failed = (char*)digit_str;
        return (digit_t)0;
    }

    return val;
}


// on success, return true.
bool
dtoa(digit_t digit, char *ascii, size_t max_size, int base)
{
    UNUSED(base);

    snprintf(ascii, max_size, "%f", digit);
    return true;
}
