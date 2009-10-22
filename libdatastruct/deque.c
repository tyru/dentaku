
/*
 * Functions for the double-ended queue
 * Copyright (c) 2008-2009, Kazuhiko Sakaguchi All rights reserved.
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

#include <stdlib.h>
#include <string.h>

#include "deque.h"

/*******************************************************************************
	Macros
*******************************************************************************/

#define void_pointer_addition(pointer,number) \
    ((void *)((char *)pointer+number))

#define refer_by_offset(deque,offset) \
    (void_pointer_addition((deque)->array \
    ,(deque)->element_size*((offset)%(deque)->array_size)))

#define refer_by_offset_from_front(deque,offset) \
    refer_by_offset((deque),(deque)->head+(offset))

#define refer_by_offset_from_back(deque,offset) \
    refer_by_offset((deque),(deque)->head+(deque)->size-1-(offset))

#define refer_front(deque) \
    refer_by_offset((deque),(deque)->head)

#define refer_back(deque) \
    refer_by_offset_from_front((deque),(deque)->size-1)

/*******************************************************************************
	Functions
*******************************************************************************/

static unsigned int deque_resize_to_large(deque_t *deque)
{
	void *temp = realloc(deque->array,deque->element_size*deque->array_size*2);
	if(!temp){
		return DEQUE_MEMORY_ALLOCATION_ERROR;
	}
	deque->array = temp;
	if(deque->array_size < deque->head+deque->size){
		memcpy(void_pointer_addition(deque->array
		    ,deque->element_size*deque->array_size),deque->array
		    ,deque->element_size*(deque->head+deque->size-deque->array_size));
	}
	deque->array_size = deque->array_size*2;
	return DEQUE_SUCCESS;
}

deque_t *deque_initialize(const size_t element_size
    ,void (*release_function)(void *))
{
	deque_t *deque = malloc(sizeof(deque_t));
	if(!deque){
		return NULL;
	}
	deque->size = 0;
	deque->element_size = element_size;
	deque->array_size = DEQUE_DEFAULT_ARRAY_SIZE;
	deque->head = 0;
	deque->release_function = release_function;
	deque->array = malloc(element_size*DEQUE_DEFAULT_ARRAY_SIZE);
	if(!deque->array){
		free(deque);
		return NULL;
	}
	return deque;
}

void deque_release(deque_t *deque)
{
	if(!deque){
		return;
	}
	if(deque->release_function){
		size_t counter = 0;
		while(counter != deque->size){
			deque->release_function(refer_by_offset_from_front(deque,counter));
			counter++;
		}
	}
	free(deque->array);
	free(deque);
}

unsigned int deque_front(deque_t *deque,void *output)
{
	if(deque_empty(deque)){
		return DEQUE_EMPTY;
	}
	if(output){
		memcpy(output,refer_front(deque),deque->element_size);
	}
	return DEQUE_SUCCESS;
}

unsigned int deque_back(deque_t *deque,void *output)
{
	if(deque_empty(deque)){
		return DEQUE_EMPTY;
	}
	if(output){
		memcpy(output,refer_back(deque),deque->element_size);
	}
	return DEQUE_SUCCESS;
}

unsigned int deque_refer_from_front
    (deque_t *deque,const size_t offset,void *output)
{
	if(offset >= deque_size(deque)){
		return DEQUE_OFFSET_IS_TOO_LARGE;
	}
	if(output){
		memcpy(output
		    ,refer_by_offset_from_front(deque,offset),deque->element_size);
	}
	return DEQUE_SUCCESS;
}

unsigned int deque_refer_from_back
    (deque_t *deque,const size_t offset,void *output)
{
	if(offset >= deque_size(deque)){
		return DEQUE_OFFSET_IS_TOO_LARGE;
	}
	if(output){
		memcpy(output
		    ,refer_by_offset_from_back(deque,offset),deque->element_size);
	}
	return DEQUE_SUCCESS;
}

unsigned int deque_push_front(deque_t *deque,const void *input)
{
	if(deque->head+deque->size <= deque->array_size
	    && deque->array_size*3 <= deque->size*4){
		deque_resize_to_large(deque);
	}
	if(deque->array_size == deque->size){
		int errcode = deque_resize_to_large(deque);
		if(errcode){
			return errcode;
		}
	}
	deque->head = (deque->head-1)%deque->array_size;
	deque->size++;
	memcpy(refer_front(deque),input,deque->element_size);
	return DEQUE_SUCCESS;
}

unsigned int deque_push_back(deque_t *deque,const void *input)
{
	if(deque->head+deque->size <= deque->array_size
	    && deque->array_size*3 <= deque->size*4){
		deque_resize_to_large(deque);
	}
	if(deque->array_size == deque->size){
		int errcode = deque_resize_to_large(deque);
		if(errcode){
			return errcode;
		}
	}
	deque->size++;
	memcpy(refer_back(deque),input,deque->element_size);
	return DEQUE_SUCCESS;
}

unsigned int deque_pop_front(deque_t *deque,void *output)
{
	if(deque_empty(deque)){
		return DEQUE_EMPTY;
	}
	if(output){
		memcpy(output,refer_front(deque),deque->element_size);
	}
	deque->head = (deque->head+1)%deque->array_size;
	deque->size--;
	return DEQUE_SUCCESS;
}

unsigned int deque_pop_back(deque_t *deque,void *output)
{
	if(deque_empty(deque)){
		return DEQUE_EMPTY;
	}
	if(output){
		memcpy(output,refer_back(deque),deque->element_size);
	}
	deque->size--;
	return DEQUE_SUCCESS;
}

