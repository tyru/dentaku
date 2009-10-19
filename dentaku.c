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
 * - check also stack_(push|pop)'s return value.
 * - delete XXX blocks
 * - check more stack function's return value
 * - what's last "(null)" on stack?
 * - let Dentaku have jmp_buf's member to escape to main()
 *   (if implemented by recursion)
 * - make d_printf() print when dentaku->is_debug
 * - rename MAX_IN_BUF
 * - make dentaku.h
 * - use tok_got to find out if eof
 * - make dentaku_get_token() easy to find out if dentaku want to return to main() like dentaku_calc_expr()
 * - is tok_got necessary?
 */


#include "dentaku.h"


#include "util.h"
#include "parser.h"




// this is function to
// - copy popped token to tok.
// - print debug message
stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok)
{
    Stack *stk = dentaku->cur_stack;

    if (tok)
        memcpy(tok, stk->top, sizeof(Token));

    // d_printf("pop! [%s]", tok->str);
    return stack_pop(stk);
}


stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok)
{
    // d_printf("push! [%s]", tok->str);
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
    Stack *stk = dentaku->cur_stack;

    // TODO
    // - check also stack function's return value.
    // - destruct 3 tokens when return.

    d_printf("before calculation");
    dentaku_show_stack(dentaku);

    *no_op = false;

    token_init(&tok_op);
    token_init(&tok_n);
    token_init(&tok_m);

    tok_result = malloc(sizeof(Token));
    if (tok_result == NULL) {
        DIE("can't allocate memory for result token!");
    }
    if (stk->top == NULL) {
        // NOTE: don't call when top is NULL!!
        DIE("wtf?");
    }


    // 1st. pop 'm' of expression 'n <op> m'.
    token_alloc(&tok_m, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_m);


    if (stk->top == NULL) {
        // no more tokens on stack. calculation has been done.
        memcpy(tok_result, &tok_m, sizeof(Token));
        *no_op = true;
        return tok_result;
    }


    // 2nd. pop '<op>' of expression 'n <op> m'.
    token_alloc(&tok_op, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_op);


    if (stk->top == NULL) {
        WARN("reaching EOF where digit is expected");

        // other token's will be destructed at dentaku_destroy().
        token_destroy(&tok_m);
        token_destroy(&tok_op);

        return NULL;
    }


    // 3rd. pop 'n' of expression 'n <op> m'.
    token_alloc(&tok_n, MAX_TOK_CHAR_BUF);
    dentaku_stack_pop(dentaku, &tok_n);


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


    d_printf("eval '%s %s %s' => '%s'", tok_n.str, tok_op.str, tok_m.str, tok_result->str);

    // free tokens.
    token_destroy(&tok_m);
    token_destroy(&tok_op);
    token_destroy(&tok_n);


    return tok_result;
}


