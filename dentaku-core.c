/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku-core.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-11-19.
 *
 */


/* TODO && FIXME
 * - use GC
 * - add more ops. (e.g.: 'log', 'exp')
 * - too ugly
 */


#include "dentaku-core.h"

#include "libdatastruct/stack.h"

#include "dentaku-stack.h"
#include "dentaku-parser.h"
#include "dentaku-recursion.h"

#include "util.h"
#include "op.h"
#include "alloc-list.h"
#include "token.h"
#include "digit.h"

#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>





void
dentaku_printf_d(Dentaku *dentaku, const char *fmt, ...)
{
    va_list ap;

    if (! dentaku->debug)
        return;

    fputs("[debug]::", stderr);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    fflush(stderr);
}


// for debug and fun.
void
dentaku_show_stack(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    size_t stk_len = stack_size(stk);

    if (sizeof(stack_t) * stk_len > MAX_ALLOCA_SIZE) {
        dentaku_printf_d(dentaku,
                "too large number to do alloca()."
                " skip showing debug message...");
        return;
    }
    Token *tokens = alloca(sizeof(stack_t) * stk_len);

    if (! dentaku->debug)
        return;

    dentaku_printf_d(dentaku, "show all stack...");
    if (stk_len == 0) {
        dentaku_printf_d(dentaku, "  ...no tokens on stack.");
        return;
    }
    // push all current stack to tokens.
    while (!stack_empty(stk)) {
        size_t cur_pos = stack_size(stk) - 1;
        stack_pop(stk, tokens + cur_pos);
        dentaku_printf_d(dentaku, "  pop...  %d: [%s]",
                        cur_pos, tokens[cur_pos].str);
    }

    // restore
    while (stack_size(stk) < stk_len) {
        stack_push(stk, tokens + stack_size(stk));
    }

    fflush(stderr);
}




NORETURN void
dentaku_die(Dentaku *dentaku, const char *errmsg)
{
    if (errmsg)
        fputs(errmsg, stderr);
    dentaku_exit(dentaku, EXIT_FAILURE);
}


NORETURN void
dentaku_dief(Dentaku *dentaku, const char *fmt, ...)
{
    va_list ap;
    if (fmt) {
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    dentaku_exit(dentaku, EXIT_FAILURE);
}


NORETURN void
dentaku_exit(Dentaku *dentaku, int status)
{
    dentaku_destroy(dentaku);
    exit(status);
}





bool
dentaku_src_eof(Dentaku *dentaku)
{
    return dentaku->src_pos >= dentaku->src_len;
}


/*
 * Calculate expression.
 * Result token is tok_result.
 */
bool
dentaku_calc_expr(Dentaku *dentaku, Token *tok_op, Token *tok_n, Token *tok_m, Token *tok_result)
{
    Digit n, m, result;

    if (dentaku->debug) {
        dentaku_printf_d(dentaku, "before calculation");
        dentaku_show_stack(dentaku);
    }

    /* Check each token's type */
    if (! (tok_n->type == TOK_DIGIT
        && TOKEN_IS_OPERATOR(*tok_op)
        && tok_m->type == TOK_DIGIT))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n->str, tok_op->str, tok_m->str);
        goto error;
    }

    /* Convert */
    token2digit(&n, tok_n, 10);
    token2digit(&m, tok_m, 10);

    /* Calculation */
    // TODO Use table for ops
    digit_init(&result);
    switch (*tok_op->str) {
    case '+': op_plus     (&result, &n, &m); break;
    case '-': op_minus    (&result, &n, &m); break;
    case '*': op_multiply (&result, &n, &m); break;
    case '/': op_divide   (&result, &n, &m); break;
    case '%': op_modulo   (&result, &n, &m); break;
    case '^': op_power    (&result, &n, &m); break;
    default:
        WARN2("unknown op '%s'", tok_op->str);
        goto error;
    }


    /* Convert result to Token */
    digit2token(tok_result, &result, MAX_TOK_CHAR_BUF, 10);
    tok_result->type = TOK_DIGIT;

    dentaku_printf_d(dentaku, "eval '%s %s %s' => '%s'",
        tok_n->str, tok_op->str, tok_m->str, tok_result->str);

    digit_destroy(&n);
    digit_destroy(&m);
    digit_destroy(&result);
    return true;
error:
    digit_destroy(&n);
    digit_destroy(&m);
    digit_destroy(&result);
    return false;
}





Dentaku*
dentaku_alloc(void)
{
    Dentaku *dentaku = malloc(sizeof(Dentaku));
    if (! dentaku) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return dentaku;
}


static void
allocate_members(Dentaku *dentaku)
{
    dentaku_printf_d(dentaku, "allocating dentaku...");

    dentaku->data_stack = stack_initialize(
        sizeof(Token),
        DEFAULT_RELEASE_FUNCTION,
        (void *(*)(void*, const void*, size_t))token_copy
    );
    if (! dentaku->data_stack) {
        DIE("failed to initialize stack");
    }

    dentaku->src = malloc(MAX_IN_BUF);
    if (! dentaku->src) {
        DIE("failed to allocate for input string");
    }
}


void
dentaku_init(Dentaku *dentaku)
{
    dentaku_printf_d(dentaku, "initializing dentaku...");

    dentaku->data_stack = NULL;
    dentaku->syntax_tree = NULL;

    dentaku->main_jmp_buf = NULL;

    dentaku->f_in  = stdin;
    dentaku->f_out = stdout;
    dentaku->f_err = stderr;

    dentaku->src = NULL;
    dentaku->src_len = -1;
    dentaku->src_pos = -1;

    // See what this does at show_usage().
    dentaku->arg_f = "stack";
    dentaku->quiet = false;
    dentaku->debug = false;


    allocate_members(dentaku);
    al_init();
}


