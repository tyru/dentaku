#ifndef DENTAKU_DIGIT_H
#define DENTAKU_DIGIT_H

#include "common.h"



void
digit_init(Digit *d);

Digit*
digit_copy(Digit *dest, Digit *src);

Digit*
digit_destroy(Digit *d);



#endif /* DENTAKU_DIGIT_H */
