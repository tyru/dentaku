/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-10-23.
 *
 */


/* TODO && FIXME
 * - add .str's capacity size to Token
 *   for allocating just token's characters length.
 *   (if capacity is a fewer than needed size, use realloc())
 * - use GC
 * - add more ops. (e.g.: '^', 'log', 'exp')
 */


#include "dentaku.h"


#include "util.h"
#include "parser.h"

#define _GNU_SOURCE
    #include <getopt.h>
#undef _GNU_SOURCE

#include <stdarg.h>
#include <unistd.h>





static void
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


    /* calc */
    switch (*tok_op.str) {
    case '+': success = double2digit(d_n + d_m, &result); break;
    case '-': success = double2digit(d_n - d_m, &result); break;
    case '*': success = double2digit(d_n * d_m, &result); break;
    case '/': success = double2digit(d_n / d_m, &result); break;
    default:
        WARN2("unknown op '%s'", tok_op.str);
        goto error;
    }
    if (! success) {
        WARN("failed to convert double to digit");
        goto error;
    }


    /* convert result token */
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);
    if (! dtoa(&result, tok_result.str, MAX_TOK_CHAR_BUF, 10)) {
        WARN("can't convert digit to string");
        goto error;
    }
    tok_result.type = TOK_DIGIT;

    dentaku_printf_d(dentaku, "eval '%s %s %s' => '%s'",
        tok_n.str, tok_op.str, tok_m.str, tok_result.str);

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
    Token *result_tok;
    bool allow_signed;
    Token top;
    stack_t *stk = dentaku->data_stack;

    stack_top(stk, &top);
    // allow '+' or '-' before digit
    // when stack is empty or '(' is on the top.
    allow_signed = stack_empty(stk) || top.type == TOK_LPAREN;

    result_tok = malloc(sizeof(Token));
    if (result_tok == NULL)
        DIE("can't allocate memory for new token!");

    token_alloc(result_tok, MAX_TOK_CHAR_BUF);

    char *cur_pos = dentaku->src + dentaku->src_pos;
    char *next_pos = get_token(cur_pos, result_tok, allow_signed, &syntax_error);

    if (next_pos == NULL) {
        // EOF or syntax error.
        token_destroy(result_tok);
        free(result_tok);
        result_tok = NULL;

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
        dentaku_printf_d(dentaku, "got! [%s]", result_tok->str);

        dentaku->src_pos += next_pos - cur_pos;
        stack_push(stk, result_tok);
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



static void
eval_when_eof_or_rparen(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    bool top_op_is_mul_div = false;    // continue evaluating when op is '*' or '/'.
    Token top_buf, result;
    const TokenType top_type = (dentaku_src_eof(dentaku) ? TOK_UNDEF : TOK_RPAREN);

    if (top_type == TOK_RPAREN) {
        // pop ')'
        stack_pop(stk, NULL);
    }
    dentaku_printf_d(dentaku, "top is %s", top_type == TOK_RPAREN ? "')'" : "eof");

    // calculate until expression becomes 1 token
    // or top is correspond '(' and next is neither '*' nor '/'.
    for (; ; token_destroy(&result)) {
        dentaku_calc_expr(dentaku);
        stack_pop(stk, &result);
        dentaku_printf_d(dentaku, "result [%s]", result.str);


        if (stack_top(stk, &top_buf) == STACK_EMPTY) {
            if (top_type == TOK_RPAREN && ! top_op_is_mul_div) {
                // token was ')' at first, but got EOF.
                token_destroy(&result);
                WARN("extra close parenthesis");
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
            }
            stack_push(stk, &result);

            if (dentaku_src_eof(dentaku)) {
                // end.
                token_destroy(&result);
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
            }
            else
                // no more tokens on stack.
                // so I will get tokens from dentaku->src.
                break;
        }
        else if (top_buf.type == TOK_LPAREN) {
            if (top_type == TOK_UNDEF) {
                // token was EOF at first, but got '('.
                token_destroy(&result);
                token_destroy(&top_buf);
                WARN("extra open parenthesis");
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
            }

            // pop '('
            stack_pop(stk, NULL);

            // unless top is '*' or '/',
            // finish calculation of one expression in parenthesis.
            // because I got correspond '('.
            token_destroy(&top_buf);
            stack_top(stk, &top_buf);
            bool mul_or_div = ! stack_empty(stk)
                            && (top_buf.str[0] == '*'
                            ||  top_buf.str[0] == '/');
            bool is_digit = ! stack_empty(stk)
                           && top_buf.type == TOK_DIGIT;

            if (stack_empty(stk) && dentaku_src_eof(dentaku)) {
                // end.
                stack_push(stk, &result);
                token_destroy(&result);
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
            }
            else if (top_type == TOK_RPAREN && is_digit) {
                dentaku_printf_d(dentaku, "add '*' between '(...)' and '(...)'");
                // push "*"
                Token tok_mul;
                tok_mul.str = "*";
                tok_mul.type = TOK_OP;
                stack_push(stk, &tok_mul);
                // push result.
                stack_push(stk, &result);

                token_destroy(&result);
                // forbid empty stack with top_type == TOK_RPAREN.
                // because '1+2)' is not valid expression.
                // set this value to through the checking.
                top_op_is_mul_div = true;
                continue;
            }
            else if (top_type == TOK_RPAREN && mul_or_div) {
                // same as above.
                stack_push(stk, &result);

                top_op_is_mul_div = true;
                token_destroy(&result);
                continue;
            }
            else {
                stack_push(stk, &result);
                token_destroy(&result);
                break;
            }
        }
    }
    token_destroy(&result);
    token_destroy(&top_buf);
}


static void
eval_when_mul_or_div(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    Token tok_got;

    // get and push right hand operand.
    dentaku_get_token(dentaku);
    // TODO I want something like alloca().
    stack_top(stk, &tok_got);


    switch (tok_got.type) {
    case TOK_DIGIT:
        dentaku_calc_expr(dentaku);
        break;

    // just syntax checking
    case TOK_RPAREN:
        WARN("reaching ')' where expression is expected");
        token_destroy(&tok_got);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    case TOK_OP:
        WARN2("reaching '%c' where expression is expected", *tok_got.str);
        token_destroy(&tok_got);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    default:
        token_destroy(&tok_got);
    }
}



/*
 * TOK_RPAREN or EOF:
 *  - pop if TOK_RPAREN
 *  - repeat
 *    - pop top of TOK_DIGIT (m)
 *    - if top is NULL, push m and return.
 *    - pop top of TOK_LPAREN (op)
 *    - pop top of TOK_DIGIT (n)
 *    - do calculation (result)
 *    - if top is '(', pop it
 *    - push result
 * TOK_OP:
 *  - if op is '*' or '/'
 *    - get token
 *    - if it is digit, call dentaku_calc_expr()
 * TOK_DIGIT:
 * TOK_LPAREN:
 *  - nop
 */
NORETURN void
dentaku_eval_src(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    Token tok_got;


    while (1) {
        dentaku_get_token(dentaku);
        stack_top(stk, &tok_got);
        dentaku_printf_d(dentaku, "dispatch! [%s]", tok_got.str);

        if (dentaku_src_eof(dentaku) || tok_got.type == TOK_RPAREN) {    // EOF or ')'
            eval_when_eof_or_rparen(dentaku);
        }
        else if (tok_got.type == TOK_OP) {    // '+', '-', '*', '/'
            // postpone '+' and '-'.
            if (tok_got.str[0] == '*' || tok_got.str[0] == '/') {
                eval_when_mul_or_div(dentaku);
            }
        }
        else if (tok_got.type == TOK_DIGIT) {
            // nop.
        }
        else if (tok_got.type == TOK_LPAREN) {
            // nop.
        }
        else {
            if (tok_got.str)
                WARN2("unknown token '%s' found", tok_got.str);
            else
                WARN("unknown token found");
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        }
    }
}





void
dentaku_clear_stack(Dentaku *dentaku)
{
    // TODO free and pop each top

    stack_release(dentaku->data_stack);

    dentaku->data_stack = stack_initialize(
        sizeof(Token),
        (void (*)(void*))token_destroy,
        (void *(*)(void*, const void*, size_t))token_copy
    );
    if (! dentaku->data_stack) {
        DIE("failed to initialize stack");
    }
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
    if (stack_top(dentaku->data_stack, &top) == STACK_SUCCESS)
        puts(top.str);
}

