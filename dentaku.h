#ifndef DENTAKU_H
#define DENTAKU_H



#include "common.h"

#include "token.h"
#include "mylib/stack.h"





typedef enum {
    ARG_RPN_NOP = 0,
    ARG_RPN_TO_RPN,
    ARG_RPN_FROM_RPN,
} ArgRPN;


typedef struct {
    Stack   *cur_stack;    // easier to access
    Stack   cur_stack__;

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




stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok);

stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok);

bool
dentaku_src_eof(Dentaku *dentaku);

Token*
dentaku_calc_expr(Dentaku *dentaku, bool *no_op);

Token*
dentaku_get_token(Dentaku *dentaku, bool *got_new_token);



void
dentaku_init(Dentaku *dentaku);

void
dentaku_alloc(Dentaku *dentaku, size_t stack_size);

void
dentaku_destroy(Dentaku *dentaku);



void
dentaku_exit(Dentaku *dentaku, int status);



void
dentaku_getopt(Dentaku *dentaku, int argc, char **argv);



bool
dentaku_read_src(Dentaku *dentaku);

bool
dentaku_eval_src(Dentaku *dentaku);

void
dentaku_clear_stack(Dentaku *dentaku);

void
dentaku_show_result(Dentaku *dentaku);

void
dentaku_show_stack(Dentaku *dentaku);


#endif /* DENTAKU_H */
