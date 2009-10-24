/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-10-25.
 *
 */


/* TODO && FIXME
 * - add .str's capacity size to Token
 *   for allocating just token's characters length.
 *   (if capacity is a fewer than needed size, use realloc())
 * - use GC
 * - add more ops. (e.g.: '^', 'log', 'exp')
 * - too ugly
 */


#include "dentaku.h"

#include "dentaku-eval.h"
#include "util.h"
#include "parser.h"

#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>





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





bool
dentaku_src_eof(Dentaku *dentaku)
{
    return dentaku->src_pos >= dentaku->src_len;
}


/*
 * calculate top 3 elems of stack.
 * result token is tok_result.
 *
 * return result of token.
 * if return value is NULL: error occured
 */
void
dentaku_calc_expr(Dentaku *dentaku)
{
    Token tok_n, tok_op, tok_m, tok_result;
    Digit n, m, result;
    double d_n, d_m;
    bool success;
    stack_t *stk = dentaku->data_stack;
    stack_ret ret;

    if (dentaku->debug) {
        dentaku_printf_d(dentaku, "before calculation");
        dentaku_show_stack(dentaku);
    }
    if (stack_empty(stk)) {
        // don't call when top is NULL!!
        DIE("no more items on stack.");
    }

    token_init(&tok_n);
    token_init(&tok_op);
    token_init(&tok_m);


    // 1st. pop 'm' of expression 'n <op> m'.
    switch (ret = stack_pop(stk, &tok_m)) {
    case STACK_SUCCESS:
        dentaku_printf_d(dentaku, "pop 'm' of 'n <op> m'... [%s]", tok_m.str);
        break;
    default:
        WARN2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }

    // 2nd. pop '<op>' of expression 'n <op> m'.
    // '<op>' or '( <op>'
    if ((ret = stack_pop(stk, &tok_op)) == STACK_EMPTY
     || (ret == STACK_SUCCESS && tok_op.type == TOK_LPAREN))
    {
        if (ret != STACK_EMPTY)
            stack_push(stk, &tok_op);
        stack_push(stk, &tok_m);
        goto just_ret;
    }
    dentaku_printf_d(dentaku, "pop '<op>' of 'n <op> m'... [%s]", tok_op.str);
    if (ret != STACK_SUCCESS) {
        WARN2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }


    // 3rd. pop 'n' of expression 'n <op> m'.
    switch (ret = stack_pop(stk, &tok_n)) {
    case STACK_SUCCESS:
        dentaku_printf_d(dentaku, "pop 'n' of 'n <op> m'... [%s]", tok_n.str);
        break;
    case STACK_EMPTY:
        WARN("reaching EOF where digit is expected");
        goto error;
    default:
        WARN2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }


    /* check each token's type */
    if (! (tok_n.type == TOK_DIGIT
        && tok_op.type == TOK_OP
        && tok_m.type == TOK_DIGIT))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n.str, tok_op.str, tok_m.str);
        goto error;
    }


    /* convert */
    if (! atod(tok_n.str, &n, 10)) {
        WARN2("can't convert '%s' to digit", tok_n.str);
        goto error;
    }
    d_n = digit2double(&n);

    if (! atod(tok_m.str, &m, 10)) {
        WARN2("can't convert '%s' to digit", tok_m.str);
        goto error;
    }
    d_m = (double)m.i + m.d;

    if (stack_top(stk, &tok_result) == STACK_SUCCESS) {
        // fix for the case that '-1-1-1' results in '-1'.
        // 1 - 1 => 0
        // -1 - 0 => -1
        if (tok_result.str[0] == '-') {
            stack_pop(stk, NULL);
            Token tok_plus;
            tok_plus.str = "+";
            tok_plus.type = TOK_OP;
            stack_push(stk, &tok_plus);
            d_n = -d_n;
        }
        token_destroy(&tok_result);
    }

    /* calc */
    switch (*tok_op.str) {
    case '+': success = double2digit(d_n + d_m, &result); break;
    case '-': success = double2digit(d_n - d_m, &result); break;
    case '*': success = double2digit(d_n * d_m, &result); break;
    case '/': success = double2digit(d_n / d_m, &result); break;
    case '^': success = double2digit(pow(d_n, d_m), &result); break;
    default:
        WARN2("unknown op '%s'", tok_op.str);
        goto error;
    }
    if (! success) {
        WARN("failed to convert double to digit");
        goto error;
    }


    /* convert result token */
    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);
    if (! dtoa(&result, tok_result.str, MAX_TOK_CHAR_BUF, 10)) {
        WARN("can't convert digit to string");
        goto error;
    }
    tok_result.type = TOK_DIGIT;

    dentaku_printf_d(dentaku, "eval '%f %s %f' => '%s'",
        d_n, tok_op.str, d_m, tok_result.str);

    // push result.
    stack_push(stk, &tok_result);
    token_destroy(&tok_result);
    goto just_ret;


error:
    token_destroy(&tok_n);
    token_destroy(&tok_op);
    token_destroy(&tok_m);
    siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
just_ret:
    token_destroy(&tok_n);
    token_destroy(&tok_op);
    token_destroy(&tok_m);
}


