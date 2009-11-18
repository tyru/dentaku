/*
 * This is not GC implementation.
 * Just save allocated pointers to allocated_list.
 */

#ifndef DENTAKU_ALLOC_LIST_H
#define DENTAKU_ALLOC_LIST_H

#include "common.h"


void
al_init(void);

void*
al_malloc(size_t size);

void*
al_realloc(void *ptr, size_t size);

void
al_free_pointers(void);

void
al_destroy(void);


#endif /* DENTAKU_ALLOC_LIST_H */
