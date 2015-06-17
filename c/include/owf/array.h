#include <owf.h>
#include <owf/alloc.h>

#ifndef OWF_ARRAY_H
#define OWF_ARRAY_H

typedef struct owf_array {
    void *ptr;
    uint32_t length, capacity;
} owf_array_t;

#define OWF_ARRAY_TYPED_PTR(arr, type) ((type *)((&(arr))->ptr))
#define OWF_ARRAY_PTR(arr, type, idx) (&OWF_ARRAY_TYPED_PTR(arr, type)[idx])
#define OWF_ARRAY_GET(arr, type, idx) (OWF_ARRAY_TYPED_PTR(arr, type)[idx])
#define OWF_ARRAY_PUT(arr, type, idx, value) do {OWF_ARRAY_GET(arr, type, idx) = value;} while (0)
#define OWF_ARRAY_LEN(arr) ((&(arr))->length)

void owf_array_init(owf_array_t *arr);
void owf_array_destroy(owf_array_t *arr, owf_alloc_t *allocator);
bool owf_array_reserve(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width);
bool owf_array_reserve_exactly(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width);
bool owf_array_push(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, void *obj, uint32_t width);
bool owf_array_put(owf_array_t *arr, owf_error_t *error, void *obj, uint32_t idx, uint32_t width);
void *owf_array_at(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width);
void *owf_array_ptr_for(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width);

#endif /* OWF_ARRAY_H */