// get one token from dentaku->src or dentaku->cur_stack->top.
// if error occured, return NULL.
Token*
dentaku_get_token(Dentaku *dentaku, bool *got_new_token)
{
    bool syntax_error;
    Token *tok;
    Stack *stk = dentaku->cur_stack;
    bool allow_signed = stk->top == NULL || ((Token*)stk->top)->type == TOK_LPAREN;

    *got_new_token = false;

    tok = malloc(sizeof(Token));
    if (tok == NULL)
        DIE("can't allocate memory for new token!");

    token_init(tok);
    token_alloc(tok, MAX_TOK_CHAR_BUF);

    char *cur_pos = dentaku->src + dentaku->src_pos;
    char *next_pos = get_token(cur_pos, tok, allow_signed, &syntax_error);

    if (next_pos == NULL) {
        // EOF or syntax error.
        token_destroy(tok);
        free(tok);
        tok = NULL;

        // set to EOF.
        dentaku->src_pos = dentaku->src_len;

        if (syntax_error)
            return NULL;
        else if (stk->top == NULL)
            // stk->top is NULL and reached EOF.
            return NULL;
        else
            return stk->top;
    }
    else {
        dentaku->src_pos += next_pos - cur_pos;
        *got_new_token = true;
        return tok;
    }
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

    if (stack_init(dentaku->cur_stack, stack_size, sizeof(Token)) != STACK_SUCCESS) {
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

    if (fileno(dentaku->f_in) == fileno(stdin)) {
        fputs(PROMPT_STR, dentaku->f_out);
        // read each line
        if (fgets(buf, MAX_IN_BUF, dentaku->f_in) == NULL)
            return false;
    }
    else {
        // stream
        size_t read_size = fread(buf, 1, MAX_IN_BUF, dentaku->f_in);

        // TODO
        if (feof(dentaku->f_in))
            ;
        else if (ferror(dentaku->f_in))
            ;
        else if (read_size < MAX_IN_BUF)
            ;
    }

    strncpy(dentaku->src, buf, MAX_IN_BUF);
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
 *    - if it is digit, call dentaku_calc_expr()
 * TOK_DIGIT:
 * TOK_LPAREN:
 *  - nop
 */
bool
dentaku_eval_src(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;
    Token tok_top, *tok_got;
    bool new_token, no_op;


    while (1) {
        if ((tok_got = dentaku_get_token(dentaku, &new_token)) == NULL)
            // couldn't get token.
            return false;

        if (new_token) {
            // push new token.
            dentaku_stack_push(dentaku, tok_got);
        }
        memcpy(&tok_top, tok_got, sizeof tok_top);

        if (dentaku_src_eof(dentaku) || tok_top.type == TOK_RPAREN) {    // EOF or ')'
            TokenType top_type;
            if (tok_top.type == TOK_RPAREN) {
                // pop ')'
                token_destroy(stk->top);
                dentaku_stack_pop(dentaku, NULL);
                top_type = tok_top.type;
            }
            else
                top_type = TOK_UNDEF;

            // calculate until expression becomes 1 token
            // or top is correspond '(' and next is neither '*' nor '/'.
            while (1) {
                Token *result;
                if ((result = dentaku_calc_expr(dentaku, &no_op)) == NULL)
                    return false;
                d_printf("after calculation");
                dentaku_show_stack(dentaku);

                Token *top_buf = stk->top;
                if (top_buf == NULL) {
                    if (top_type == TOK_RPAREN) {
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
                else if (top_buf->type == TOK_LPAREN) {
                    if (top_type == TOK_UNDEF) {
                        // token was EOF at first, but got '('.
                        token_destroy(result);
                        WARN("extra open parenthesis");
                        return false;
                    }

                    // pop '('
                    token_destroy(stk->top);
                    dentaku_stack_pop(dentaku, NULL);

                    // unless top is '*' or '/',
                    // finish calculation of one expression in parenthesis.
                    // because I got correspond '('.
                    top_buf = stk->top;
                    bool mul_or_div = top_buf && (top_buf->str[0] == '*'
                                               || top_buf->str[0] == '/');

                    dentaku_stack_push(dentaku, result);

                    if (top_buf == NULL && dentaku_src_eof(dentaku))
                        // end.
                        return true;
                    if (top_type == TOK_RPAREN && mul_or_div) {
                        // I have already gotten correspond '('.
                        // set this value for above's if block. see it.
                        top_type = TOK_UNDEF;
                        continue;
                    }
                    else
                        break;
                }

                // push for next operator.
                dentaku_stack_push(dentaku, result);
            }
        }
        else if (tok_top.type == TOK_OP) {    // '+', '-', '*', '/'
            // postpone '+' and '-'.
            if (tok_top.str[0] == '*' || tok_top.str[0] == '/') {
                // get and push right hand operand.
                if ((tok_got = dentaku_get_token(dentaku, &new_token)) == NULL)
                    // couldn't get token.
                    return false;

                if (new_token){
                    // push new token.
                    dentaku_stack_push(dentaku, tok_got);
                }
                memcpy(&tok_top, tok_got, sizeof tok_top);


                switch (tok_top.type) {
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
                    WARN2("reaching '%c' where expression is expected", *tok_top.str);
                    return false;
                default:
                    break;    // to satisfy gcc.
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


// for debug and fun.
void
dentaku_show_stack(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;
    int stk_len = stk->cur_pos + 1;
    Token *tokens = alloca(sizeof(Stack) * stk_len);
    int i;

    d_printf("show all stack...");
    if (stk->cur_pos < 0) {
        d_printf("  ...no tokens on stack.");
        return;
    }

    for (; stk->cur_pos >= 0; ) {
        d_printf("  pop...  %d: [%s]", stk->cur_pos, ((Token*)stk->top)->str);
        dentaku_stack_pop(dentaku, tokens + stk->cur_pos);
    }

    for (i = 0; i < stk_len; i++) {
        // d_printf("  push...  %d: [%s]", i, tokens[i]);
        dentaku_stack_push(dentaku, tokens + i);
    }

    fflush(stderr);
}


