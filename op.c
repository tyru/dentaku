/*
 * TODO
 * - Return Digit* after making GC
 */

#include "op.h"

#include "util.h"
#include <math.h>




Digit
op_plus(Digit *n, Digit *m)
{
    Digit result;
    bool success = double2digit(digit2double(n) + digit2double(m), &result);
    if (! success)
        DIE("can't convert double to digit");
    return result;
}


Digit
op_minus(Digit *n, Digit *m)
{
    Digit result;
    bool success = double2digit(digit2double(n) - digit2double(m), &result);
    if (! success)
        DIE("can't convert double to digit");
    return result;
}


Digit
op_multiply(Digit *n, Digit *m)
{
    Digit result;
    bool success = double2digit(digit2double(n) * digit2double(m), &result);
    if (! success)
        DIE("can't convert double to digit");
    return result;
}


Digit
op_divide(Digit *n, Digit *m)
{
    Digit result;
    bool success = double2digit(digit2double(n) / digit2double(m), &result);
    if (! success)
        DIE("can't convert double to digit");
    return result;
}


Digit
op_power(Digit *n, Digit *m)
{
    Digit result;
    bool success = double2digit(pow(digit2double(n), digit2double(m)), &result);
    if (! success)
        DIE("can't convert double to digit");
    return result;
}


Digit
op_unary_minus(Digit *n)
{
    Digit result;
    bool success = double2digit(-digit2double(n), &result);
    if (! success)
        DIE("can't convert double to digit");
    return result;
}
