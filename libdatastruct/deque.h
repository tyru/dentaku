
/*
 * Definition for the double-ended queue
 * Copyright (c) 2009, Kazuhiko Sakaguchi All rights reserved.
 * This file is part of the libdatastruct.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Stricter.org nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY KAZUHIKO SAKAGUCHI ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL KAZUHIKO SAKAGUCHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HEADER_DEQUE_H

#define HEADER_DEQUE_H

/*******************************************************************************
	Constants
*******************************************************************************/

#define DEQUE_SUCCESS                             0x00000000
#define DEQUE_MEMORY_ALLOCATION_ERROR             0x00000001
#define DEQUE_EMPTY                               0x00000002
#define DEQUE_OFFSET_IS_TOO_LARGE                 0x00000004

#define DEQUE_DEFAULT_ARRAY_SIZE                  64

/*******************************************************************************
	Including Headers
*******************************************************************************/

#include "common_public.h"

/*******************************************************************************
	Structures
*******************************************************************************/

typedef struct deque deque_t;

struct deque
{
	char *array;
	size_t size;
	size_t element_size;
	size_t array_size;
	size_t head;
	void (*release_function)(void *);
};

/*******************************************************************************
	Macros
*******************************************************************************/

#define deque_size(deque) \
    ((deque)->size)

#define deque_empty(deque) \
    (!(deque)->size)

/*******************************************************************************
	Functions
*******************************************************************************/

extern deque_t *deque_initialize
    (size_t,void (*)(void *));
extern void deque_release
    (deque_t *);
extern unsigned int deque_front
    (deque_t *,void *);
extern unsigned int deque_back
    (deque_t *,void *);
extern unsigned int deque_refer_from_front
    (deque_t *,size_t,void *);
extern unsigned int deque_refer_from_back
    (deque_t *,size_t,void *);
extern unsigned int deque_push_front
    (deque_t *,const void *);
extern unsigned int deque_push_back
    (deque_t *,const void *);
extern unsigned int deque_pop_front
    (deque_t *,void *);
extern unsigned int deque_pop_back
    (deque_t *,void *);

#endif /* HEADER_DEQUE_H */
