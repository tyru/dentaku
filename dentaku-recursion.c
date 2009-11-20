/*
 * "recursion"
 *
 * This uses dentaku->data_stack as stash
 * for get_token(), unget_token(), peek_token().
 */

#include "dentaku-recursion.h"

#define stack_t     stack_t__
    #include "libdatastruct/stack.h"
#undef stack_t

#include "dentaku-core.h"
#include "lexer.h"
#include "token.h"
#include "util.h"
#include "op.h"




static bool
get_token(Dentaku *dentaku, Token *tok_result);

static bool
peek_token(Dentaku *dentaku, Token *tok);

static void
get_primary_expression(Dentaku *dentaku, Token *result);

static void
get_term(Dentaku *dentaku, Token *result);

static void
get_expression(Dentaku *dentaku, Token *result);

static void
get_line(Dentaku *dentaku, Token *result);




// Get head of token.
// If dentaku->data_stack is NOT empty, return the top of its token.
//
// This returns false if EOF.
static bool
get_token(Dentaku *dentaku, Token *tok_result)
{
    bool syntax_error;
    char *cur_pos;
    char *next_pos;
    Token *got_tok;

    if (! stack_empty(dentaku->data_stack)) {
        stack_pop(dentaku->data_stack, tok_result);
        return true;
    }

    cur_pos = dentaku->src + dentaku->src_pos;
    // Allow '+' or '-' before digit
    // when stack is empty or '(' is on the top.
    got_tok = lexer_get_token(cur_pos, &next_pos, false, &syntax_error);

    if (got_tok == NULL) {
        // set to EOF.
        dentaku->src_pos = dentaku->src_len;
        dentaku_printf_d(dentaku, "reach EOF");
        if (syntax_error)
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        return false;
    }
    else {
        dentaku_printf_d(dentaku, "got! [%s]", got_tok->str);

        dentaku->src_pos += next_pos - cur_pos;
        token_copy(tok_result, got_tok, 0);
        return true;
    }
}


static void
unget_token(Dentaku *dentaku, Token *tok)
{
    assert(stack_push(dentaku->data_stack, tok) == STACK_SUCCESS);
}


// This returns false if EOF.
static bool
peek_token(Dentaku *dentaku, Token *tok)
{
    if (stack_empty(dentaku->data_stack)) {
        Token tmp;
        if (get_token(dentaku, &tmp)) {
            assert(stack_push(dentaku->data_stack, &tmp) == STACK_SUCCESS);
        }
        else {
            return false;
        }
    }
    if (tok) {
        assert(stack_top(dentaku->data_stack, tok) == STACK_SUCCESS);
    }
    return true;
}




static void
get_primary_expression(Dentaku *dentaku, Token *result)
{
    dentaku_printf_d(dentaku, "get_primary_expression()");
    if (! get_token(dentaku, result)) {
        WARN("reaching EOF where <primary expression> is expected");
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    assert(result);

    if (result->type == TOK_DIGIT) {
        // <DOUBLE_LITERAL>
        return;
    }
    else if (result->type == TOK_LPAREN) {
        // <YYTOK_LP> <expression> <YYTOK_RP>
        get_expression(dentaku, result);
        dentaku_printf_d(dentaku, "get_expression()...done");

        // Remove ')'
        Token rp;
        if (! (get_token(dentaku, &rp) && rp.type == TOK_RPAREN)) {
            WARN("reaching EOF or non ')' token where ')' is expected");
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        }
    }
    else if (result->type == TOK_PLUS) {
        // <YYTOK_ADD> <primary expression>
        get_primary_expression(dentaku, result);
        dentaku_printf_d(dentaku, "get_primary_expression()...done");
    }
    else if (result->type == TOK_MINUS) {
        // <YYTOK_SUB> <primary expression>
        get_primary_expression(dentaku, result);
        dentaku_printf_d(dentaku, "get_primary_expression()...done");

        Digit d;
        token2digit(&d, result, 10);
        op_unary_minus(&d, &d);
        digit2token(result, &d, MAX_TOK_CHAR_BUF, 10);
    }
    else {
        // syntax error
        WARN2("reacing '%s' where <primary expression> is expected", result->str);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
}


static void
get_term(Dentaku *dentaku, Token *result)
{
    dentaku_printf_d(dentaku, "get_term()");
    get_primary_expression(dentaku, result);
    dentaku_printf_d(dentaku, "get_primary_expression()...done");
    Token tok;

    if (! peek_token(dentaku, &tok)) {
        // <primary expression>
        return;
    }
    else if (TOKEN_HAS_HIGHER_PRIORITY(tok)) {
        // <term> <YYTOK_MUL> <primary expression>
        //      or
        // <term> <YYTOK_DIV> <primary expression>
        Token n, op, m;
        // Get 'n'
        token_copy(&n, result, 0);
        // Get '*' or '/'
        get_token(dentaku, &op);
        // Get 'm'
        get_primary_expression(dentaku, &m);
        dentaku_printf_d(dentaku, "get_primary_expression()...done");
        // result = n <op> m
        if (! dentaku_calc_expr(dentaku, &op, &n, &m, result)) {
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        }

        if (peek_token(dentaku, &tok) && tok.type != TOK_RPAREN) {
            // Not the end of <term>.
            unget_token(dentaku, result);
            get_term(dentaku, result);
            dentaku_printf_d(dentaku, "get_term()...done");
        }
    }

    // Through TOK_RPAREN, and so on.
}


static void
get_expression(Dentaku *dentaku, Token *result)
{
    dentaku_printf_d(dentaku, "get_expression()");
    get_term(dentaku, result);
    dentaku_printf_d(dentaku, "get_term()...done");
    Token tok;

    if (! peek_token(dentaku, &tok)) {
        // <term>
    }
    else if (TOKEN_IS_OPERATOR(tok) && ! TOKEN_HAS_HIGHER_PRIORITY(tok)) {
        // <expression> <YYTOK_ADD> <term>
        //      or
        // <expression> <YYTOK_SUB> <term>
        Token n, op, m;
        // Get 'n'
        token_copy(&n, result, 0);
        // Get '+' or '-'
        get_token(dentaku, &op);
        // Get 'm'
        get_term(dentaku, &m);
        dentaku_printf_d(dentaku, "get_term()...done");
        // result = n <op> m
        if (! dentaku_calc_expr(dentaku, &op, &n, &m, result)) {
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        }

        if (peek_token(dentaku, &tok) && tok.type != TOK_RPAREN) {
            // Not the end of <expression>.
            unget_token(dentaku, result);
            get_expression(dentaku, result);
            dentaku_printf_d(dentaku, "get_expression()...done");
        }
    }

    // Through TOK_RPAREN, and so on.
}


static void
get_line(Dentaku *dentaku, Token *result)
{
    dentaku_printf_d(dentaku, "get_line()");
    get_expression(dentaku, result);
    dentaku_printf_d(dentaku, "get_expression()...done");
    // No more token
    Token tmp;
    if (peek_token(dentaku, &tmp)) {
        WARN2("reaching '%s' where EOF is expected", tmp.str);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
}



NORETURN void
dentaku_recursion_run(Dentaku *dentaku)
{
    Token result;

    get_line(dentaku, &result);
    dentaku_printf_d(dentaku, "get_line()...done");
    stack_push(dentaku->data_stack, &result);
    siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
}
