#include "dentaku-parser.h"
#include "dentaku-core.h"

#define stack_t     stack_t__
    #include "libdatastruct/stack.h"
#undef stack_t



NORETURN void
dentaku_parser_run(Dentaku *dentaku)
{
    extern int yyparse(void);
    extern FILE *yyin;
    extern stack_t *parser_result_stack;

    FILE *tmp_in = tmpfile();
    if (! tmp_in) {
        perror("tmpfile");
        dentaku_die(dentaku, "error: tmpfile()");
    }
    if (fwrite(dentaku->src, 1, dentaku->src_len, tmp_in) < dentaku->src_len) {
        dentaku_die(dentaku, "can't write input to temporary file");
    }
    rewind(tmp_in);

    yyin = tmp_in;
    parser_result_stack = dentaku->data_stack;
    if (yyparse()) {
        // dentaku_die() calls exit()
        // So don't have to close tmp_in manually.
        dentaku_die(dentaku,
                    "error: yyparse(): "
                    "something wrong happened at yyparse()\n");
    }
    fclose(tmp_in);

    if (stack_empty(dentaku->data_stack)) {
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else {
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
    }
}
