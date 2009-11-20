/*
 * NSTL - common headers, types, etc.
 * Written by tyru
 *
 * See LICENSE in NSTL's directory about the license.
 */

#ifndef NSTL_COMMON_H
#define NSTL_COMMON_H

#include <stdlib.h>
#include <stdbool.h>    // This library requires C99 compiler.


typedef enum NstlErrno_tag {
    NSTL_OK,
    NSTL_ARGS,
    NSTL_ALLOC,
    NSTL_EMPTY,
} NstlErrno;


#endif /* NSTL_COMMON_H */