// get one token from dentaku->src or dentaku->data_stack->top.
// if error occured, return NULL.
void
dentaku_get_token(Dentaku *dentaku)
{
    bool syntax_error;
    Token tok_result;
    bool allow_signed;
    Token top;
    stack_t *stk = dentaku->data_stack;

    stack_top(stk, &top);
    // allow '+' or '-' before digit
    // when stack is empty or '(' is on the top.
    allow_signed = stack_empty(stk) || top.type == TOK_LPAREN;

    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);

    char *cur_pos = dentaku->src + dentaku->src_pos;
    char *next_pos = get_token(cur_pos, &tok_result, allow_signed, &syntax_error);

    if (next_pos == NULL) {
        // EOF or syntax error.
        token_destroy(&tok_result);

        // set to EOF.
        dentaku->src_pos = dentaku->src_len;
        dentaku_printf_d(dentaku, "reach EOF");

        if (syntax_error)
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        else if (stack_empty(stk))
            // top of stack is NULL and reached EOF.
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else {
        dentaku_printf_d(dentaku, "got! [%s]", tok_result.str);

        dentaku->src_pos += next_pos - cur_pos;
        stack_push(stk, &tok_result);
    }
}







void
dentaku_init(Dentaku *dentaku)
{
    dentaku_printf_d(dentaku, "initializing dentaku...");

    dentaku->data_stack = NULL;

    dentaku->main_jmp_buf = NULL;

    dentaku->f_in  = stdin;
    dentaku->f_out = stdout;
    dentaku->f_err = stderr;

    dentaku->src = NULL;
    dentaku->src_len = -1;
    dentaku->src_pos = -1;

    // rec: recursion
    // stk: stack
    // cmp: compile and run
    dentaku->arg_f = "stk";

    dentaku->arg_rpn = ARG_RPN_NOP;
    dentaku->debug = false;
}


void
dentaku_alloc(Dentaku *dentaku)
{
    dentaku_printf_d(dentaku, "allocating dentaku...");

    dentaku->data_stack = stack_initialize(
        sizeof(Token),
        (void (*)(void *))token_destroy,
        (void *(*)(void*, const void*, size_t))token_copy
    );
    if (! dentaku->data_stack) {
        DIE("failed to initialize stack");
    }

    dentaku->src = malloc(MAX_IN_BUF);
    if (dentaku->src == NULL) {
        DIE("failed to allocate for input string");
    }
}


void
dentaku_destroy(Dentaku *dentaku)
{
    dentaku_printf_d(dentaku, "destroying dentaku...");

    dentaku_clear_stack(dentaku);
    stack_release(dentaku->data_stack);

    if (dentaku->src) {
        free(dentaku->src);
        dentaku->src = NULL;
    }
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



static void
show_usage(void)
{
    puts("");
    printf("Usage: %s [OPTIONS] [--] [file]\n", DENTAKU_PROG_NAME);
    puts("  --help              show this help");
    puts("  --debug             show debug message");
    puts("  --to-rpn            convert infix notation to RPN of input numerical expression");
    puts("  --from-rpn          convert RPN to infix notation of input numerical expression");
    puts("  -f [rec,stk,cmp]    change internal behavior until dentaku shows answer");
    puts("");
}

void
dentaku_getopt(Dentaku *dentaku, int argc, char **argv)
{
    static const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"debug", 0, NULL, 'd'},
        {"to-rpn", 0, NULL, 0},    // TODO
        {"from-rpn", 0, NULL, 0},    // TODO
        {0, 0, 0, 0}
    };
    int opt_index = 0;
    int c;

    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((c = getopt_long(argc, argv, "hdf:", long_opts, &opt_index)) != -1) {
        switch (c) {
            case 'd':
                dentaku->debug = true;
                break;

            case 'f':
                dentaku->arg_f = optarg;

            case 'h':
                show_usage();
                dentaku_exit(dentaku, EXIT_SUCCESS);
            case '?':
                show_usage();
                dentaku_exit(dentaku, EXIT_FAILURE);
        }
    }


    // TODO get filename and set to dentaku->f_in, dentaku->f_out, dentaku->f_err.
}




bool
dentaku_read_src(Dentaku *dentaku)
{
    char buf[MAX_IN_BUF];
    size_t read_size;

    if (fileno(dentaku->f_in) == fileno(stdin) && isatty(0)) {
        fputs(PROMPT_STR, dentaku->f_out);
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
    if (STREQ(dentaku->arg_f, "rec")) {
        // TODO
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else if (STREQ(dentaku->arg_f, "stk")) {
        dentaku_eval_src(dentaku);
    }
    else if (STREQ(dentaku->arg_f, "cmp")) {
        // TODO
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else {
        dentaku_dief(dentaku, "Unknown -f option: %s", dentaku->arg_f);
    }
}





void
dentaku_clear_stack(Dentaku *dentaku)
{
    stack_pop_many_elements(
            dentaku->data_stack,
            stack_size(dentaku->data_stack),
            NULL);
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
        token_destroy(&top);
    }
}

