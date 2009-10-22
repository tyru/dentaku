#ifndef COMMON_H
#define COMMON_H



// #define NDEBUG

// #define _GNU_SOURCE


#include "digit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdbool.h>

#include <assert.h>



typedef unsigned int    stack_ret;

// from libdatastruct/stack.c
#define __void_pointer_addition(pointer,number) \
    ((void *)((char *)pointer+number))

#define __refer_by_offset(stack,offset) \
    (__void_pointer_addition((stack)->array,(stack)->element_size*(offset)))

#define __refer_by_offset_from_bottom(stack,offset) \
    (__refer_by_offset((stack),(offset)))

#define __refer_by_offset_from_top(stack,offset) \
    (__refer_by_offset((stack),stack_size(stack)-1-(offset)))

#define __refer_bottom(stack) \
    ((stack)->array)

#define refer_top(stack) \
    __refer_by_offset((stack),(stack)->size-1)




#define PROMPT_STR          "=> "
#define DENTAKU_PROG_NAME   "dentaku"

#define MAX_IN_BUF          1024
#define MAX_TOK_CHAR_BUF    32
#define MAX_STACK_SIZE      128

#define JMP_RET_OK          1
#define JMP_RET_ERR         2


#define STREQ(s1, s2)       (*(s1) == *(s2) && strcmp((s1), (s2)) == 0)
#define ALLOCATED(ptr)      ((ptr) != NULL && errno != ENOMEM)
#define UNUSED(x)           ((void)x)



#ifdef __GNUC__
    #define NORETURN __attribute__((__noreturn__))
#else
    #define NORETURN
#endif


#endif /* COMMON_H */
