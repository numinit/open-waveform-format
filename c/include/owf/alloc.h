#include <owf.h>
#include <owf/error.h>
#include <owf/platform.h>

#include <stdlib.h>
#include <string.h>

#ifndef OWF_ALLOC_H
#define OWF_ALLOC_H

/**
 * Max allocation size: 1 MB. This can be overridden when initializing the owf_alloc_t.
 * If we ever try to allocate a size bigger than this, the allocation function will not be called
 * and will return NULL.
 */
#define OWF_ALLOC_DEFAULT_MAX 1048576

typedef void *(*owf_malloc_cb_t)(size_t);
typedef void *(*owf_realloc_cb_t)(void *, size_t);
typedef void (*owf_free_cb_t)(void *);

typedef struct owf_alloc {
    owf_malloc_cb_t malloc;
    owf_realloc_cb_t realloc;
    owf_free_cb_t free;
    size_t max_alloc;
} owf_alloc_t;

void owf_alloc_init(owf_alloc_t *alloc, owf_malloc_cb_t malloc_fn, owf_realloc_cb_t realloc_fn, owf_free_cb_t free_fn, size_t max_alloc);
void *owf_malloc(owf_alloc_t *alloc, owf_error_t *error, size_t size);
void *owf_realloc(owf_alloc_t *alloc, owf_error_t *error, void *bp, size_t size);
void owf_free(owf_alloc_t *alloc, void *bp);

#endif /* OWF_ALLOC_H */
