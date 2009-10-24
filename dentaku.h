#ifndef DENTAKU_DENTAKU_H
#define DENTAKU_DENTAKU_H



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




/* for debug */
void
dentaku_printf_d(Dentaku *dentaku, const char *fmt, ...);

void
dentaku_show_stack(Dentaku *dentaku);



NORETURN void
dentaku_die(Dentaku *dentaku, const char *errmsg);

NORETURN void
dentaku_dief(Dentaku *dentaku, const char *fmt, ...);

NORETURN void
dentaku_exit(Dentaku *dentaku, int status);



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

void
dentaku_getopt(Dentaku *dentaku, int argc, char **argv);

bool
dentaku_read_src(Dentaku *dentaku);

NORETURN void
dentaku_dispatch(Dentaku *dentaku);

void
dentaku_clear_stack(Dentaku *dentaku);

bool
dentaku_register_main_cont(Dentaku *dentaku, sigjmp_buf *cont);

void
dentaku_show_result(Dentaku *dentaku);


#endif /* DENTAKU_DENTAKU_H */
