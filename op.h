#ifndef DENTAKU_OP_H
#define DENTAKU_OP_H

#include "common.h"



Digit
op_plus(Digit *n, Digit *m);

Digit
op_minus(Digit *n, Digit *m);

Digit
op_multiply(Digit *n, Digit *m);

Digit
op_divide(Digit *n, Digit *m);

Digit
op_power(Digit *n, Digit *m);

Digit
op_unary_minus(Digit *n);



#endif /* DENTAKU_OP_H */
