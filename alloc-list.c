/*
 * This is not GC implementation.
 * Just save allocated pointers to AllocList.
 */

#include "alloc-list.h"
#include "util.h"

// I want to see the definition of struct List_tag,
// But I don't want to see typedef of List
// to avoid multiple List's typedef definition.
#define List    List__
    #include "nstl/list/list.h"
#undef List

static List    *pointers_list = NULL;




static void
free_func(void *ptr)
{
    free(*(void**)ptr);
    *(void**)ptr = NULL;
    free(ptr);
}
static void*
copy_func(void *dest, const void *src, size_t n)
{
    UNUSED(n);
    *(void**)dest = *(void**)src;
    return dest;
}


void
al_init(void)
{
    pointers_list = list_init(sizeof(void*));
    list_set_free_func(pointers_list, free_func);
    list_set_copy_func(pointers_list, copy_func);
}


/*
 * You(who?) can use perror()
 * if this function's return value is NULL.
 */
void*
al_malloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        return NULL;
    }
    if (list_push_back(pointers_list, &p) != NSTL_OK) {
        DIE("something wrong: list_push_back()\n");
    }
    return p;
}

/*
 * You(who?) can use perror()
 * if this function's return value is NULL.
 */
void*
al_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        return NULL;
    }
    if (new_ptr != ptr) {
        if (list_remove(pointers_list, &ptr) != NSTL_OK) {
            DIE("something wrong: list_remove()");
        }
        if (list_push_back(pointers_list, &new_ptr) != NSTL_OK) {
            DIE("something wrong: list_push_back()");
        }
    }
    return new_ptr;
}


void
al_free_pointers(void)
{
    list_clear(pointers_list);
}


void
al_destroy(void)
{
    list_destruct(pointers_list);
}