void
dentaku_destroy(Dentaku *dentaku)
{
    dentaku_printf_d(dentaku, "destroying dentaku...");

    // dentaku->data_stack
    stack_release(dentaku->data_stack);
    // dentaku->src
    free(dentaku->src);
    dentaku->src = NULL;
    // dentaku
    free(dentaku);
    // free() all allocated pointers
    al_destroy();
}




static void
show_usage(void)
{
    puts("");
    puts("Usage: " DENTAKU_PROG_NAME " [OPTIONS] [--] [file]");
    puts("");
    puts("  --help");
    puts("    show this help");
    puts("  --debug");
    puts("    show debug message");
    puts("  -f [rec,stk,cmp,rpn]");
    puts("    change internal behavior until dentaku shows answer");
    puts("");
    puts("F_OPTIONS");
    puts("  recursion");
    puts("    calculate using recursion");
    puts("  stack");
    puts("    calculate using stack");
    puts("  compile");
    puts("    compile and calculate");
    // puts("  rpn");
    // puts("    show RPN expression");
    puts("  parser");
    puts("    calculate using yacc & lex");
}
static void
validate_arg_f(Dentaku *dentaku)
{
    const char *arg_f = dentaku->arg_f;
    STREQ(arg_f, "recursion") ?
    0
    : STREQ(arg_f, "stack") ?
    0
    : STREQ(arg_f, "compile") ?
    0
    : STREQ(arg_f, "rpn") ?
    0
    : STREQ(arg_f, "parser") ?
    0
    : dentaku_dief(dentaku, "Unknown -f option: %s", arg_f)
    ;
}
void
dentaku_getopt(Dentaku *dentaku, int argc, char **argv)
{
    static const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"debug", 0, NULL, 'd'},
        {"quiet", 0, NULL, 'q'},
        {0, 0, 0, 0}
    };
    int opt_index = 0;
    int c;

    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((c = getopt_long(argc, argv, "hdf:q", long_opts, &opt_index)) != -1) {
        switch (c) {
            case 'h':
                show_usage();
                dentaku_exit(dentaku, EXIT_SUCCESS);
                break;
            case 'd':
                dentaku->debug = true;
                break;
            case 'f':
                dentaku->arg_f = optarg;
                validate_arg_f(dentaku);
                break;
            case 'q':
                dentaku->quiet = true;
                break;
            case '?':
                show_usage();
                dentaku_exit(dentaku, EXIT_FAILURE);
                break;
        }
    }


    // TODO get filename and set to dentaku->f_in, dentaku->f_out, dentaku->f_err.
}




bool
dentaku_read_src(Dentaku *dentaku)
{
    char buf[MAX_IN_BUF];
    size_t read_size;

    if (isatty(fileno(dentaku->f_in)) && isatty(fileno(dentaku->f_out))) {
        if (! dentaku->quiet) {
            fputs(PROMPT_STR, dentaku->f_out);
            fflush(dentaku->f_out);
        }
        // read each line
        if (fgets(buf, MAX_IN_BUF, dentaku->f_in) == NULL)
            return false;
    }
    else {
        memset(buf, 0, MAX_IN_BUF);

        do {
            // TODO when length > MAX_IN_BUF
            read_size = fread(buf, 1, MAX_IN_BUF, dentaku->f_in);

            if (ferror(dentaku->f_in)) {
                perror("fread");
                dentaku_exit(dentaku, EXIT_FAILURE);
            }
        } while (! (feof(dentaku->f_in) || read_size < MAX_IN_BUF));
    }

    strncpy(dentaku->src, buf, MAX_IN_BUF);
    dentaku_printf_d(dentaku, "read! [%s]", dentaku->src);

    dentaku->src_len = strlen(dentaku->src);
    dentaku->src_pos = 0;

    if (feof(dentaku->f_in) && dentaku->src_len == 0)
        return false;
    return true;
}




NORETURN void
dentaku_dispatch(Dentaku *dentaku)
{
    assert(dentaku->main_jmp_buf);

    if (STREQ(dentaku->arg_f, "recursion")) {
        dentaku_recursion_run(dentaku);
    }
    else if (STREQ(dentaku->arg_f, "stack")) {
        dentaku_stack_run(dentaku);
    }
    else if (STREQ(dentaku->arg_f, "compile")) {
        // TODO
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else if (STREQ(dentaku->arg_f, "rpn")) {
        // TODO
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else if (STREQ(dentaku->arg_f, "parser")) {
        dentaku_parser_run(dentaku);
    }
    else {
        DIE("internal error: check -f option's value!");
    }
}





void
dentaku_clear_stack(Dentaku *dentaku)
{
    stack_clear(dentaku->data_stack);
}


void
dentaku_free_alloc_tokens(Dentaku *dentaku)
{
    al_free_pointers();
}


bool
dentaku_register_main_cont(Dentaku *dentaku, sigjmp_buf *cont)
{
    dentaku->main_jmp_buf = cont;
    return true;
}


void
dentaku_show_result(Dentaku *dentaku)
{
    Token top;
    if (stack_top(dentaku->data_stack, &top) == STACK_SUCCESS) {
        puts(top.str);
    }
}

