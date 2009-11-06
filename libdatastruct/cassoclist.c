
/*
 * Functions for the associative array data structure
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

/*******************************************************************************
	Including Headers
*******************************************************************************/

#include "cassoclist.h"
#include "common_private.h"

/*******************************************************************************
	Macros
*******************************************************************************/

#define hash(cassoclist,key,number) \
    (hash_function(key,number)&((cassoclist)->array_size-1))

#define get_element_info_by_offset(cassoclist,offset) \
    (&(cassoclist)->element_info_array[offset])

#define get_used_flag(element_info) \
    ((element_info)->used_flag)

#define get_used_flag_by_offset(cassoclist,offset) \
    ((cassoclist)->element_info_array[offset].used_flag)

#define get_mode_flag(element_info) \
    ((element_info)->mode_flag)

#define get_mode_flag_by_offset(cassoclist,offset) \
    ((cassoclist)->element_info_array[offset].mode_flag)

#define get_hash_id(element_info) \
    ((element_info)->hash_id)

#define get_hash_id_by_offset(cassoclist,offset) \
    ((cassoclist)->element_info_array[offset].hash_id)

#define get_long_key(element_info) \
    ((element_info)->key.long_key)

#define get_long_key_by_offset(cassoclist,offset) \
    ((cassoclist)->element_info_array[offset].key.long_key)

#define get_short_key(element_info) \
    ((element_info)->key.short_key)

#define get_short_key_by_offset(cassoclist,offset) \
    ((cassoclist)->element_info_array[offset].key.short_key)

#define get_key(element_info) \
    (get_mode_flag(element_info)?get_long_key(element_info) \
                                :get_short_key(element_info))

#define get_key_by_offset(cassoclist,offset) \
    (get_mode_flag_by_offset(cassoclist,offset) \
        ?get_long_key_by_offset(cassoclist,offset) \
        :get_short_key_by_offset(cassoclist,offset))

#define free_long_key(element_info) \
    (get_mode_flag(element_info) && (free(get_long_key(element_info)),1))

#define free_long_key_by_offset(cassoclist,offset) \
    (get_mode_flag_by_offset(cassoclist,offset) \
    && (free(get_long_key_by_offset(cassoclist,offset)),1))

#define get_value_by_offset(cassoclist,offset) \
    ((cassoclist)->value_array+(cassoclist)->element_size*(offset))

#define compare_key_and_element_info(key,element_info) \
    (get_used_flag(element_info) && compare_string(key,get_key(element_info)))

#define compare_key_and_offset(cassoclist,key,offset) \
    (get_used_flag_by_offset(cassoclist,offset) \
    && compare_string(key,get_key_by_offset(cassoclist,offset)))

/*******************************************************************************
	Functions
*******************************************************************************/

static int compare_string(const char *str1,const char *str2)
{
	while(*str1 == *str2){
		if(!*str1){
			return 1;
		}
		str1++;
		str2++;
	}
	return 0;
}

static size_t hash_function(const char *string,size_t num)
{
	size_t result = num;
	while(*string){
		result = (result*0x00000129)^*string;
		string++;
	}
	return result;
}

static unsigned int cassoclist_private_lookup(cassoclist_t *cassoclist
    ,const char *key,size_t *offset)
{
	size_t hash_value,hash_id = 0;
	while(hash_id != CASSOCLIST_HASH_TYPES){
		hash_value = hash(cassoclist,key,hash_id);
		if(hash_id == get_hash_id_by_offset(cassoclist,hash_value)
		    && compare_key_and_offset(cassoclist,key,hash_value)){
			if(offset){
				*offset = hash_value;
			}
			return CASSOCLIST_SUCCESS;
		}
		hash_id++;
	}
	return CASSOCLIST_NOT_EXISTENT_KEY;
}

