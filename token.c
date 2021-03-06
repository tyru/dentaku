
#include "token.h"
#include "alloc-list.h"





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
    tok->str = al_malloc(size);
    if (! tok->str) {
        perror("al_malloc");
        exit(EXIT_FAILURE);
    }
}

void
token_realloc(Token *tok, size_t size)
{
    void *new_str = al_realloc(tok->str, size);
    if (! new_str) {
        perror("al_realloc");
        exit(EXIT_FAILURE);
    }
    tok->str = new_str;
}

void
token_destroy(Token *tok)
{
    free(tok->str);
    tok->str = NULL;
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
