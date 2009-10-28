
#include "dentaku-stack.h"

#include "dentaku-core.h"
#include "util.h"
#include "parser.h"




static void
eval_stack_expr(Dentaku *dentaku)
{
    Token tok_n, tok_op, tok_m, tok_result, tmp;
    stack_t *stk = dentaku->data_stack;
    stack_ret ret;

    token_init(&tok_n);
    token_init(&tok_op);
    token_init(&tok_m);


    if (stack_empty(stk)) {
        // don't call when top is NULL!!
        DIE("no more items on stack.");
    }

    // 1st. pop 'm' of expression 'n <op> m'.
    switch (ret = stack_pop(stk, &tok_m)) {
    case STACK_SUCCESS:
        dentaku_printf_d(dentaku, "pop 'm' of 'n <op> m'... [%s]", tok_m.str);
        break;
    default:
        DIE2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }

    // 2nd. pop '<op>' of expression 'n <op> m'.
    // '<op>' or '( <op>'
    switch (ret = stack_pop(stk, &tok_op)) {
    /* when '(' or empty stack, just push result (tok_m) and return */
    case STACK_SUCCESS:
        if (tok_op.type != TOK_LPAREN)
            break;
        stack_push(stk, &tok_op);
        /* FALLTHROUGH */
    case STACK_EMPTY:
        stack_push(stk, &tok_m);
        token_destroy(&tok_n);
        token_destroy(&tok_op);
        return;
    default:
        DIE2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }
    dentaku_printf_d(dentaku, "pop '<op>' of 'n <op> m'... [%s]", tok_op.str);

    // 3rd. pop 'n' of expression 'n <op> m'.
    switch (ret = stack_pop(stk, &tok_n)) {
    case STACK_SUCCESS:
        dentaku_printf_d(dentaku, "pop 'n' of 'n <op> m'... [%s]", tok_n.str);
        break;
    case STACK_EMPTY:
        WARN("reaching EOF where digit is expected");
        goto error;
    default:
        DIE2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }

    if (stack_top(stk, &tmp) == STACK_SUCCESS) {
        // fix for the case that '-1 - 1 - 1' results in '-1'.
        // 1 - 1 => 0
        // -1 - 0 => -1
        if (tmp.type == TOK_MINUS) {
            // fix to '-1 + -1 - 1'
            // -1 - 1 => -2
            // -1 + -2 => -3

            // pop '-'
            stack_pop(stk, NULL);

            // push '+'
            Token tok_plus;
            tok_plus.str = "+";
            tok_plus.type = TOK_PLUS;
            stack_push(stk, &tok_plus);

            // n -> -n
            char *str = malloc(strlen(tok_n.str) + 2);
            if (! str) {
                DIE("can't allocate memory!");
            }
            str[0] = '-';
            strcpy(str + 1, tok_n.str);
            if (tok_n.str)
                free(tok_n.str);
            tok_n.str = str;
        }
        token_destroy(&tmp);
    }

    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);
    if (! dentaku_calc_expr(dentaku, &tok_op, &tok_n, &tok_m, &tok_result))
        goto error;

    // Push result.
    stack_push(stk, &tok_result);

    token_destroy(&tok_result);
    token_destroy(&tok_n);
    token_destroy(&tok_op);
    token_destroy(&tok_m);
    return;


error:
    token_destroy(&tok_n);
    token_destroy(&tok_op);
    token_destroy(&tok_m);
    siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
}


// get one token from dentaku->src or dentaku->data_stack->top.
static void
push_got_token(Dentaku *dentaku)
{
    bool syntax_error;
    Token tok_result;
    bool allow_signed;
    Token top;
    stack_t *stk = dentaku->data_stack;

    if (dentaku_src_eof(dentaku))
        return;

    stack_top(stk, &top);
    // allow '+' or '-' before digit
    // when stack is empty or '(' is on the top.
    allow_signed = stack_empty(stk) || top.type == TOK_LPAREN;

    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);

    char *cur_pos = dentaku->src + dentaku->src_pos;
    char *next_pos = parser_get_token(cur_pos, &tok_result, allow_signed, &syntax_error);

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


