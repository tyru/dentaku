#ifndef COMMON_H
#define COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdbool.h>



typedef double digit_t;


#define NDEBUG 1



#define PROMPT_STR  "=> "
#define MAX_IN_BUF          1024
#define MAX_TOK_CHAR_BUF    32
#define MAX_STACK_SIZE      128


#define STREQ(s1, s2)       (*(s1) == *(s2) && strcmp((s1), (s2)) == 0)
#define ALLOCATED(ptr)      ((ptr) != NULL && errno != ENOMEM)
#define UNUSED(x)           ((void)x)


#endif /* COMMON_H */
