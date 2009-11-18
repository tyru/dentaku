#ifndef DENTAKU_UTIL_H
#define DENTAKU_UTIL_H

#include "common.h"



#define WARN(msg) \
    warn(msg)
#define WARN2(msg, arg1) \
    warn(msg, arg1)
#define WARN3(msg, arg1, arg2) \
    warn(msg, arg1, arg2)
#define WARN4(msg, arg1, arg2, arg3) \
    warn(msg, arg1, arg2, arg3)
#define WARN5(msg, arg1, arg2, arg3, arg4) \
    warn(msg, arg1, arg2, arg3, arg4)

void
warn(const char *fmt, ...);


#define DIE(msg) \
    die(__FILE__, __LINE__, msg)
#define DIE2(msg, arg1) \
    die(__FILE__, __LINE__, msg, arg1)
#define DIE3(msg, arg1, arg2) \
    die(__FILE__, __LINE__, msg, arg1, arg2)
#define DIE4(msg, arg1, arg2, arg3) \
    die(__FILE__, __LINE__, msg, arg1, arg2, arg3)

NORETURN void
die(const char *filename, int line, const char *msg, ...);




void
token2digit(Digit *digit, Token *tok, int base);

void
digit2token(Token *tok, Digit *digit, size_t max_size, int base);

void
double2digit(Digit *dest, double src);

double
digit2double(Digit *src);

unsigned long
digit2ulong(Digit *src);




char*
strchr_not(const char *str, int c);

char*
last_chr(const char *str);


#endif /* DENTAKU_UTIL_H */
