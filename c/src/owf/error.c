#include <owf/error.h>

void owf_error_init(owf_error_t *error) {
    error->is_error = false;
    error->error[0] = 0;
}

void owf_error_set(owf_error_t *error, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    error->is_error = true;
    vsnprintf(error->error, sizeof(error->error), fmt, va);
    va_end(va);
}