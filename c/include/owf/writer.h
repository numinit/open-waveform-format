#include <owf.h>
#include <owf/types.h>
#include <owf/error.h>
#include <owf/alloc.h>

#include <stdio.h>
#include <string.h>

#ifndef OWF_WRITER_H
#define OWF_WRITER_H

typedef struct owf_writer owf_writer_t;

typedef bool (*owf_write_cb_t)(const void *, const size_t, void *);

/* Abstracts a writer.
 * Writers store the current state of a write operation.
 */
struct owf_writer {
    /* Error status */
    owf_error_t *error;

    /* The allocator */
    owf_alloc_t *alloc;

    /* Callbacks */
    owf_write_cb_t write;

    /* User data */
    void *data;
};

/* Initializes an <owf_writer_t>.
 *
 * @writer The writer
 * @alloc The allocator
 * @error The error context
 * @write The write callback
 * @data User data supplied to the write callback
 */
void owf_writer_init(owf_writer_t *writer, owf_alloc_t *alloc, owf_error_t *error, owf_write_cb_t write, void *data);

#endif /* OWF_WRITER_H */
