#ifndef DENTAKU_MYPARSER_H
#define DENTAKU_MYPARSER_H

#include "common.h"



Token*
myparser_get_token(char *src, char **next_pos, bool allow_signed, bool *error);


#endif /* DENTAKU_MYPARSER_H */
