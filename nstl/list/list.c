/*
 * NSTL - list
 * Written by tyru
 *
 * See LICENSE in NSTL's directory about the license.
 */

#include "list.h"

#include <string.h>
#include <assert.h>

#define COMPARE_EQ(lis, item1, item2) \
    ((lis)->compare_func((item1), (item2), (lis)->elem_size) == 0)





/*
 * Delete node from node's list.
 */
static NstlErrno
free_node(Node *node, ListFreeFunc free_func)
{
    assert(node);

    // Remove current node from chained other nodes.
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }

    // Call handler to destruct node->item.
    free_func(node->item);
    node->item = NULL;
    // Free node itself.
    free(node);

    return NSTL_OK;
}


/*
 * Create node with src.
 */
static Node*
create_node(
    void *src,
    size_t item_size,
    Node *prev,
    Node *next,
    ListCopyFunc copy_func,
    ListAllocFunc alloc_func)
{
    Node *node;
    void *dest;

    node = malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    // Chain like prev -> node -> next.
    node->prev = prev;
    node->next = next;
    if (prev) {
        prev->next = node;
    }
    if (next) {
        next->prev = node;
    }

    dest = alloc_func(item_size);
    if (dest == NULL) {
        return NULL;
    }
    // Copy with copy_func. This is default to 'memcpy'.
    copy_func(dest, src, item_size);
    node->item = dest;

    return node;
}






/*
 * This returns allocated (List *).
 * If any error happened, This returns NULL.
 */
List*
list_init(size_t elem_size)
{
    List *lis = malloc(sizeof(List));
    if (lis == NULL) {
        return NULL;
    }

    list_front(lis) = NULL;
    list_back(lis)  = NULL;
    lis->elem_size  = elem_size;

#if LIST_SIZE_CONST_TIME
    lis->size = 0;
#endif

    lis->copy_func    = memcpy;
    lis->alloc_func   = malloc;
    lis->free_func    = free;
    lis->compare_func = memcmp;

    return lis;
}


/*
 * Set functions for element.
 * If you don't want to change the default function,
 * You can use 0 not to change it.
 */
List*
list_init_func(
    size_t          elem_size,
    ListCopyFunc    copy_func,
    ListAllocFunc   alloc_func,
    ListFreeFunc    free_func,
    ListCompareFunc compare_func)
{
    List *lis = list_init(elem_size);
    if (lis == NULL) {
        return NULL;
    }

    list_set_copy_func(lis, copy_func);
    list_set_alloc_func(lis, alloc_func);
    list_set_free_func(lis, free_func);
    list_set_compare_func(lis, compare_func);
    return lis;
}


void
list_destruct(List *lis)
{
    assert(lis);

    if (list_front(lis)) {
        list_clear(lis);
    }
    free(lis);
}






#if !LIST_SIZE_CONST_TIME
size_t
list_size(List *lis)
{
    size_t count = 0;
    Node *begin  = list_front(lis);
    while (begin) {
        begin = begin->next;
        count++;
    }
    return count;
}
#endif


/*
 * Get node of idx.
 * This is same as lis->next->next->next ... as idx.
 */
Node*
list_find_idx(List *lis, size_t idx)
{
    size_t i = 0;
    Node *begin = list_front(lis);
    while (i < idx) {
        if (begin == NULL) {
            return NULL;
        }
        begin = begin->next;
        i++;
    }
    return begin;
}


/*
 * Get node matching ptr.
 *
 * XXX: not tested yet.
 */
Node*
list_find(List *lis, void *ptr)
{
    Node *begin = list_front(lis);
    while (begin) {
        if (COMPARE_EQ(lis, ptr, begin->item)) {
            return begin;
        }
        begin = begin->next;
    }
    return NULL;
}






/*
 * Remove
 * - n elements (at most or as possible if n == 0)
 * - matching ptr (Any element if ptr == NULL)
 * - in ascending/descending order
 *
 * NOTE: begin must NOT be NULL !
 */
NstlErrno
list_remove_range_n(
    List *lis,
    Node *begin,
    Node *end,
    void *ptr,
    size_t n,
    bool ascending)
{
    Node *next_node;
    NstlErrno ret_val;
    Node *node_before_range, **top_node, **bottom_node;
#if LIST_SIZE_CONST_TIME
    size_t count = 0;
#endif

    assert(lis);
    assert(begin);

    // If ascending:
    //      Advance to begin->next
    // If not ascending:
    //      Advance to begin->prev
    if (ascending) {
        node_before_range = begin->prev;
        top_node          = &list_front(lis);
        bottom_node       = &list_back(lis);
    }
    else {
        node_before_range = begin->next;
        top_node          = &list_back(lis);
        bottom_node       = &list_front(lis);
    }

    while (begin != end) {
        next_node = ascending ? begin->next : begin->prev;

        if (ptr == NULL || (ptr && COMPARE_EQ(lis, ptr, begin->item))) {
            if (*top_node == begin) {
                *top_node = next_node;
            }
            if (*bottom_node == begin && end == NULL) {
                *bottom_node = node_before_range;
            }

            ret_val = free_node(begin, lis->free_func);
            if (ret_val != NSTL_OK) {
                return ret_val;
            }
#if LIST_SIZE_CONST_TIME
            count--;
#endif
            if (n != 0 && --n == 0) {
#if LIST_SIZE_CONST_TIME
                lis->size -= count;
#endif
                return ret_val;
            }
        }

        begin = next_node;
    }

    return NSTL_OK;
}


