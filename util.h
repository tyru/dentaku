#ifndef UTIL_H
#define UTIL_H

#include "common.h"




#define WARN(msg)           warn(__LINE__, msg)
#define WARN2(msg, arg1)    warn(__LINE__, msg, arg1)
#define WARN3(msg, arg1, arg2)    warn(__LINE__, msg, arg1, arg2)
#define WARN4(msg, arg1, arg2, arg3)    warn(__LINE__, msg, arg1, arg2, arg3)

void
warn(int line, const char *fmt, ...);


#define DIE(msg)    die(__LINE__, msg)

void
die(int line, const char *msg);



bool
double2digit(double val, Digit *digit);

double
digit2double(Digit *digit);



bool
atod(const char *digit_str, Digit *digit, int base);

bool
dtoa(Digit *digit, char *ascii, size_t max_size, int base);


#endif /* UTIL_H */
