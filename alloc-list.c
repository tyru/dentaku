/*
 * This is not GC implementation.
 * Just save allocated pointers to AllocList.
 */

#include "alloc-list.h"
#include "util.h"


static List    *pointers_list;




static void
release_func(void *ptr)
{
    void *item = *(void**)ptr;
    if (item)
        free(item);
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
    pointers_list = list_init_with_func(
        sizeof(void*),
        release_func,
        copy_func
    );
}


void*
al_malloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL)
        return NULL;
    if (list_push_back(pointers_list, &p) != LIST_RET_SUCCESS)
        DIE("something wrong: list_push_back(ptr_list, p)\n");
    return p;
}


void
al_free_pointers(void)
{
    list_destruct(pointers_list);
}


void
al_destroy(void)
{
    if (pointers_list) {
        free(pointers_list);
        pointers_list = NULL;
    }
}