static void
eval_when_eof_or_rparen(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    bool allow_rparen = false;    // continue evaluating when op has higher priority
    Token top_buf, result;
    const TokenType top_type = (dentaku_src_eof(dentaku) ? TOK_UNDEF : TOK_RPAREN);
    const char *top_type_str = top_type == TOK_RPAREN ? "')'" : "eof";

    if (top_type == TOK_RPAREN) {
        // pop ')'
        stack_pop(stk, NULL);
    }
    dentaku_printf_d(dentaku, "top is %s", top_type_str);

    // calculate until expression becomes 1 token
    // or top is correspond '(' and next does not have higher priority
    for (; ; token_destroy(&result)) {
        eval_stack_expr(dentaku);
        stack_pop(stk, &result);
        dentaku_printf_d(dentaku, "result [%s]", result.str);

        stack_ret ret = stack_top(stk, &top_buf);
        dentaku_printf_d(dentaku,
                "dispatched %s: top is %s.",
                top_type_str,
                ret == STACK_SUCCESS ? top_buf.str : "empty");


        if (stack_empty(stk)) {
            if (top_type == TOK_RPAREN && ! allow_rparen) {
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
            bool is_digit = top_type == TOK_RPAREN
                         && ! stack_empty(stk)
                         && top_buf.type == TOK_DIGIT;
            bool high_priority = top_type == TOK_RPAREN
                              && ! stack_empty(stk)
                              && TOKEN_HAS_HIGHER_PRIORITY(top_buf);

            if (stack_empty(stk) && dentaku_src_eof(dentaku)) {
                dentaku_printf_d(dentaku,
                        "no more tokens. just push the result [%s].",
                        result.str);

                stack_push(stk, &result);
                token_destroy(&result);
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
            }
            else if (is_digit) {
                dentaku_printf_d(dentaku, "add '*' between '(...)' and '(...)'");

                // push "*"
                Token tok_mul;
                tok_mul.str = "*";
                tok_mul.type = TOK_MULTIPLY;
                stack_push(stk, &tok_mul);
                // push result.
                stack_push(stk, &result);

                token_destroy(&result);
                // forbid empty stack with top_type == TOK_RPAREN.
                // because '1+2)' is not valid expression.
                // set this value to through the checking.
                allow_rparen = true;
                continue;
            }
            else if (high_priority) {
                dentaku_printf_d(dentaku,
                        "next op is '*' or '/' or '^'. continue evaluating...");

                // same as above.
                stack_push(stk, &result);

                allow_rparen = true;
                token_destroy(&result);
                continue;
            }
            else {
                stack_push(stk, &result);
                token_destroy(&result);
                break;
            }
        }
        else if (TOKEN_IS_OPERATOR(top_buf)) {
            stack_push(stk, &result);
            if (! TOKEN_HAS_HIGHER_PRIORITY(top_buf))
                break;
        }
        else {
            stack_push(stk, &result);
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
    push_got_token(dentaku);
    // TODO I want something like alloca().
    stack_top(stk, &tok_got);


    if (tok_got.type == TOK_DIGIT) {
        eval_stack_expr(dentaku);
    }
    else if (tok_got.type == TOK_RPAREN) {
        WARN("reaching ')' where expression is expected");
        token_destroy(&tok_got);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else if (TOKEN_IS_OPERATOR(tok_got)) {
        WARN2("reaching '%c' where expression is expected", tok_got.str[0]);
        token_destroy(&tok_got);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else {
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
 * operator:
 *  - if op is '*' or '/'
 *    - get token
 *    - if it is digit, call eval_stack_expr()
 * TOK_DIGIT:
 * TOK_LPAREN:
 *  - nop
 */
NORETURN void
dentaku_stack_run(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    Token tok_got;


    while (1) {
        push_got_token(dentaku);
        stack_top(stk, &tok_got);
        dentaku_printf_d(dentaku, "dispatch! [%s]", tok_got.str);

        if (dentaku_src_eof(dentaku) || tok_got.type == TOK_RPAREN) {    // EOF or ')'
            eval_when_eof_or_rparen(dentaku);
        }
        else if (TOKEN_IS_OPERATOR(tok_got)) {
            // postpone '+' and '-'.
            if (TOKEN_HAS_HIGHER_PRIORITY(tok_got)) {
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
