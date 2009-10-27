
#include "list.h"

#include <string.h>
#include <assert.h>
#include <stdbool.h>



// TODO
// - add length to List's member if required by macro.
// - implement STL list's methods.






/*
 * Delete node from node's list.
 */
static ListRVal
free_node(Node *node, ListFunc func)
{
    assert(node);

    // Remove current node from chained other nodes.
    if (node->prev)
        node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;

    // Call handler to destruct node->item.
    if (func != 0)
        func(node->item);
    // Free node->item pointer.
    free(node->item);
    node->item = NULL;
    // Free node itself.
    free(node);

    return LIST_RET_SUCCESS;
}


/*
 * Create node with src.
 */
static Node*
create_node(void *src, size_t item_size, Node *prev, Node *next, list_copy_func copy_func)
{
    Node *node;
    void *dest;

    node = malloc(sizeof(Node));
    if (! node)
        return NULL;
    // Chain like prev -> node -> next.
    node->prev = prev;
    node->next = next;
    if (prev)
        prev->next = node;
    if (next)
        next->prev = node;

    dest = malloc(item_size);
    if (! dest)
        return NULL;
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
    if (! lis)
        return NULL;

    lis->head      = NULL;
    lis->tail      = NULL;
    lis->elem_size = elem_size;
    lis->rel_func  = 0;
    lis->copy_func = memcpy;

    return lis;
}


List*
list_init_with_func(size_t  elem_size,
        list_release_func   rel_func,
        list_copy_func      copy_func)
{
    List *lis = list_init(elem_size);
    if (! lis)
        return NULL;
    lis->rel_func  = rel_func;
    lis->copy_func = copy_func;
    return lis;
}


ListRVal
list_destruct_func(List *lis, ListFunc func)
{
    assert(lis);

    if (lis->head == NULL)
        return LIST_RET_SUCCESS;
    return list_erase_func(lis, lis->head, NULL, func);
}






/*
 * TODO Return length in constant time if required
 */
size_t
list_length(List *lis)
{
    size_t count = 0;
    Node *begin = lis->head;
    while (begin) {
        begin = begin->next;
        count++;
    }
    return count;
}


/*
 * Get node of idx.
 * This is same as lis->next->next->next ... to idx.
 *
 * NOTE: I want Lisp's macro...
 */
Node*
list_get_node_idx(List *lis, size_t idx)
{
    size_t i = 0;
    Node *begin = lis->head;
    while (i < idx) {
        if (! begin)
            return NULL;
        begin = begin->next;
        i++;
    }
    return begin;
}






/*
 * Free all nodes in range [begin, end).
 */

ListRVal
list_erase_func(List *lis, Node *begin, Node *end, ListFunc func)
{
    Node *next_node;
    ListRVal tmp, ret_val = LIST_RET_SUCCESS;
    Node *begin_orig, *begin_prev;
    bool head_deleted = false, tail_deleted = false;

    assert(lis);
    assert(begin);

    begin_orig = begin;
    begin_prev = begin->prev;

    while (begin && begin != end) {
        next_node = begin->next;

        if (lis->head == begin)
            head_deleted = true;
        if (lis->tail == begin)
            tail_deleted = true;

        // do not return on failure.
        tmp = free_node(begin, func);
        if (tmp != LIST_RET_SUCCESS)
            ret_val = tmp;

        begin = next_node;
    }

    if (head_deleted)
        lis->head = NULL;

    // I assume that all right elems
    // including current node of begin are deleted.
    // Because no tail is in range.
    //
    // Set last valid node to lis->tail.
    if (tail_deleted)
        lis->tail = begin_prev;

    return ret_val;
}


ListRVal
list_push_front(List *lis, void *ptr)
{
    Node *tmp;
    assert(lis);

    tmp = create_node(ptr, lis->elem_size, NULL, lis->head, lis->copy_func);
    if (! tmp)
        return LIST_RET_ALLOC;

    if (list_empty(lis)) {
        lis->head = lis->tail = tmp;
    }
    else {
        lis->head = tmp;
    }

    return LIST_RET_SUCCESS;
}


ListRVal
list_push_back(List *lis, void *ptr)
{
    Node *tmp;
    assert(lis);

    tmp = create_node(ptr, lis->elem_size, lis->tail, NULL, lis->copy_func);
    if (! tmp)
        return LIST_RET_ALLOC;

    if (list_empty(lis)) {
        lis->head = lis->tail = tmp;
    }
    else {
        lis->tail = tmp;
    }

    return LIST_RET_SUCCESS;
}


ListRVal
list_pop_front(List *lis, void *ptr)
{
    ListRVal ret_val;
    Node *head_next;

    assert(lis);
    if (list_empty(lis))
        return LIST_RET_EMPTY;
    head_next = lis->head->next;

    if (ptr)    // Pop to ptr.
        lis->copy_func(ptr, lis->head->item, lis->elem_size);

    ret_val = free_node(lis->head, lis->rel_func);
    if (ret_val != LIST_RET_SUCCESS)
        return ret_val;

    if (lis->head == lis->tail)    // There was only 1 element.
        lis->head = lis->tail = NULL;
    else
        lis->head = head_next;


    return LIST_RET_SUCCESS;
}


ListRVal
list_pop_back(List *lis, void *ptr)
{
    ListRVal ret_val;
    Node *tail_prev;

    assert(lis);
    if (list_empty(lis))
        return LIST_RET_EMPTY;
    tail_prev = lis->tail->prev;

    if (ptr)    // Pop to ptr.
        lis->copy_func(ptr, lis->tail->item, lis->elem_size);

    ret_val = free_node(lis->tail, lis->rel_func);
    if (ret_val != LIST_RET_SUCCESS)
        return ret_val;

    if (lis->head == lis->tail)    // There was only 1 element.
        lis->head = lis->tail = NULL;
    else
        lis->tail = tail_prev;

    return LIST_RET_SUCCESS;
}
