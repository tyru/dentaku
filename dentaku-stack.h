#ifndef DENTAKU_DENTAKU_STACK_H
#define DENTAKU_DENTAKU_STACK_H

#include "common.h"

#include "dentaku.h"



NORETURN void
dentaku_stack_run(Dentaku *dentaku);

void
dentaku_push_got_token(Dentaku *dentaku);



#endif /* DENTAKU_DENTAKU_STACK_H */
