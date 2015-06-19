#include <owf.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef OWF_ERROR_H
#define OWF_ERROR_H

#define OWF_ERROR_BUF_SIZE 128

typedef struct owf_error {
    /** Buffer to store error strings */
    char error[OWF_ERROR_BUF_SIZE];

    /** Whether this is actually an error */
    bool is_error;
} owf_error_t;

#define OWF_ERR_SET(err, fmt) \
    do { \
        (&(err))->is_error = true; \
        snprintf((&(err))->error, sizeof((&(err))->error), "%s:%d@%s: " fmt, __FILE__, __LINE__, __FUNCTION__); \
    } while (0)
#define OWF_ERR_SETF(err, fmt, ...) \
    do { \
        (&(err))->is_error = true; \
        snprintf((&(err))->error, sizeof((&(err))->error), "%s:%d@%s: " fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while (0)

#endif /* OWF_ERROR_H */
