#include <owf.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef OWF_ERROR_H
#define OWF_ERROR_H

/* The error buffer size. */
#define OWF_ERROR_BUF_SIZE 256

/* Stores information about errors occurring during OWF operations. */
typedef struct owf_error owf_error_t;

/* @see owf_error_t */
struct owf_error {
    /** Buffer to store error strings */
    char error[OWF_ERROR_BUF_SIZE];

    /** Whether this is actually an error */
    bool is_error;
};

/* Initializes an <owf_error_t>.
 * @error The <owf_error_t> to initialize
 * Note that the same thing can be performed using code like:
 *     `owf_error_t error = OWF_ERROR_DEFAULT;`
 */
void owf_error_init(owf_error_t *error);

/* Sets the error in an <owf_error_t>.
 * @error The <owf_error_t> object.
 * @fmt The format string (see printf)
 */
void owf_error_set(owf_error_t *error, const char *fmt, ...);

/* Returns true if an error occurred.
 * @error The <owf_error_t> to test.
 * Note that all functions that populate an <owf_error_t> also have a
 * return value indicating that an error occurred.
 *
 * @return True if an error occurred, false othereise
 */
bool owf_error_test(owf_error_t *error);

/* Returns a pointer to an error string for this <owf_error_t>.
 * @error The <owf_error_t>
 *
 * @return A pointer to a read-only NULL-terminated string
 */
const char *owf_error_strerror(owf_error_t *error);

/* Sets the error to `fmt`.
 * @_err The error pointer
 * @_fmt The format
 */
#define OWF_ERROR_SET(_err, _fmt) do {owf_error_set(_err, "%s:%d@%s: " _fmt, __FILE__, __LINE__, __FUNCTION__); } while (0)

/* Sets the error to `fmt`, processing the given format args.
 * @_err The error pointer
 * @_fmt The format
 */
#define OWF_ERROR_SETF(_err, _fmt, ...) do {owf_error_set(_err, "%s:%d@%s: " _fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while (0)

/* The default <owf_error_t> state: no error, no message */
#define OWF_ERROR_DEFAULT {.error = {0}, .is_error = false}

#endif /* OWF_ERROR_H */
