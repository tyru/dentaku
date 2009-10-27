#ifndef DENTAKU_TOKEN_H
#define DENTAKU_TOKEN_H

#include "common.h"


#define TOKEN_IS_OPERATOR(tok_ptr) \
    ((tok_ptr).type == TOK_PLUS \
  || (tok_ptr).type == TOK_MINUS \
  || (tok_ptr).type == TOK_MULTIPLY \
  || (tok_ptr).type == TOK_DIVIDE \
  || (tok_ptr).type == TOK_UP_ALLOW)


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


typedef struct {
    char        *str;
    TokenType   type;
} Token;


void
token_init(Token *tok);

void
token_alloc(Token *tok, size_t size);

void
token_destroy(Token *tok);

Token*
token_copy(Token *dest, const Token *src, size_t n);




#endif /* DENTAKU_TOKEN_H */
