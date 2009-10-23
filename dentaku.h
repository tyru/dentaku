#ifndef DENTAKU_H
#define DENTAKU_H



#include "common.h"

#include "token.h"
#include "libdatastruct/stack.h"

#include <setjmp.h>




typedef enum {
    ARG_RPN_NOP = 0,
    ARG_RPN_TO_RPN,
    ARG_RPN_FROM_RPN,
} ArgRPN;


typedef struct {
    stack_t *data_stack;

    sigjmp_buf  *main_jmp_buf;

    FILE    *f_in;
    FILE    *f_out;
    FILE    *f_err;

    char    *src;
    int     src_pos;
    int     src_len;

    // for arguments
    char    *arg_f;
    ArgRPN  arg_rpn;
    bool    debug;
} Dentaku;





void
dentaku_show_stack(Dentaku *dentaku);




stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok);

stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok);

bool
dentaku_src_eof(Dentaku *dentaku);

void
dentaku_calc_expr(Dentaku *dentaku);

void
dentaku_get_token(Dentaku *dentaku);



void
dentaku_init(Dentaku *dentaku);

void
dentaku_alloc(Dentaku *dentaku);

void
dentaku_destroy(Dentaku *dentaku);



NORETURN void
dentaku_exit(Dentaku *dentaku, int status);



void
dentaku_getopt(Dentaku *dentaku, int argc, char **argv);



bool
dentaku_read_src(Dentaku *dentaku);

NORETURN void
dentaku_eval_src(Dentaku *dentaku);

void
dentaku_clear_stack(Dentaku *dentaku);

bool
dentaku_register_main_cont(Dentaku *dentaku, sigjmp_buf *cont);

void
dentaku_show_result(Dentaku *dentaku);


#endif /* DENTAKU_H */
