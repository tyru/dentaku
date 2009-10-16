#ifndef TOKEN_H
#define TOKEN_H

#include "common.h"




typedef enum {
    TOK_UNDEF,
    TOK_DIGIT,
    TOK_OP,
    TOK_LPAREN,
    TOK_RPAREN,
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




#endif /* TOKEN_H */
