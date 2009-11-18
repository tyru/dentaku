#include "digit.h"
#include <gmp.h>
#include <string.h>


void
digit_init(Digit *d)
{
    mpf_init2(*d, DIGIT_PREC_BITS);
}


Digit*
digit_copy(Digit *dest, Digit *src)
{
    memmove(*dest, *src, sizeof(Digit));
    return dest;
}


void
digit_destroy(Digit *d)
{
    mpf_clear(*d);
}
