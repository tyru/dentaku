#include "dentaku.h"


int
main(int argc, char *argv[])
{
    Dentaku dentaku;

    dentaku_init(&dentaku);
    dentaku_alloc(&dentaku, MAX_STACK_SIZE);
    dentaku_getopt(&dentaku, argc, argv);

    return dentaku_main(&dentaku);
}
