#include <owf.h>
#include <owf/types.h>
#include <owf/error.h>
#include <owf/alloc.h>

#include <stdio.h>
#include <string.h>

#ifndef OWF_READER_H
#define OWF_READER_H

/* The current type being read. */
typedef enum owf_reader_cb_type {
    OWF_READ_CHANNEL,
    OWF_READ_NAMESPACE,
    OWF_READ_SIGNAL,
    OWF_READ_EVENT,
    OWF_READ_ALARM
} owf_reader_cb_type_t;

/* Abstracts a context.
 * Contexts store the current objects being read.
 */
typedef struct owf_reader_ctx {
    owf_package_t owf;
    owf_channel_t channel;
    owf_namespace_t ns;
    owf_signal_t signal;
    owf_event_t event;
    owf_alarm_t alarm;
} owf_reader_ctx_t;

typedef struct owf_reader owf_reader_t;

/* A read callback. Takes a destination, a size, and a data pointer. */
typedef bool (*owf_read_cb_t)(void *, const size_t, void *);

/* A visit callback. Takes a reader, a reader context, the type we're reading, and a data pointer. */
typedef bool (*owf_visit_cb_t)(owf_reader_t *, owf_reader_ctx_t *, owf_reader_cb_type_t, void *);

/* Abstracts a reader.
 * Readers store the current state of the read operation.
 */
struct owf_reader {
    /* Context for what we're currently reading */
    owf_reader_ctx_t ctx;

    /* Error status */
    owf_error_t *error;

    /* The allocator */
    owf_alloc_t *alloc;

    /* The read callback */
    owf_read_cb_t read;

    /* The visit callback */
    owf_visit_cb_t visit;

    /* User data */
    void *data;
};

/* Initializes an <owf_reader_t>.
 *
 * @reader The reader
 * @alloc The allocator
 * @error The error context
 * @read The read callback
 * @visitor The visit callback
 * @data User data supplied to both the read callback and the visit callback
 */
void owf_reader_init(owf_reader_t *reader, owf_alloc_t *alloc, owf_error_t *error, owf_read_cb_t read, owf_visit_cb_t visitor, void *data);

/* A visitor callback used to materialize packets.
 *
 * @reader The reader
 * @ctx The context
 * @type The type currently being read
 * @ptr A pointer to user data
 */
bool owf_reader_materialize_cb(owf_reader_t *reader, owf_reader_ctx_t *ctx, owf_reader_cb_type_t type, void *ptr);

/* Invokes the visitor callback.
 * @_reader The reader
 * @_type   The type currently being read
 */
#define OWF_READER_VISIT(_reader, _type) \
    (((&(_reader))->visit != NULL && (&(_reader))->visit(&(_reader), &(&(_reader))->ctx, _type, (&(_reader))->data)))

#endif /* OWF_READER_H */
