#include <owf.h>
#include <owf/types.h>
#include <owf/error.h>
#include <owf/alloc.h>

#include <stdio.h>
#include <string.h>

#ifndef OWF_READER_H
#define OWF_READER_H

typedef enum owf_reader_cb_type {
    OWF_READ_CHANNEL,
    OWF_READ_NAMESPACE,
    OWF_READ_SIGNAL,
    OWF_READ_EVENT,
    OWF_READ_ALARM
} owf_reader_cb_type_t;

/**
 * Abstracts a context.
 * Contexts store the current objects being read.
 */
typedef struct owf_reader_ctx {
    owf_t owf;
    owf_channel_t channel;
    owf_namespace_t ns;
    owf_signal_t signal;
    owf_event_t event;
    owf_alarm_t alarm;
} owf_reader_ctx_t;

typedef struct owf_reader owf_reader_t;
typedef bool (*owf_read_cb_t)(void *, const size_t, void *);
typedef bool (*owf_visit_cb_t)(owf_reader_t *, owf_reader_ctx_t *, owf_reader_cb_type_t, void *);

/**
 * Abstracts a reader.
 * Readers store the current state of the read operation.
 */
struct owf_reader {
    /** Context for what we're currently reading */
    owf_reader_ctx_t ctx;
    
    /** Error status */
    owf_error_t *error;

    /** The allocator */
    owf_alloc_t *alloc;

    /** Callbacks */
    owf_read_cb_t read;
    owf_visit_cb_t visit;

    /** User data */
    void *data;
};

void owf_reader_init(owf_reader_t *reader, owf_alloc_t *alloc, owf_error_t *error, owf_read_cb_t read, owf_visit_cb_t visitor, void *data);
bool owf_reader_is_error(owf_reader_t *reader);
const char *owf_reader_strerror(owf_reader_t *reader);
bool owf_reader_materialize_cb(owf_reader_t *reader, owf_reader_ctx_t *ctx, owf_reader_cb_type_t type, void *ptr);

#define OWF_READER_VISIT(reader, type) \
    (((&(reader))->visit != NULL && (&(reader))->visit(&(reader), &(&(reader))->ctx, type, (&(reader))->data)))

#endif /* OWF_READER_H */
