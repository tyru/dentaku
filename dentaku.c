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
 * - check also stack_(push|pop)'s return value.
 * - check more stack function's return value
 * - use GC
 * - use siglongjmp() when error occured
 * - token_alloc() should call token_init()
 *   not to write 'token_init(), token_alloc()' ...
 * - memcpy() しない関数(stack.cのrefer_top())がstack.hにいてくれると助かる
 *   現在refer_top()をそのままcommon.hに持ってきてる...
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
    while (stack_empty(stk)) {
        size_t cur_size = stack_size(stk);
        dentaku_stack_pop(dentaku, tokens + cur_size);
        dentaku_printf_d(dentaku, "  pop...  %d: [%s]",
                        cur_size, tokens[cur_size].str);
    }

    // restore
    while (stack_size(stk) < stk_len) {
        dentaku_stack_push(dentaku, tokens + stack_size(stk));
    }

    fflush(stderr);
}





// this is function to
// - copy popped token to tok.
// - print debug message
stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok)
{
    // dentaku_printf_d(dentaku, "pop! [%s]", tok->str);
    return stack_pop(dentaku->data_stack, tok);
}


stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok)
{
    // dentaku_printf_d(dentaku, "push! [%s]", tok->str);
    return stack_push(dentaku->data_stack, tok);
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
Token*
dentaku_calc_expr(Dentaku *dentaku, bool *no_op)
{
    Token tok_op, tok_n, tok_m, *tok_result;
    Digit n, m, result;
    double d_n, d_m;
    bool success;
    stack_t *stk = dentaku->data_stack;
    Token top;


    // NOTE: too ugly...



    if (dentaku->debug) {
        dentaku_printf_d(dentaku, "before calculation");
        dentaku_show_stack(dentaku);
    }

    *no_op = false;

    token_init(&tok_op);
    token_init(&tok_n);
    token_init(&tok_m);

    tok_result = malloc(sizeof(Token));
    if (tok_result == NULL) {
        DIE("can't allocate memory for result token!");
    }
    if (stack_empty(stk)) {
        // NOTE: don't call when top is NULL!!
        DIE("no more items on stack.");
    }

    // TODO
    // do 'pop' and 'check' if it failed.
    // now 'check' and 'pop'.


    // 1st. pop 'm' of expression 'n <op> m'.
    token_alloc(&tok_m, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_m);
    dentaku_printf_d(dentaku, "pop 'm' of 'n <op> m'... [%s]", tok_m.str);


    stack_top(stk, &top);
    if (stack_empty(stk) || top.type == TOK_LPAREN) {
        // no more tokens on stack. calculation has been done.
        memcpy(tok_result, &tok_m, sizeof(Token));
        *no_op = true;
        return tok_result;
    }


    // 2nd. pop '<op>' of expression 'n <op> m'.
    token_alloc(&tok_op, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_op);
    dentaku_printf_d(dentaku, "pop '<op>' of 'n <op> m'... [%s]", tok_op.str);


    if (stack_empty(stk)) {
        WARN("reaching EOF where digit is expected");

        // other token's will be destructed at dentaku_destroy().
        token_destroy(&tok_m);
        token_destroy(&tok_op);

        return NULL;
    }


    // 3rd. pop 'n' of expression 'n <op> m'.
    token_alloc(&tok_n, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_n);
    dentaku_printf_d(dentaku, "pop 'n' of 'n <op> m'... [%s]", tok_n.str);


    /* check each token's type */
    if (! (tok_n.type == TOK_DIGIT
        && tok_m.type == TOK_DIGIT
        && tok_op.type == TOK_OP))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n.str, tok_op.str, tok_m.str);
        return NULL;
    }


    /* convert */
    if (! atod(tok_n.str, &n, 10)) {
        WARN2("can't convert '%s' to digit", tok_n.str);
        return NULL;
    }
    d_n = digit2double(&n);

    if (! atod(tok_m.str, &m, 10)) {
        WARN2("can't convert '%s' to digit", tok_m.str);
        return NULL;
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
        return NULL;
    }
    if (! success) {
        WARN("failed to convert double to digit");
        return NULL;
    }


    /* convert result token */
    token_init(tok_result);
    token_alloc(tok_result, MAX_TOK_CHAR_BUF);
    if (! dtoa(&result, tok_result->str, MAX_TOK_CHAR_BUF, 10)) {
        WARN("can't convert digit to string");
        return NULL;
    }
    tok_result->type = TOK_DIGIT;


    dentaku_printf_d(dentaku, "eval '%s %s %s' => '%s'", tok_n.str, tok_op.str, tok_m.str, tok_result->str);

    // free tokens.
    token_destroy(&tok_m);
    token_destroy(&tok_op);
    token_destroy(&tok_n);


    return tok_result;
}


