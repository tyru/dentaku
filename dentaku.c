/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-10-19.
 *
 */


/* TODO
 * - add .str's capacity size to Token
 *   for allocating just token's characters length.
 *   (if capacity is a fewer than needed size, use realloc())
 * - replace 'destroy' with 'destruct'
 * - can use TOKEN_ALLOCA() only if 'USE_TOKEN_ALLOCA' is defined
 * - check also stack_(push|pop)'s return value.
 */


#define _POSIX_C_SOURCE 1
    #include "common.h"
#undef _POSIX_C_SOURCE

#include "util.h"
#include "token.h"
#include "parser.h"

#include "mylib/stack.h"


#include <stdarg.h>






typedef struct {
    Stack   *cur_stack;    // easier to access
    Stack   cur_stack__;

    FILE    *f_in;
    FILE    *f_out;
    FILE    *f_err;

    char    *src;
    int     src_pos;
    int     src_len;
} Dentaku;




stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok);

stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok);

bool
dentaku_src_eof(Dentaku *dentaku);

bool
dentaku_calc_op(Dentaku *dentaku, Token *tok_result, bool *done);

Token*
dentaku_get_token(Dentaku *dentaku, bool *error);



void
dentaku_init(Dentaku *dentaku);

void
dentaku_alloc(Dentaku *dentaku, size_t stack_size);

void
dentaku_destroy(Dentaku *dentaku);



bool
dentaku_read_src(Dentaku *dentaku);

bool
dentaku_eval_src(Dentaku *dentaku);

void
dentaku_clear_stack(Dentaku *dentaku);

void
dentaku_show_result(Dentaku *dentaku);





// this is function to
// - copy popped token to tok.
// - print debug message
stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok)
{
    Stack *stk = dentaku->cur_stack;

    if (tok)
        memcpy(tok, stk->top, sizeof(Token));

    d_printf("pop! [%s]", ((Token*)stk->top)->str);
    return stack_pop(stk);
}


stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok)
{
    d_printf("push! [%s]", tok->str);
    return stack_push(dentaku->cur_stack, tok);
}



bool
dentaku_src_eof(Dentaku *dentaku)
{
    return dentaku->src_pos >= dentaku->src_len;
}





/*
 * calculate top 3 elems of stack.
 * result token is tok_result.
 */
bool
dentaku_calc_op(Dentaku *dentaku, Token *tok_result, bool *done)
{
    Token tok_op, tok_n, tok_m;
    Digit n, m, result;
    double d_n, d_m;
    bool success;
    Stack *stk = dentaku->cur_stack;

    // TODO check also stack function's return value.


    token_init(&tok_op);
    token_init(&tok_n);
    token_init(&tok_m);

    *done = false;


    // pop 'm' of expression 'n <op> m'.
    token_alloc(&tok_m, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_m);


    if (stk->top == NULL) {
        // no more tokens on stack. calculation has been done.
        strncpy(tok_result->str, tok_m.str, MAX_TOK_CHAR_BUF);
        tok_result->type = tok_m.type;
        token_destroy(&tok_m);

        *done = true;
        return true;
    }
    // allow '(<digit>)'.
    if (((Token*)stk->top)->type == TOK_LPAREN) {
        // pop '('
        token_destroy(stk->top);
        dentaku_stack_pop(dentaku, NULL);

        // copy <digit> to result.
        strncpy(tok_result->str, tok_m.str, MAX_TOK_CHAR_BUF);
        tok_result->type = tok_m.type;
        token_destroy(&tok_m);

        return true;
    }


    // pop '<op>' of expression 'n <op> m'.
    token_alloc(&tok_op, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_op);


    if (stk->top == NULL) {
        WARN("reaching EOF where digit is expected");

        // other token's will be destructed at dentaku_destroy().
        token_destroy(&tok_m);
        token_destroy(&tok_op);

        return false;
    }


    // pop 'n' of expression 'n <op> m'.
    token_alloc(&tok_n, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_n);

    // back to main() after this calculation.
    if (stk->top == NULL) {
        *done = true;
    }


    /* check each token's type */
    if (! (tok_n.type == TOK_DIGIT
        && tok_m.type == TOK_DIGIT
        && tok_op.type == TOK_OP))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n.str, tok_op.str, tok_m.str);
        return false;
    }


    /* convert */
    if (! atod(tok_n.str, &n, 10)) {
        WARN2("can't convert '%s' to digit", tok_n.str);
        return false;
    }
    d_n = digit2double(&n);

    if (! atod(tok_m.str, &m, 10)) {
        WARN2("can't convert '%s' to digit", tok_m.str);
        return false;
    }
    d_m = (double)m.i + m.d;


    /* calc */
    d_printf("eval '%s %s %s'", tok_n.str, tok_op.str, tok_m.str);
    switch (*tok_op.str) {
    case '+': success = double2digit(d_n + d_m, &result); break;
    case '-': success = double2digit(d_n - d_m, &result); break;
    case '*': success = double2digit(d_n * d_m, &result); break;
    case '/': success = double2digit(d_n / d_m, &result); break;
    default:
        WARN2("unknown op '%s'", tok_op.str);
        return false;
    }
    if (! success) {
        WARN("failed to convert double to digit");
        return false;
    }


    // free tokens.
    token_destroy(&tok_m);
    token_destroy(&tok_op);
    token_destroy(&tok_n);


    /* push result */
    // XXX tok_result->str's length may not be MAX_TOK_CHAR_BUF.
    if (! dtoa(&result, tok_result->str, MAX_TOK_CHAR_BUF, 10)) {
        WARN("can't convert digit to string");
        return false;
    }
    tok_result->type = TOK_DIGIT;


    return true;
}


