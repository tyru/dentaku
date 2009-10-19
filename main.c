#include "dentaku.h"


int
main(int argc, char *argv[])
{
    Dentaku dentaku;
    Dentaku *d = &dentaku;

    dentaku_init(d);
    dentaku_alloc(d, MAX_STACK_SIZE);
    dentaku_getopt(d, argc, argv);
    return dentaku_main(d);
}