// get one token from dentaku->src or dentaku->data_stack->top.
// if error occured, return NULL.
Token*
dentaku_get_token(Dentaku *dentaku, bool *got_new_token)
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

    *got_new_token = false;

    result_tok = malloc(sizeof(Token));
    if (result_tok == NULL)
        DIE("can't allocate memory for new token!");

    token_init(result_tok);
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

        if (syntax_error)
            return NULL;
        else if (stack_empty(stk))
            // stk->top is NULL and reached EOF.
            return NULL;
        else {
            // FIXME see line 21 of this file
            return refer_top(stk);
        }
    }
    else {
        dentaku_printf_d(dentaku, "got! [%s]", result_tok->str);

        dentaku->src_pos += next_pos - cur_pos;
        *got_new_token = true;
        return result_tok;
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
        (void (*)(void *))token_destroy
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



static bool
eval_when_eof_or_rparen(Dentaku *dentaku, const TokenType top_type, bool *back_to_main)
{
    stack_t *stk = dentaku->data_stack;
    bool no_op, added_mul = false;
    Token top_buf;

    *back_to_main = true;

    if (top_type == TOK_RPAREN) {
        // pop ')'
        // FIXME see line 21 of this file
        token_destroy(refer_top(stk));
        dentaku_stack_pop(dentaku, NULL);
    }

    // calculate until expression becomes 1 token
    // or top is correspond '(' and next is neither '*' nor '/'.
    while (1) {
        Token *result;
        if ((result = dentaku_calc_expr(dentaku, &no_op)) == NULL)
            return false;

        if (dentaku->debug) {
            dentaku_printf_d(dentaku, "after calculation");
            dentaku_show_stack(dentaku);
        }

        if (! no_op) {
            // dentaku has more tokens.
            dentaku_stack_push(dentaku, result);
            continue;
        }


        stack_top(stk, &top_buf);

        if (stack_empty(stk)) {
            if (top_type == TOK_RPAREN && ! added_mul) {
                // token was ')' at first, but got EOF.
                token_destroy(result);
                WARN("extra close parenthesis");
                return false;
            }
            dentaku_stack_push(dentaku, result);

            if (dentaku_src_eof(dentaku))
                // end.
                return true;
            else
                // no more tokens on stack.
                // so I will get tokens from dentaku->src.
                break;
        }
        else if (top_buf.type == TOK_LPAREN) {
            if (top_type == TOK_UNDEF) {
                // token was EOF at first, but got '('.
                token_destroy(result);
                WARN("extra open parenthesis");
                return false;
            }

            // pop '('
            // FIXME see line 21 of this file
            token_destroy(refer_top(stk));
            dentaku_stack_pop(dentaku, NULL);

            // unless top is '*' or '/',
            // finish calculation of one expression in parenthesis.
            // because I got correspond '('.
            stack_top(stk, &top_buf);
            bool mul_or_div = ! stack_empty(stk)
                                && (top_buf.str[0] == '*'
                                ||  top_buf.str[0] == '/');
            bool is_digit = ! stack_empty(stk)
                            && top_buf.type == TOK_DIGIT;

            if (stack_empty(stk) && dentaku_src_eof(dentaku)) {
                // end.
                dentaku_stack_push(dentaku, result);
                return true;
            }
            if (top_type == TOK_RPAREN && is_digit) {
                Token tok_mul;
                token_init(&tok_mul);
                token_alloc(&tok_mul, MAX_TOK_CHAR_BUF);
                strncpy(tok_mul.str, "*", 2);
                tok_mul.type = TOK_OP;

                // push "*"
                dentaku_printf_d(dentaku, "add '*' between '(...)' and '(...)'");
                dentaku_stack_push(dentaku, &tok_mul);
                // push result.
                dentaku_stack_push(dentaku, result);

                // forbid empty stack with top_type == TOK_RPAREN.
                // because '1+2)' is not valid expression.
                // set this value to through the checking.
                added_mul = true;

                continue;
            }
            if (top_type == TOK_RPAREN && mul_or_div) {
                // same as above.
                added_mul = true;

                dentaku_stack_push(dentaku, result);
                continue;
            }
            else {
                dentaku_stack_push(dentaku, result);
                break;
            }
        }
    }

    *back_to_main = false;
    return true;
}


static bool
eval_when_mul_or_div(Dentaku *dentaku, bool *back_to_main)
{
    bool no_op, new_token;
    Token *tok_got;

    *back_to_main = true;

    // get and push right hand operand.
    if ((tok_got = dentaku_get_token(dentaku, &new_token)) == NULL)
        // couldn't get token.
        return false;
    if (new_token) {
        // push new token.
        dentaku_stack_push(dentaku, tok_got);
    }


    switch (tok_got->type) {
    case TOK_DIGIT:
        {
            Token *result;
            if ((result = dentaku_calc_expr(dentaku, &no_op)) == NULL)
                return false;
            dentaku_stack_push(dentaku, result);
            if (no_op)
                return true;
        }
        break;

    // syntax checking
    case TOK_RPAREN:
        WARN("reaching ')' where expression is expected");
        return false;
    case TOK_OP:
        WARN2("reaching '%c' where expression is expected", *tok_got->str);
        return false;
    default:
        break;    // to satisfy gcc.
    }

    *back_to_main = false;
    return true;
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
    Token *tok_got;
    bool new_token, back_to_main;


    while (1) {
        if ((tok_got = dentaku_get_token(dentaku, &new_token)) == NULL)
            // couldn't get token.
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        if (new_token) {
            // push new token.
            dentaku_stack_push(dentaku, tok_got);
        }

        if (dentaku_src_eof(dentaku) || tok_got->type == TOK_RPAREN) {    // EOF or ')'
            bool ret_val = eval_when_eof_or_rparen(
                    dentaku,
                    (tok_got->type == TOK_RPAREN ? TOK_RPAREN : TOK_UNDEF),
                    &back_to_main);
            if (back_to_main)
                siglongjmp(*dentaku->main_jmp_buf, ret_val ? JMP_RET_OK : JMP_RET_ERR);
        }
        else if (tok_got->type == TOK_OP) {    // '+', '-', '*', '/'
            // postpone '+' and '-'.
            if (tok_got->str[0] == '*' || tok_got->str[0] == '/') {
                bool ret_val = eval_when_mul_or_div(dentaku, &back_to_main);
                if (back_to_main)
                    siglongjmp(*dentaku->main_jmp_buf, ret_val ? JMP_RET_OK : JMP_RET_ERR);
            }
        }
        else if (tok_got->type == TOK_DIGIT) {
            // nop.
        }
        else if (tok_got->type == TOK_LPAREN) {
            // nop.
        }
        else {
            if (tok_got->str)
                WARN2("unknown token '%s' found", tok_got->str);
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
            (void (*)(void *))token_destroy);
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

