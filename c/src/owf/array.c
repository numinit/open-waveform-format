#include <owf/array.h>
#include <owf/types.h>
#include <owf/arith.H>

void owf_array_init(owf_array_t *arr) {
    arr->ptr = NULL;
    arr->length = 0;
    arr->capacity = 0;
}

void owf_array_destroy(owf_array_t *arr, owf_alloc_t *allocator) {
    owf_free(allocator, arr->ptr);
}

bool owf_array_reserve(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width) {
    /*
     * Extend the capacity by a factor of 3/2.
     * Do a safe multiply by 3, then divide by 2.
     */
    capacity = owf_arith_safe_mul32(capacity, 3, error);
    if (OWF_NOEXPECT(error->is_error)) {
        return false;
    } else {
        capacity /= 2;
    }

    return owf_array_reserve_exactly(arr, allocator, error, capacity, width);
}

bool owf_array_reserve_exactly(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width) {
    void *ptr = arr->ptr;
    uint32_t new_size;

    /* Calculate the new total size */
    new_size = owf_arith_safe_mul32(capacity, width, error);
    if (OWF_NOEXPECT(error->is_error)) {
        return false;
    } else if (OWF_NOEXPECT(new_size == 0)) {
        OWF_ERR_SET(*error, "tried to reserve zero-byte length");
        return false;
    }

    /* Reallocate */
    ptr = owf_realloc(allocator, error, ptr, new_size);
    if (OWF_NOEXPECT(ptr == NULL)) {
        return false;
    }

    /* Commit and refit the array length */
    arr->ptr = ptr;
    arr->capacity = capacity;
    arr->length = OWF_MIN(arr->length, capacity);
    return true;
}

bool owf_array_push(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, void *obj, uint32_t width) {
    if (OWF_NOEXPECT(arr->length == arr->capacity && !owf_array_reserve(arr, allocator, error, arr->capacity + 1, width))) {
        return false;
    }

    return owf_array_put(arr, error, obj, arr->length++, width);
}

bool owf_array_put(owf_array_t *arr, owf_error_t *error, void *obj, uint32_t idx, uint32_t width) {
    void *ptr = owf_array_ptr_for(arr, error, idx, width);
    if (OWF_NOEXPECT(ptr == NULL)) {
        return false;
    }

    /* Commit */
    memcpy(ptr, obj, width);
    return true;
}

void *owf_array_at(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width) {
    if (OWF_EXPECT(idx < arr->length)) {
        return owf_array_ptr_for(arr, error, idx, width);
    } else {
        OWF_ERR_SETF(*error, "array index out of bounds: %" PRIu32 " >= %" PRIu32, idx, arr->length);
        return NULL;
    }
}

void *owf_array_ptr_for(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width) {
    uint32_t offset = owf_arith_safe_mul32(idx, width, error);
    if (OWF_NOEXPECT(error->is_error)) {
        return NULL;
    } else {
        return (uint8_t *)arr->ptr + offset;
    }
}
