#include <owf.h>

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
#define OWF_READER_ERR_BUF_SIZE 128

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

typedef bool (*owf_read_cb_t)(void *, size_t, void *);
typedef bool (*owf_visit_cb_t)(owf_reader_ctx_t *, owf_reader_cb_type_t, void *);
typedef void *(*owf_alloc_cb_t)(size_t);
typedef void (*owf_free_cb_t)(void *);

/**
 * Abstracts a reader.
 * Readers store the current state of the read operation.
 */
typedef struct owf_reader {
    /** Context for what we're currently reading */
    owf_reader_ctx_t ctx;

    /** Buffer to store error strings */
    char error[OWF_READER_ERR_BUF_SIZE];

    /** Callbacks */
    owf_read_cb_t read;
    owf_visit_cb_t visit;
    owf_alloc_cb_t alloc;
    owf_free_cb_t free;

    /** User data */
    void *data;

    /** Max allocation size */
    size_t max_alloc;
} owf_reader_t;

void owf_reader_init(owf_reader_t *reader, owf_alloc_cb_t alloc_fn, owf_free_cb_t free_fn, owf_read_cb_t read, owf_visit_cb_t visitor, size_t max_alloc, void *data);

#define OWF_READER_ERR(reader, fmt) \
    do {snprintf((&(reader))->error, sizeof((&(reader))->error), (fmt));} while (0)
#define OWF_READER_ERRF(reader, fmt, ...) \
    do {snprintf((&(reader))->error, sizeof((&(reader))->error), (fmt), __VA_ARGS__);} while (0)
#define OWF_READER_VISIT(reader, type) \
    (((&(reader))->visit != NULL && (&(reader))->visit(&(&(reader))->ctx, type, (&(reader))->data)))

#endif /* OWF_READER_H */
