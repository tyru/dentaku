
#include "token.h"





// initialize.
void
token_init(Token *tok)
{
    tok->str  = NULL;
    tok->type = TOK_UNDEF;
}

// initialize and allocate.
void
token_alloc(Token *tok, size_t size)
{
    token_init(tok);
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

// copy constructor for stack_initialize().
Token*
token_copy(Token *dest, const Token *src, size_t n)
{
    token_alloc(dest, strlen(src->str) + 1);

    strcpy(dest->str, src->str);
    dest->type = src->type;
    return dest;
}
