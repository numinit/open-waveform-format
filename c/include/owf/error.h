#include <owf.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef OWF_ERROR_H
#define OWF_ERROR_H

#define OWF_ERROR_BUF_SIZE 256

typedef struct owf_error {
    /** Buffer to store error strings */
    char error[OWF_ERROR_BUF_SIZE];

    /** Whether this is actually an error */
    bool is_error;
} owf_error_t;

void owf_error_init(owf_error_t *error);
void owf_error_set(owf_error_t *error, const char *fmt, ...);

#define OWF_ERROR_SET(_err, _fmt) do {owf_error_set(_err, "%s:%d@%s: " _fmt, __FILE__, __LINE__, __FUNCTION__); } while (0)
#define OWF_ERROR_SETF(_err, _fmt, ...) do {owf_error_set(_err, "%s:%d@%s: " _fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while (0)
#define OWF_ERROR_DEFAULT {.error = {0}, .is_error = false}

#endif /* OWF_ERROR_H */
