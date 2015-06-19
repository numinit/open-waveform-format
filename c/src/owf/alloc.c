#include <owf/alloc.h>
#include <owf/platform.h>

void owf_alloc_init(owf_alloc_t *alloc, owf_malloc_cb_t malloc_fn, owf_realloc_cb_t realloc_fn, owf_free_cb_t free_fn, size_t max_alloc) {
    alloc->malloc = malloc_fn;
    alloc->realloc = realloc_fn;
    alloc->free = free_fn;
    alloc->max_alloc = max_alloc;
}

void *owf_malloc(owf_alloc_t *alloc, owf_error_t *error, size_t size) {
    if (OWF_NOEXPECT(size == 0)) {
        OWF_ERR_SET(*error, "can't allocate zero bytes");
        return NULL;
    } else if (OWF_NOEXPECT(size > alloc->max_alloc)) {
        OWF_ERR_SETF(*error, "allocated size was greater than max (%zu > %zu)", size, alloc->max_alloc);
        return NULL;
    } else {
        void *ret = alloc->malloc(size);
        if (OWF_NOEXPECT(ret == NULL)) {
            OWF_ERR_SET(*error, "malloc failure");
        }

        return ret;
    }
}

void *owf_realloc(owf_alloc_t *alloc, owf_error_t *error, void *bp, size_t size) {
    if (OWF_NOEXPECT(size == 0 && bp != NULL)) {
        OWF_ERR_SET(*error, "can't reallocate zero bytes");
        owf_free(alloc, bp);
        return NULL;
    } else if (OWF_NOEXPECT(size == 0 && bp == NULL)) {
        OWF_ERR_SET(*error, "can't reallocate zero bytes, much less with a NULL pointer");
        return NULL;
    } else if (OWF_NOEXPECT(bp == NULL)) {
        return owf_malloc(alloc, error, size);
    } else if (OWF_NOEXPECT(size > alloc->max_alloc)) {
        OWF_ERR_SETF(*error, "reallocated size was greater than max (%zu > %zu)", size, alloc->max_alloc);
        owf_free(alloc, bp);
        return NULL;
    } else {
        bp = alloc->realloc(bp, size);
        if (OWF_NOEXPECT(bp == NULL)) {
            OWF_ERR_SET(*error, "realloc failure");
        }
        return bp;
    }
}

void owf_free(owf_alloc_t *alloc, void *bp) {
    alloc->free(bp);
}
