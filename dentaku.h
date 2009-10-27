#ifndef DENTAKU_DENTAKU_H
#define DENTAKU_DENTAKU_H



#include "common.h"

// TODO Hide these headers
#include "token.h"
#include "libdatastruct/stack.h"
#include "mylib/list/list.h"

#include <setjmp.h>




typedef struct {
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
