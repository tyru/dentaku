#include "dentaku.h"
#include <assert.h>


int
main(int argc, char *argv[])
{
    Dentaku *d;
    JMP_BUF jbuf;

    d = dentaku_alloc();
    dentaku_init(d);
    dentaku_getopt(d, argc, argv);

    while (1) {
        // Free previously allocated tokens.
        dentaku_free_alloc_tokens(d);
        // Clear current stack.
        dentaku_clear_stack(d);

        if (! dentaku_read_src(d))
            break;    // EOF

        switch (sigsetjmp(jbuf, 1)) {
        case 0:
            if (! dentaku_register_main_cont(d, &jbuf)) {
                fputs("can't register jmp_buf", stderr);
                dentaku_destroy(d);
                return -1;
            }
            // Evaluation
            dentaku_dispatch(d);

        case JMP_RET_OK:
            dentaku_show_result(d);
            break;
        }
    }

    dentaku_destroy(d);
    return 0;
}
