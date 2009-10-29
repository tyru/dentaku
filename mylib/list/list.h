#ifndef LIST_H
#define LIST_H


#include <stdlib.h>


/* Some macros */
#define list_head(lis)      ((lis)->head)
#define list_tail(lis)      ((lis)->tail)
#define list_destruct(lis)  list_destruct_func((lis), (lis)->rel_func)
#define list_erase(lis, begin, end)     list_erase_func((lis), (begin), (end), (lis)->rel_func)
// If list is empty, Both lis->head and lis->tail are NULL.
#define list_empty(lis)     ((lis)->head == NULL)

#define LIST_GET_NODE(node, type)   (*(type*)(node)->item)



typedef void (*list_release_func)(void *);
typedef void *(*list_copy_func)(void *, const void *, size_t);



typedef enum {
    LIST_RET_SUCCESS,
    LIST_RET_ARGS,
    LIST_RET_ALLOC,
    LIST_RET_EMPTY,
} ListRVal;


typedef struct Node_tag {
    struct Node_tag     *next;
    struct Node_tag     *prev;
    void                *item;
} Node;

typedef struct List_tag {
    Node                *head;
    Node                *tail;
    size_t              elem_size;
    list_release_func   rel_func;
    list_copy_func      copy_func;
} List;


// handler to free item.
// this is called before freeing list.
typedef void (*ListFunc)(void *item);




List*
list_init(size_t elem_size);

List*
list_init_with_func(size_t elem_size,
        list_release_func rel_func,
        list_copy_func copy_func);

ListRVal
list_destruct_func(List *lis, ListFunc func);

size_t
list_length(List *lis);

Node*
list_get_node_idx(List *lis, size_t idx);

ListRVal
list_erase_func(List *lis, Node *begin, Node *end, ListFunc func);

ListRVal
list_push_front(List *lis, void *ptr);

ListRVal
list_push_back(List *lis, void *ptr);

ListRVal
list_pop_front(List *lis, void *ptr);

ListRVal
list_pop_back(List *lis, void *ptr);



#endif /* LIST_H */
