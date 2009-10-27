#ifndef DENTAKU_DENTAKU_CORE_H
#define DENTAKU_DENTAKU_CORE_H



#include "common.h"
#include "dentaku.h"    // typedef struct Dentaku_tag Dentaku

// TODO Hide these headers
#include "token.h"
#include "libdatastruct/stack.h"
#include "mylib/list/list.h"
#include <setjmp.h>




struct Dentaku_tag {
    stack_t *data_stack;
    List    *syntax_tree;

    sigjmp_buf  *main_jmp_buf;

    FILE    *f_in;
    FILE    *f_out;
    FILE    *f_err;

    char    *src;
    int     src_pos;
    int     src_len;

    // for arguments
    char    *arg_f;
    bool    debug;
};




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

bool
dentaku_calc_expr(Dentaku *dentaku, Token *tok_op, Token *tok_n, Token *tok_m, Token *tok_result);



Dentaku*
dentaku_alloc();

void
dentaku_init();

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


#endif /* DENTAKU_DENTAKU_CORE_H */
