#ifndef DENTAKU_COMMON_H
#define DENTAKU_COMMON_H

// TODO Hide headers as possible


// #define NDEBUG


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdbool.h>



typedef struct Dentaku_tag      Dentaku;
typedef struct Token_tag        Token;
typedef struct List_tag         List;

typedef unsigned int            stack_ret;

#include <setjmp.h>
#define JMP_BUF                 sigjmp_buf

#include <gmp.h>
typedef mpf_t                   Digit;




#define PROMPT_STR          "=> "
#define DENTAKU_PROG_NAME   "dentaku"

#define MAX_IN_BUF          1024
#define MAX_TOK_CHAR_BUF    32
#define MAX_STACK_SIZE      128

#define JMP_RET_OK          1
#define JMP_RET_ERR         2




#define STREQ(s1, s2)       (*(s1) == *(s2) && strcmp((s1), (s2)) == 0)
#define UNUSED(x)           ((void)x)




#ifdef __GNUC__
    #define NORETURN __attribute__((__noreturn__))
#else
    #define NORETURN
#endif


#endif /* DENTAKU_COMMON_H */