// get and return one token.
// if EOF, return NULL.
Token*
dentaku_get_token(Dentaku *dentaku, bool *error)
{
    Token *tok;
    Stack *stk = dentaku->cur_stack;
    bool allow_signed = stk->top == NULL || ((Token*)stk->top)->type == TOK_LPAREN;

    tok = malloc(sizeof(Token));
    if (tok == NULL) {
        DIE("can't allocate memory for new token.");
    }

    token_init(tok);
    token_alloc(tok, MAX_TOK_CHAR_BUF);

    char *cur_pos = dentaku->src + dentaku->src_pos;
    char *next_pos = get_token(cur_pos, tok, allow_signed, error);

    // advance dentaku->src_pos.
    if (next_pos == NULL) {
        token_destroy(tok);
        free(tok);
        tok = NULL;

        dentaku->src_pos = dentaku->src_len;    // EOF
    }
    else {
        dentaku->src_pos += next_pos - cur_pos;
        // d_printf("rest [%s]", next_pos);
    }

    return tok;
}







void
dentaku_init(Dentaku *dentaku)
{
    d_printf("initializing dentaku...");

    dentaku->cur_stack = &dentaku->cur_stack__;
    dentaku->f_in  = stdin;
    dentaku->f_out = stdout;
    dentaku->f_err = stderr;

    dentaku->src = NULL;
    dentaku->src_len = -1;
    dentaku->src_pos = -1;
}


void
dentaku_alloc(Dentaku *dentaku, size_t stack_size)
{
    d_printf("allocating dentaku...");

    // allocate Token's stack
    stack_ret ret = stack_init(dentaku->cur_stack, stack_size, sizeof(Token));
    if (ret != STACK_SUCCESS) {
        WARN("failed to initialize stack");
        exit(EXIT_FAILURE);
    }
}


void
dentaku_destroy(Dentaku *dentaku)
{
    d_printf("destroying dentaku...");

    dentaku_clear_stack(dentaku);
    if (stack_destruct(dentaku->cur_stack) != STACK_SUCCESS)
        WARN("failed to destruct stack");

    if (dentaku->src) {
        free(dentaku->src);
        dentaku->src = NULL;
    }
}




