#include "dentaku.h"


int
main(int argc, char *argv[])
{
    Dentaku dentaku;
    Dentaku *d = &dentaku;
    sigjmp_buf jbuf;

    dentaku_init(d);
    dentaku_alloc(d);
    dentaku_getopt(d, argc, argv);


    while (1) {
        dentaku_clear_stack(d);

        if (! dentaku_read_src(d))
            break;    // EOF

        switch (sigsetjmp(jbuf, 1)) {
        case 0:
            // set jbuf
            if (! dentaku_register_main_cont(d, &jbuf)) {
                fputs("can't register jmp_buf", stderr);
                dentaku_destroy(d);
                return EXIT_FAILURE;
            }
            // evaluation
            dentaku_eval_src(d);

            /* NOTREACHED */
            assert(0);

        case JMP_RET_OK:
            // evaluation has done.
            dentaku_show_result(d);
            break;
        }
    }

    dentaku_destroy(d);
    return EXIT_SUCCESS;
}
