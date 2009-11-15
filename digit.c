#include "digit.h"
#include <gmp.h>
#include <string.h>


void
digit_init(Digit *d)
{
    mpf_init2(*d);
}


Digit*
digit_copy(Digit *dest, Digit *src)
{
    memmove(*dest, *src, sizeof(Digit));
    return dest;
}