bool
dentaku_read_src(Dentaku *dentaku)
{
    char buf[MAX_IN_BUF];
    d_printf("dentaku_read_src()");

    buf[0] = '\0';

    if (fileno(dentaku->f_in) == fileno(stdin)) {
        fputs(PROMPT_STR, dentaku->f_out);
        // read each line
        if (fgets(buf, MAX_IN_BUF, dentaku->f_in) == NULL)
            return false;
    }
    else {
    }

    dentaku->src = strndup(buf, MAX_IN_BUF);
    if (dentaku->src == NULL) {
        WARN("can't allocate for input string!");
        return false;
    }
    d_printf("read! [%s]", dentaku->src);

    dentaku->src_len = strlen(dentaku->src);
    dentaku->src_pos = 0;

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
 *    - if it is digit, call dentaku_calc_op()
 * TOK_DIGIT:
 * TOK_LPAREN:
 *  - nop
 */
bool
dentaku_eval_src(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;
    Token tok_top;
    bool syntax_error;
    TokenType top_type;


    while (1) {
        Token *buf = dentaku_get_token(dentaku, &syntax_error);
        if (buf)
            memcpy(&tok_top, buf, sizeof tok_top);

        if (syntax_error) {
            return false;
        }
        else if (dentaku_src_eof(dentaku)) {
            if (stk->top == NULL)
                // there are no tokens on stack, and parser gets EOF.
                return false;
            else
                memcpy(&tok_top, stk->top, sizeof tok_top);
        }
        else {
            dentaku_stack_push(dentaku, &tok_top);
        }
        top_type = tok_top.type;


        // TODO
        // - separate each case into static functions


        if (dentaku_src_eof(dentaku) || tok_top.type == TOK_RPAREN) {    // EOF or ')'
            if (tok_top.type == TOK_RPAREN) {
                token_destroy(stk->top);
                dentaku_stack_pop(dentaku, NULL);
            }

            // XXX
            // if input is '1 + 2 * (3 + 4) + 5'.
            //
            // when '1 + 2 * (3 + 4)':
            //      expression should be '1 + 2 * 7'
            //      but then it gotta get one more token.
            //      so expression becomes '1 + 2 * 7 + 5'.
            //      and after that,
            //
            //      '1 + 2 * 12'
            //      '1 + 24'
            //      '25'
            //
            //      but correct answer is '20'.

            while (1) {
                bool done;
                token_init(&tok_top);
                token_alloc(&tok_top, MAX_TOK_CHAR_BUF);

                // tok_top is result.
                if (! dentaku_calc_op(dentaku, &tok_top, &done)) {
                    // error.
                    token_destroy(&tok_top);
                    return false;
                }
                else if (done && dentaku_src_eof(dentaku)) {
                    // calculation has been done.
                    dentaku_stack_push(dentaku, &tok_top);
                    return true;
                }

                Token *top_tmp = stk->top;
                if (top_tmp && top_tmp->type == TOK_LPAREN) {
                    // pop '('.
                    token_destroy(top_tmp);
                    dentaku_stack_pop(dentaku, NULL);

                    // finish calculation of one expression in parenthesis.
                    // unless top is '*' or '/'.
                    top_tmp = stk->top;
                    bool mul_or_div = top_tmp && (top_tmp->str[0] == '*' || top_tmp->str[0] == '/');
                    if (! (top_type == TOK_RPAREN && mul_or_div))
                        break;
                }

                dentaku_stack_push(dentaku, &tok_top);
            }
        }
        else if (tok_top.type == TOK_OP) {    // '+', '-', '*', '/'
            // postpone '+' and '-'.
            if (tok_top.str[0] == '*' || tok_top.str[0] == '/') {
                // get and push digit token.
                Token *buf = dentaku_get_token(dentaku, &syntax_error);
                if (buf)
                    memcpy(&tok_top, buf, sizeof tok_top);

                if (syntax_error) {
                    return false;
                }
                if (dentaku_src_eof(dentaku)) {
                    WARN("reaching EOF where expression is expected");
                    return false;
                }
                dentaku_stack_push(dentaku, &tok_top);


                switch (tok_top.type) {
                case TOK_DIGIT:
                    {
                        bool done;
                        token_init(&tok_top);
                        token_alloc(&tok_top, MAX_TOK_CHAR_BUF);

                        // tok_top is result.
                        if (! dentaku_calc_op(dentaku, &tok_top, &done)) {
                            // error.
                            token_destroy(&tok_top);
                            return false;
                        }
                        else if (done && dentaku_src_eof(dentaku)) {
                            // calculation has been done.
                            dentaku_stack_push(dentaku, &tok_top);
                            return true;
                        }

                        // Token *top_tmp = stk->top;
                        // if (top_tmp && top_tmp->type == TOK_LPAREN) {
                        //     // pop '('.
                        //     token_destroy(top_tmp);
                        //     dentaku_stack_pop(dentaku, NULL);
                        // 
                        //     // finish calculation of one expression in parenthesis.
                        //     // unless top is '*' or '/'.
                        //     top_tmp = stk->top;
                        //     bool mul_or_div = top_tmp && (top_tmp->str[0] == '*' || top_tmp->str[0] == '/');
                        //     if (! (top_type == TOK_RPAREN && mul_or_div))
                        //         break;
                        // }

                        dentaku_stack_push(dentaku, &tok_top);
                    }

                    break;

                // syntax checking
                case TOK_RPAREN:
                    WARN("reaching ')' where expression is expected");
                    return false;
                case TOK_OP:
                    WARN2("reaching '%c' where expression is expected", *tok_top.str);
                    return false;
                }
            }
        }
        else if (tok_top.type == TOK_DIGIT) {
            // nop.
        }
        else if (tok_top.type == TOK_LPAREN) {
            // nop.
            //
            // TODO
            // because I want calc to assume '(expr)(expr)' as '(expr)*(expr)'.
        }
        else {
            if (tok_top.str)
                WARN2("unknown token '%s' found", tok_top.str);
            else
                WARN("unknown token found");
            return false;
        }
    }
}


void
dentaku_clear_stack(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;

    while (stk->top) {
        token_destroy(stk->top);
        dentaku_stack_pop(dentaku, NULL);
    }
}


void
dentaku_show_result(Dentaku *dentaku)
{
    Token *top = dentaku->cur_stack->top;
    if (top)
        puts(top->str);
}






int main(int argc, char *argv[])
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
