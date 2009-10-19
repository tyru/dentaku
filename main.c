#include "dentaku.h"


int
main(int argc, char *argv[])
{
    Dentaku dentaku;
    Dentaku *d = &dentaku;

    dentaku_init(d);
    dentaku_alloc(d, MAX_STACK_SIZE);

    while (dentaku_read_src(d)) {
        if (dentaku_eval_src(d)) {
            dentaku_show_result(d);
        }
        dentaku_clear_stack(d);
    }

    dentaku_destroy(d);


    return EXIT_SUCCESS;
}
