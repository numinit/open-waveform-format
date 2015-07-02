#include <owf.h>
#include <owf/platform.h>
#include <owf/types.h>
#include <owf/arith.h>
#include <owf/reader.h>

#include <stdio.h>

#ifndef OWF_BINARY_H
#define OWF_BINARY_H

#define OWF_MAGIC 0x4f574631UL
#define OWF_BINARY_READER_SKIP_BUF_SIZE 256

typedef struct owf_binary_reader {
    owf_reader_t reader;
    char skip[OWF_BINARY_READER_SKIP_BUF_SIZE];
    uint32_t segment_length, skip_length;
} owf_binary_reader_t;

typedef bool (*owf_binary_reader_cb_t)(owf_binary_reader_t *, void *);

void owf_binary_reader_init(owf_binary_reader_t *binary, owf_alloc_t *alloc, owf_reader_read_cb_t read, owf_reader_visit_cb_t visitor, void *data);
void owf_binary_reader_init_file(owf_binary_reader_t *binary, FILE *file, owf_alloc_t *alloc, owf_reader_visit_cb_t visitor);
void owf_binary_reader_init_buffer(owf_binary_reader_t *binary, owf_buffer_t *bu, owf_alloc_t *alloc, owf_reader_visit_cb_t visitor);

const char *owf_binary_reader_strerror(owf_binary_reader_t *binary);

bool owf_binary_reader_unwrap(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, void *ptr);
bool owf_binary_reader_unwrap_top(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, uint32_t *length_ptr, void *ptr);
bool owf_binary_reader_unwrap_nested_multi(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb);
bool owf_binary_reader_unwrap_multi(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb);
bool owf_binary_reader_read_str(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_samples(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_signal(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_signals(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_event(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_events(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_alarm(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_alarms(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_namespace(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_channel(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_reader_read_channels(owf_binary_reader_t *binary, void *ptr);
bool owf_binary_read(owf_binary_reader_t *binary);
owf_t *owf_binary_materialize(owf_binary_reader_t *binary);

#endif /* OWF_BINARY_H */
