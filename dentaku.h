#ifndef DENTAKU_DENTAKU_H
#define DENTAKU_DENTAKU_H

#include "common.h"
#include <setjmp.h>





/* Public Functions */

NORETURN void
dentaku_die(Dentaku *dentaku, const char *errmsg);

NORETURN void
dentaku_dief(Dentaku *dentaku, const char *fmt, ...);

NORETURN void
dentaku_exit(Dentaku *dentaku, int status);



Dentaku*
dentaku_alloc(void);

void
dentaku_init(Dentaku *dentaku);

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

void
dentaku_free_alloc_tokens(Dentaku *dentaku);

bool
dentaku_register_main_cont(Dentaku *dentaku, sigjmp_buf *cont);

void
dentaku_show_result(Dentaku *dentaku);




#endif /* DENTAKU_DENTAKU_H */
