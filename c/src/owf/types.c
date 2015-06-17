#include <owf/types.h>

bool owf_str_init(owf_str_t *str, owf_alloc_t *allocator, owf_error_t *error, uint32_t length) {
    owf_array_init(&str->bytes);
    return owf_array_reserve_exactly(&str->bytes, allocator, error, sizeof(uint8_t), length + 1);
}

uint32_t owf_str_bytesize(owf_str_t *str) {
    /* Extra for the trailing null byte that was added */
    return (uint32_t)strnlen(str->bytes.ptr, str->bytes.length - 1);
}
