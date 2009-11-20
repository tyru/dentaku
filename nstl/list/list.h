#ifndef NSTL_LIST_H
#define NSTL_LIST_H

#include "../common.h"



/* Feature Macros */
#ifndef LIST_SIZE_CONST_TIME
    #define LIST_SIZE_CONST_TIME 0
#endif



/* Macros */
#define list_front(lis) \
    ((lis)->front)

#define list_back(lis) \
    ((lis)->back)

// If list is empty, Both lis->front and lis->back are NULL.
#define list_empty(lis) \
    ((lis)->front == NULL)

#define list_clear(lis) \
    list_erase((lis), (lis)->front, NULL)

#define list_remove(lis, ptr) \
    list_remove_front_n((lis), (ptr), 0)

#if LIST_SIZE_CONST_TIME
    #define list_size(lis) \
        (lis)->size
#endif



#define list_set_copy_func(lis, func)                  \
    do {                                               \
        if ((ListCopyFunc)(func) != (ListCopyFunc)0) { \
            (lis)->copy_func = (ListCopyFunc)(func);   \
        }                                              \
    } while (0)                                        \

#define list_set_alloc_func(lis, func)                   \
    do {                                                 \
        if ((ListAllocFunc)(func) != (ListAllocFunc)0) { \
            (lis)->alloc_func = (ListAllocFunc)(func);   \
        }                                                \
    } while (0)                                          \

#define list_set_free_func(lis, func)                  \
    do {                                               \
        if ((ListFreeFunc)(func) != (ListFreeFunc)0) { \
            (lis)->free_func = (ListFreeFunc)(func);   \
        }                                              \
    } while (0)                                        \

#define list_set_compare_func(lis, func)                     \
    do {                                                     \
        if ((ListCompareFunc)(func) != (ListCompareFunc)0) { \
            (lis)->compare_func = (ListCompareFunc)(func);   \
        }                                                    \
    } while (0)                                              \



/* Accessors */
#define LIST_GET_NODE(node, type)   (*(type*)(node)->item)



/* Iterators */
#define list_iter_begin(lis) \
    list_front(lis)

#define list_iter_end(lis) \
    NULL

#define list_iter_next(node) \
    (node)->next

#define list_iter_prev(node) \
    (node)->prev

#define list_iter_incl(node) \
    ((node) = (node)->next)

#define list_iter_decl(node) \
    ((node) = (node)->prev)



typedef void *(*ListCopyFunc)(void *, const void *, size_t);
typedef void *(*ListAllocFunc)(size_t);
typedef void (*ListFreeFunc)(void *);
typedef int  (*ListCompareFunc)(const void *, const void *, size_t);



typedef struct Node_tag {
    struct Node_tag     *next;
    struct Node_tag     *prev;
    void                *item;
} Node;

typedef struct List_tag {
    Node                *front;
    Node                *back;
    size_t              elem_size;
#if LIST_SIZE_CONST_TIME
    size_t              size;
#endif
    ListCopyFunc        copy_func;
    ListAllocFunc       alloc_func;
    ListFreeFunc        free_func;
    ListCompareFunc     compare_func;
} List;




List*
list_init(size_t elem_size);

List*
list_init_func(
    size_t          elem_size,
    ListCopyFunc    copy_func,
    ListAllocFunc   alloc_func,
    ListFreeFunc    free_func,
    ListCompareFunc compare_func);

void
list_destruct(List *lis);

#if !LIST_SIZE_CONST_TIME
size_t
list_size(List *lis);
#endif

Node*
list_find_idx(List *lis, size_t idx);

Node*
list_find(List *lis, void *ptr);

NstlErrno
list_remove_range_n(
        List *lis,
        Node *begin,
        Node *end,
        void *ptr,
        size_t n,
        bool ascending);

NstlErrno
list_erase(List *lis, Node *begin, Node *end);

NstlErrno
list_remove_front_n(List *lis, void *ptr, size_t n);

NstlErrno
list_remove_back_n(List *lis, void *ptr, size_t n);

NstlErrno
list_push_front(List *lis, void *ptr);

NstlErrno
list_push_back(List *lis, void *ptr);

NstlErrno
list_pop_front(List *lis, void *ptr);

NstlErrno
list_pop_back(List *lis, void *ptr);



#endif /* NSTL_LIST_H */