/*
 * Remove all elements in range [begin, end).
 */
NstlErrno
list_erase(List *lis, Node *begin, Node *end)
{
    assert(lis);
    if (begin == NULL) {
        // May I return success code here?
        return NSTL_OK;
    }
    return list_remove_range_n(lis, begin, end, NULL, 0, true);
}


/*
 * Remove
 * - n elements (at most or as possible if n == 0)
 * - matching ptr (Any element if ptr == NULL)
 */
NstlErrno
list_remove_front_n(List *lis, void *ptr, size_t n)
{
    assert(lis);
    if (list_front(lis) == NULL) {
        // May I return success code here?
        return NSTL_OK;
    }
    // Iterate from front to back.
    return list_remove_range_n(lis, list_front(lis), NULL, ptr, n, true);
}


NstlErrno
list_remove_back_n(List *lis, void *ptr, size_t n)
{
    assert(lis);
    if (list_back(lis) == NULL) {
        // May I return success code here?
        return NSTL_OK;
    }
    // Iterate from back to front.
    return list_remove_range_n(lis, list_back(lis), NULL, ptr, n, false);
}


NstlErrno
list_push_front(List *lis, void *ptr)
{
    Node *node;
    assert(lis);

    node = create_node(ptr, lis->elem_size, NULL, list_front(lis),
                      lis->copy_func, lis->alloc_func);
    if (node == NULL) {
        return NSTL_ALLOC;
    }

    if (list_empty(lis)) {
        list_front(lis) = list_back(lis) = node;
    }
    else {
        list_front(lis) = node;
    }

#if LIST_SIZE_CONST_TIME
    lis->size++;
#endif
    return NSTL_OK;
}


NstlErrno
list_push_back(List *lis, void *ptr)
{
    Node *node;
    assert(lis);

    node = create_node(ptr, lis->elem_size, list_back(lis), NULL,
                      lis->copy_func, lis->alloc_func);
    if (node == NULL) {
        return NSTL_ALLOC;
    }

    if (list_empty(lis)) {
        list_front(lis) = list_back(lis) = node;
    }
    else {
        list_back(lis) = node;
    }

#if LIST_SIZE_CONST_TIME
    lis->size++;
#endif
    return NSTL_OK;
}


NstlErrno
list_pop_front(List *lis, void *ptr)
{
    NstlErrno ret_val;
    Node *front_next;

    assert(lis);
    if (list_empty(lis))
        return NSTL_EMPTY;
    front_next = list_front(lis)->next;

    if (ptr)    // Pop to ptr.
        lis->copy_func(ptr, list_front(lis)->item, lis->elem_size);

    ret_val = free_node(list_front(lis), lis->free_func);
    if (ret_val != NSTL_OK)
        return ret_val;

    if (list_front(lis) == list_back(lis)) {    // There was only 1 element.
        list_front(lis) = list_back(lis) = NULL;
    }
    else {
        list_front(lis) = front_next;
    }

#if LIST_SIZE_CONST_TIME
    lis->size--;
#endif
    return NSTL_OK;
}


NstlErrno
list_pop_back(List *lis, void *ptr)
{
    NstlErrno ret_val;
    Node *back_prev;

    assert(lis);
    if (list_empty(lis)) {
        return NSTL_EMPTY;
    }
    back_prev = list_back(lis)->prev;

    if (ptr) {    // Pop to ptr.
        lis->copy_func(ptr, list_back(lis)->item, lis->elem_size);
    }

    ret_val = free_node(list_back(lis), lis->free_func);
    if (ret_val != NSTL_OK) {
        return ret_val;
    }

    if (list_front(lis) == list_back(lis)) {    // There was only 1 element.
        list_front(lis) = list_back(lis) = NULL;
    }
    else {
        list_back(lis) = back_prev;
    }

#if LIST_SIZE_CONST_TIME
    lis->size--;
#endif
    return NSTL_OK;
}


/*
 * XXX: not tested yet.
 */
NstlErrno
list_swap(List *to, List *from)
{
    List temp = *to;
    *to       = *from;
    *from     = temp;
    return NSTL_OK;
}
