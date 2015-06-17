#include <owf.h>
#include <owf/types.h>
#include <owf/error.h>
#include <owf/alloc.h>

#include <stdio.h>
#include <string.h>

#ifndef OWF_READER_H
#define OWF_READER_H

/**
 * Max allocation size: 1 MB. This can be overridden when initializing the owf_reader_t.
 * If we ever try to allocate a size bigger than this, the allocation function will not be called
 * and the unpack will fail.
 */
#define OWF_READER_DEFAULT_MAX_ALLOC 1048576

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
    owf_channel_t channel;
    owf_namespace_t ns;
    owf_signal_t signal;
    owf_event_t event;
    owf_alarm_t alarm;
} owf_reader_ctx_t;

typedef bool (*owf_reader_read_cb_t)(void *, size_t, void *);
typedef bool (*owf_reader_visit_cb_t)(owf_reader_ctx_t *, owf_reader_cb_type_t, void *);

/**
 * Abstracts a reader.
 * Readers store the current state of the read operation.
 */
typedef struct owf_reader {
    /** Error status */
    owf_error_t error;

    /** Context for what we're currently reading */
    owf_reader_ctx_t ctx;

    /** The allocator */
    owf_alloc_t *alloc;

    /** Callbacks */
    owf_reader_read_cb_t read;
    owf_reader_visit_cb_t visit;

    /** User data */
    void *data;
} owf_reader_t;

void owf_reader_init(owf_reader_t *reader, owf_alloc_t *alloc, owf_reader_read_cb_t read, owf_reader_visit_cb_t visitor, void *data);
bool owf_reader_is_error(owf_reader_t *reader);
const char *owf_reader_strerror(owf_reader_t *reader);

#define OWF_READER_VISIT(reader, type) \
    (((&(reader))->visit != NULL && (&(reader))->visit(&(&(reader))->ctx, type, (&(reader))->data)))

#define OWF_READER_ERR(reader, err) \
    do { OWF_ERR_SET((&(reader))->error, err); } while (0)

#define OWF_READER_ERRF(reader, err, ...) \
    do { OWF_ERR_SETF((&(reader))->error, err, __VA_ARGS__); } while (0)

#endif /* OWF_READER_H */
