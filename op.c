#include "op.h"

#include "util.h"
#include "digit.h"
#include <math.h>
#include <gmp.h>




void
op_plus(Digit *result, Digit *n, Digit *m)
{
    mpf_add(*result, *n, *m);
}


void
op_minus(Digit *result, Digit *n, Digit *m)
{
    mpf_sub(*result, *n, *m);
}


void
op_multiply(Digit *result, Digit *n, Digit *m)
{
    mpf_mul(*result, *n, *m);
}


void
op_divide(Digit *result, Digit *n, Digit *m)
{
    mpf_div(*result, *n, *m);
}


void
op_power(Digit *result, Digit *n, Digit *m)
{
    mpf_pow_ui(*result, *n, mpf_get_ui(*m));
}


void
op_unary_minus(Digit *result, Digit *n)
{
    Digit tmp;
    digit_init(&tmp);
    mpf_neg(tmp, *n);
    digit_copy(result, &tmp);
}
