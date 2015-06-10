#include <owf.h>

#include <stdio.h>
#include <string.h>

#ifndef OWF_READER_H
#define OWF_READER_H

#define OWF_READER_ERR_BUF_SIZE 128

typedef enum owf_reader_cb_type {
    OWF_READ_CHANNEL,
    OWF_READ_NAMESPACE,
    OWF_READ_SIGNAL,
    OWF_READ_EVENT,
    OWF_READ_ALARM
} owf_reader_cb_type_t;

typedef bool (*owf_read_cb_t)(void *, size_t, void *);
typedef bool (*owf_visit_cb_t)(void *, owf_reader_cb_type_t, void *);
typedef void *(*owf_alloc_cb_t)(size_t);
typedef void (*owf_free_cb_t)(void *);

typedef struct owf_reader {
    owf_read_cb_t read;
    owf_visit_cb_t visit;
    owf_alloc_cb_t alloc;
    owf_free_cb_t free;
    void *data;
    char error[OWF_READER_ERR_BUF_SIZE];
} owf_reader_t;

void owf_reader_init(owf_reader_t *reader, owf_alloc_cb_t alloc_fn, owf_free_cb_t free_fn, owf_read_cb_t read, owf_visit_cb_t visitor, void *data);

#define OWF_READER_ERR(reader, fmt) do {snprintf((&(reader))->error, sizeof((&(reader))->error), (fmt));} while (0)
#define OWF_READER_ERRF(reader, fmt, ...) do {snprintf((&(reader))->error, sizeof((&(reader))->error), (fmt), __VA_ARGS__);} while (0)
#define OWF_READER_VISIT(reader, obj, type) \
    do { \
        if ((&(reader))->visit != NULL && !(&(reader))->visit(obj, type, (&(reader))->data)) { \
            /* Don't traverse this subtree, but return success */ \
            return true; \
        } \
    } while (0)

#endif /* OWF_READER_H */
