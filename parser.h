#ifndef PARSER_H
#define PARSER_H

#include "common.h"

#include "token.h"



char*
get_token(char *src, Token *tok, bool allow_signed);


#endif /* PARSER_H */