static unsigned int cassoclist_resize(cassoclist_t *cassoclist)
{
	size_t element_counter = 0,array_size = cassoclist->array_size;
	void *temp = realloc(cassoclist->element_info_array
	    ,sizeof(cassoclist_element_info_t)*array_size*2);
	cassoclist_element_info_t *element_info;
	if(!temp){
		return CASSOCLIST_MEMORY_ALLOCATION_ERROR;
	}
	cassoclist->element_info_array = temp;
	temp = realloc(cassoclist->value_array
	    ,cassoclist->element_size*array_size*2);
	if(!temp){
		return CASSOCLIST_MEMORY_ALLOCATION_ERROR;
	}
	cassoclist->value_array = temp;
	while(element_counter != array_size){
		element_info = get_element_info_by_offset(cassoclist,element_counter);
		if(get_used_flag(element_info) && (hash_function(get_key(element_info)
		    ,get_hash_id(element_info))&array_size)){
			*get_element_info_by_offset(cassoclist,element_counter+array_size)
			    = *get_element_info_by_offset(cassoclist,element_counter);
			memcpy(get_value_by_offset(cassoclist,element_counter+array_size)
			    ,get_value_by_offset(cassoclist,element_counter)
			    ,cassoclist->element_size);
			get_used_flag(element_info) = 0;
		}
		else{
			get_used_flag(element_info+array_size) = 0;
		}
		element_counter++;
	}
	cassoclist->array_size = array_size*2;
	return CASSOCLIST_SUCCESS;
}

static unsigned int cassoclist_trymove(cassoclist_t *cassoclist
    ,size_t offset,unsigned int recur_limit)
{
	typedef struct
	{
		size_t offset;
		cassoclist_element_info_t *element_info;
		unsigned int hash_id;
	} trymove_stack_t;
	trymove_stack_t *stack_top,*stack_bottom;
	if(!get_used_flag_by_offset(cassoclist,offset)){
		return CASSOCLIST_SUCCESS;
	}
	stack_top = stack_bottom
	    = portable_alloca(sizeof(trymove_stack_t)*(recur_limit+1));
	if(!portable_alloca_check(stack_bottom)){
		return CASSOCLIST_MEMORY_ALLOCATION_ERROR;
	}
	stack_top->offset = offset;
	stack_top->element_info = get_element_info_by_offset(cassoclist,offset);
	stack_top->hash_id = 0;
	while(1){
		while(stack_top-stack_bottom != recur_limit-1){
			stack_top++;
			stack_top->offset = hash(cassoclist
			    ,get_key((stack_top-1)->element_info),(stack_top-1)->hash_id);
			stack_top->element_info
			    = get_element_info_by_offset(cassoclist,stack_top->offset);
			stack_top->hash_id = 0;
			if(!get_used_flag(stack_top->element_info)){
				while(stack_top != stack_bottom){
					*(stack_top->element_info) = *((stack_top-1)->element_info);
					stack_top->element_info->hash_id = (stack_top-1)->hash_id;
					memcpy(get_value_by_offset(cassoclist,stack_top->offset)
					    ,get_value_by_offset(cassoclist,(stack_top-1)->offset)
					    ,cassoclist->element_size);
					stack_top--;
				}
				get_used_flag(stack_top->element_info) = 0;
				portable_alloca_free(stack_bottom);
				return CASSOCLIST_SUCCESS;
			}
		}
		stack_top--;
		stack_top->hash_id++;
		while(stack_top->hash_id == CASSOCLIST_HASH_TYPES){
			if(stack_top == stack_bottom){
				portable_alloca_free(stack_bottom);
				return CASSOCLIST_HASH_COLLISION;
			}
			stack_top--;
			stack_top->hash_id++;
		}
	}
}

cassoclist_t *cassoclist_initialize(size_t element_size
    ,void (*release_function)(void *))
{
	cassoclist_t *cassoclist = malloc(sizeof(cassoclist_t));
	if(!cassoclist){
		return NULL;
	}
	cassoclist->size = 0;
	cassoclist->element_size = element_size;
	cassoclist->array_size = CASSOCLIST_DEFAULT_ARRAY_SIZE;
	cassoclist->release_function = release_function;
	cassoclist->element_info_array = calloc
	    (CASSOCLIST_DEFAULT_ARRAY_SIZE,sizeof(cassoclist_element_info_t));
	cassoclist->value_array
	    = calloc(CASSOCLIST_DEFAULT_ARRAY_SIZE,element_size);
	if(!(cassoclist->element_info_array && cassoclist->value_array)){
		free(cassoclist->element_info_array);
		free(cassoclist->value_array);
		free(cassoclist);
		return NULL;
	}
	return cassoclist;
}

void cassoclist_release(cassoclist_t *cassoclist)
{
	size_t counter = 0;
	if(!cassoclist){
		return;
	}
	while(counter != cassoclist->array_size){
		cassoclist_element_info_t *element_info
		    = get_element_info_by_offset(cassoclist,counter);
		if(get_used_flag(element_info)){
			free_long_key(element_info);
			if(cassoclist->release_function != DEFAULT_RELEASE_FUNCTION){
				cassoclist->release_function
				    (get_value_by_offset(cassoclist,counter));
			}
		}
		counter++;
	}
	free(cassoclist->element_info_array);
	free(cassoclist->value_array);
	free(cassoclist);
}

