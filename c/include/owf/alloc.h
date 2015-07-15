#include <owf.h>
#include <owf/error.h>

#include <stdlib.h>
#include <string.h>

#ifndef OWF_ALLOC_H
#define OWF_ALLOC_H

/* Max allocation size: 1 MB. This can be overridden when initializing the owf_alloc_t.
 * If we ever try to allocate a size bigger than this, the allocation function will not be called
 * and will return NULL.
 */
#define OWF_ALLOC_DEFAULT_MAX 1048576

/* A malloc callback.
 *
 * Takes the size of the allocation, which will never be 0.
 */
typedef void *(*owf_malloc_cb_t)(size_t);

/* A realloc callback.
 *
 * Takes the block pointer, which will never be NULL, and
 * the size of the new allocation, which will never be 0.
 */
typedef void *(*owf_realloc_cb_t)(void *, size_t);

/* A free callback.
 *
 * Takes the block pointer, which will never be NULL.
 */
typedef void (*owf_free_cb_t)(void *);

/* A struct holding information about memory allocation functions. */
typedef struct owf_alloc owf_alloc_t;

/* @see owf_alloc_t */
struct owf_alloc {
    /* The malloc callback. */
    owf_malloc_cb_t malloc;

    /* The realloc callback. */
    owf_realloc_cb_t realloc;

    /* The free callback. */
    owf_free_cb_t free;

    /* The maximum size we are willing to allocate. */
    size_t max_alloc;
};

/* Initializes an <owf_alloc_t> structure.
 * @alloc A pointer to an <owf_alloc_t>
 * @malloc_fn A malloc callback
 * @realloc_fn A realloc callback
 * @free_fn A free callback
 * @max_alloc The maximum size that we should ever allocate in a single allocation.
 */
void owf_alloc_init(owf_alloc_t *alloc, owf_malloc_cb_t malloc_fn, owf_realloc_cb_t realloc_fn, owf_free_cb_t free_fn, size_t max_alloc);

/* Calls the provided malloc callback. Will never pass it a zero or too large size.
 * @alloc A pointer to an <owf_alloc_t>
 * @error A pointer to an <owf_error_t> to store potential errors resulting from the call
 * @size The size of the allocation
 *
 * @return A pointer to an allocated block of at least size `size`, or NULL on error.
 *         If this function returns NULL, an error will be set in the <owf_error_t>.
 */
void *owf_malloc(owf_alloc_t *alloc, owf_error_t *error, size_t size);

/* Calls the provided realloc callback. Will never pass it a NULL pointer, zero size, or too large size.
 * @alloc A pointer to an <owf_alloc_t>
 * @error A pointer to an <owf_error_t> to store potential errors resulting from the call
 * @bp A pointer to a pointer to the previously allocated block
 * @size The size of the allocation
 *
 * @return True on success, or false on failure. On failure, the input pointer is not freed or changed.
 *         If this function returns true, bp will never be set to NULL. Note that this breaks the glibc
 *         behavior of passing in a zero size to realloc, but that behavior is harmful for our uses anyway.
 */
bool owf_realloc(owf_alloc_t *alloc, owf_error_t *error, void **bp, size_t size);

/* Calls the provided free callback. Will never pass it a NULL pointer.
 * @alloc A pointer to an <owf_alloc_t>
 * @bp A pointer to an allocated block
 */
void owf_free(owf_alloc_t *alloc, void *bp);

#endif /* OWF_ALLOC_H */
