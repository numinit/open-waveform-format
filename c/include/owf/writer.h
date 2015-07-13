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

struct owf_writer {
    /** Error status */
    owf_error_t *error;

    /** The allocator */
    owf_alloc_t *alloc;

    /** Callbacks */
    owf_write_cb_t write;

    /** User data */
    void *data;
};

void owf_writer_init(owf_writer_t *writer, owf_alloc_t *alloc, owf_error_t *error, owf_write_cb_t write, void *data);
bool owf_writer_is_error(owf_writer_t *writer);
const char *owf_writer_strerror(owf_writer_t *writer);

#endif /* OWF_WRITER_H */
