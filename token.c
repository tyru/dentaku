
#include "token.h"





void
token_init(Token *tok)
{
    tok->str  = NULL;
    tok->type = TOK_UNDEF;
}

void
token_alloc(Token *tok, size_t size)
{
    tok->str = malloc(size);
    if (! ALLOCATED(tok->str)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void
token_destroy(Token *tok)
{
    if (tok->str) {
        free(tok->str);
        tok->str = NULL;
    }
}
