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
        /* The size is too small */
        OWF_ERROR_SET(error, "can't allocate zero bytes");
        return NULL;
    } else if (OWF_NOEXPECT(size > alloc->max_alloc)) {
        /* The size is too big */
        OWF_ERROR_SETF(error, "allocated size was greater than max (" OWF_PRINT_SIZE " > " OWF_PRINT_SIZE ")", size, alloc->max_alloc);
        return NULL;
    } else {
        /* This is just malloc */
        void *ret = alloc->malloc(size);
        if (OWF_NOEXPECT(ret == NULL)) {
            OWF_ERROR_SET(error, "malloc failure");
            return NULL;
        } else {
            return ret;
        }
    }
}

bool owf_realloc(owf_alloc_t *alloc, owf_error_t *error, void **bp, size_t size) {
    void *new_bp = NULL;

    if (OWF_NOEXPECT(bp == NULL)) {
        OWF_ERROR_SET(error, "no reallocation pointer provided");
        return false;
    } else if (OWF_NOEXPECT(size == 0)) {
        /* This would be free, but we want to avoid setting bp to NULL and returning success */
        OWF_ERROR_SET(error, "cannot call realloc with size of zero");
        return false;
    } else if (OWF_NOEXPECT(*bp == NULL)) {
        /* This is just malloc */
        new_bp = owf_malloc(alloc, error, size);
        if (OWF_EXPECT(new_bp != NULL)) {
            *bp = new_bp;
            return true;
        } else {
            /* owf_malloc set our error */
            return false;
        }
    } else if (OWF_NOEXPECT(size > alloc->max_alloc)) {
        /* This is realloc, but the size is too big */
        OWF_ERROR_SETF(error, "reallocated size was greater than max (" OWF_PRINT_SIZE " > " OWF_PRINT_SIZE ")", size, alloc->max_alloc);
        return false;
    } else {
        /* This is realloc */
        new_bp = alloc->realloc(*bp, size);
        if (OWF_NOEXPECT(new_bp == NULL)) {
            OWF_ERROR_SET(error, "realloc failure");
            return false;
        } else {
            *bp = new_bp;
            return true;
        }
    }
}

void owf_free(owf_alloc_t *alloc, void *bp) {
    if (OWF_EXPECT(bp != NULL)) {
        alloc->free(bp);
    }
}