unsigned int cassoclist_add(cassoclist_t *cassoclist
    ,const char *key,const void *input)
{
	size_t hash_value,hash_id,key_length;
	unsigned int errcode,counter = 1;
	cassoclist_element_info_t *element_info;
	if(!key){
		return CASSOCLIST_INVALID_KEY;
	}
	if(!cassoclist_private_lookup(cassoclist,key,NULL)){
		return CASSOCLIST_KEY_COLLISION;
	}
	if(cassoclist->size*4 >= cassoclist->array_size*3){
		/* 
			もしハッシュテーブルの3/4がデータで埋まっていれば、リサイズを試みる。
			失敗した場合でもエラー処理の必要は無い。
		*/
		cassoclist_resize(cassoclist);
	}
	while(1){
		hash_id = 0;
		while(hash_id != CASSOCLIST_HASH_TYPES){
			hash_value = hash(cassoclist,key,hash_id);
			if(!cassoclist_trymove(cassoclist,hash_value,counter*2)){
				goto SUCCEED_IN_TRYMOVE;
			}
			hash_id++;
		}
		if(counter == 1 && (errcode = cassoclist_resize(cassoclist))){
			return errcode;
		}
		else if(counter == 2){
			return CASSOCLIST_HASH_COLLISION;
		}
		counter++;
	}
	SUCCEED_IN_TRYMOVE:;
	key_length = strlen(key);
	element_info = get_element_info_by_offset(cassoclist,hash_value);
	get_mode_flag(element_info)
	    = (key_length >= CASSOCLIST_MAX_OF_SHORT_KEY_SIZE);
	if(get_mode_flag(element_info)){
		get_long_key(element_info) = malloc(key_length+1);
		if(!get_long_key(element_info)){
			return CASSOCLIST_MEMORY_ALLOCATION_ERROR;
		}
		memcpy(get_long_key(element_info),key,key_length+1);
	}
	else{
		memcpy(get_short_key(element_info),key,key_length+1);
	}
	get_used_flag(element_info) = 1;
	get_hash_id(element_info) = hash_id;
	memcpy(get_value_by_offset(cassoclist,hash_value)
	    ,input,cassoclist->element_size);
	cassoclist->size++;
	return CASSOCLIST_SUCCESS;
}

unsigned int cassoclist_reassign(cassoclist_t *cassoclist
    ,const char *key,const void *input)
{
	size_t offset;
	unsigned int errcode;
	if(!key){
		return CASSOCLIST_INVALID_KEY;
	}
	errcode = cassoclist_private_lookup(cassoclist,key,&offset);
	if(errcode){
		return errcode;
	}
	if(cassoclist->release_function != DEFAULT_RELEASE_FUNCTION){
		cassoclist->release_function(get_value_by_offset(cassoclist,offset));
	}
	memcpy(get_value_by_offset(cassoclist,offset)
	    ,input,cassoclist->element_size);
	return CASSOCLIST_SUCCESS;
}

unsigned int cassoclist_lookup(cassoclist_t *cassoclist
    ,const char *key,void *output)
{
	size_t offset;
	unsigned int errcode;
	if(!key){
		return CASSOCLIST_INVALID_KEY;
	}
	errcode = cassoclist_private_lookup(cassoclist,key,&offset);
	if(errcode){
		return errcode;
	}
	if(output){
		memcpy(output
		    ,get_value_by_offset(cassoclist,offset),cassoclist->element_size);
	}
	return CASSOCLIST_SUCCESS;
}

unsigned int cassoclist_remove(cassoclist_t *cassoclist
    ,const char *key,void *output)
{
	size_t offset;
	unsigned int errcode;
	cassoclist_element_info_t *element_info;
	if(!key){
		return CASSOCLIST_INVALID_KEY;
	}
	errcode = cassoclist_private_lookup(cassoclist,key,&offset);
	if(errcode){
		return errcode;
	}
	element_info = get_element_info_by_offset(cassoclist,offset);
	free_long_key(element_info);
	if(output){
		memcpy(output
		    ,get_value_by_offset(cassoclist,offset),cassoclist->element_size);
	}
	else if(cassoclist->release_function != DEFAULT_RELEASE_FUNCTION){
		cassoclist->release_function(get_value_by_offset(cassoclist,offset));
	}
	get_used_flag(element_info) = 0;
	cassoclist->size--;
	return CASSOCLIST_SUCCESS;
}
