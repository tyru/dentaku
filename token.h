#ifndef DENTAKU_TOKEN_H
#define DENTAKU_TOKEN_H

#include "common.h"


#define TOKEN_IS_OPERATOR(tok) \
    ((tok).type == TOK_PLUS \
  || (tok).type == TOK_MINUS \
  || (tok).type == TOK_MULTIPLY \
  || (tok).type == TOK_DIVIDE \
  || (tok).type == TOK_UP_ALLOW)

/*
 * Calculator (dentaku) has only
 * higher priority group (*, /, ^)
 * and lower priority group (+, -),
 * So just calculate these operators
 * before lower priority operators.
 * If this is practical programming language,
 * I have to execute each operator by the order
 * of each operator.
 */
#define TOKEN_HAS_HIGHER_PRIORITY(tok) \
    ((tok).type == TOK_MULTIPLY \
  || (tok).type == TOK_DIVIDE \
  || (tok).type == TOK_UP_ALLOW)



typedef enum {
    TOK_UNDEF,
    TOK_DIGIT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_UP_ALLOW,
} TokenType;


struct Token_tag {
    char        *str;
    TokenType   type;
};


void
token_init(Token *tok);

void
token_alloc(Token *tok, size_t size);

void
token_destroy(Token *tok);

Token*
token_copy(Token *dest, const Token *src, size_t n);




#endif /* DENTAKU_TOKEN_H */
