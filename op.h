#ifndef DENTAKU_OP_H
#define DENTAKU_OP_H

#include "common.h"


void
op_plus(Digit *result, Digit *n, Digit *m);

void
op_minus(Digit *result, Digit *n, Digit *m);

void
op_multiply(Digit *result, Digit *n, Digit *m);

void
op_divide(Digit *result, Digit *n, Digit *m);

void
op_power(Digit *result, Digit *n, Digit *m);

void
op_unary_minus(Digit *result, Digit *n);



#endif /* DENTAKU_OP_H */
