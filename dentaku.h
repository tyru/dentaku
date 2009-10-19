#ifndef DENTAKU_H
#define DENTAKU_H



#include "common.h"

#include "token.h"
#include "mylib/stack.h"






typedef struct {
    Stack   *cur_stack;    // easier to access
    Stack   cur_stack__;

    FILE    *f_in;
    FILE    *f_out;
    FILE    *f_err;

    char    *src;
    int     src_pos;
    int     src_len;
} Dentaku;




stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok);

stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok);

bool
dentaku_src_eof(Dentaku *dentaku);

bool
dentaku_calc_expr(Dentaku *dentaku, Token *tok_result, bool *done);

bool
dentaku_eval_expr(Dentaku *dentaku, bool *done_eval_expr);

Token*
dentaku_get_token(Dentaku *dentaku, bool *got_new_token);



void
dentaku_init(Dentaku *dentaku);

void
dentaku_alloc(Dentaku *dentaku, size_t stack_size);

void
dentaku_destroy(Dentaku *dentaku);



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
