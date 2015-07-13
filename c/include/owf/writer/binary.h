#include <owf.h>
#include <owf/types.h>
#include <owf/arith.h>
#include <owf/writer.h>

#include <stdio.h>

#ifndef OWF_BINARY_WRITER_H
#define OWF_BINARY_WRITER_H

typedef struct owf_binary_writer {
    /** The writer */
    owf_writer_t writer;
} owf_binary_writer_t;

typedef bool (*owf_binary_writer_cb_t)(owf_binary_writer_t *, void *);

void owf_binary_writer_init(owf_binary_writer_t *binary, owf_alloc_t *alloc, owf_error_t *error, owf_write_cb_t write, void *data);
void owf_binary_writer_init_file(owf_binary_writer_t *binary, FILE *file, owf_alloc_t *alloc, owf_error_t *error);
void owf_binary_writer_init_buffer(owf_binary_writer_t *binary, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error);
const char *owf_binary_writer_strerror(owf_binary_writer_t *binary);

bool owf_binary_writer_write_double(owf_binary_writer_t *binary, double val);
bool owf_binary_writer_write_time(owf_binary_writer_t *binary, owf_time_t time);
bool owf_binary_writer_write_size(owf_binary_writer_t *binary, uint32_t length);
bool owf_binary_writer_write_u32(owf_binary_writer_t *binary, uint32_t u32);
bool owf_binary_writer_write_u16(owf_binary_writer_t *binary, uint16_t u16);
bool owf_binary_writer_write_u8(owf_binary_writer_t *binary, uint8_t u8);

bool owf_binary_writer_write_str(owf_binary_writer_t *binary, owf_str_t *str);
bool owf_binary_writer_write_samples(owf_binary_writer_t *binary, const double *ptr, uint32_t count);
bool owf_binary_writer_write_signal_header(owf_binary_writer_t *binary, owf_signal_t *signal);
bool owf_binary_writer_write_signal(owf_binary_writer_t *binary, owf_signal_t *signal);
bool owf_binary_writer_write_event_header(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event);
bool owf_binary_writer_write_event(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event);
bool owf_binary_writer_write_alarm_header(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm);
bool owf_binary_writer_write_alarm(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm);
bool owf_binary_writer_write_namespace_header(owf_binary_writer_t *binary, owf_namespace_t *ns, uint32_t size);
bool owf_binary_writer_write_namespace(owf_binary_writer_t *binary, owf_namespace_t *ns);
bool owf_binary_writer_write_channel_header(owf_binary_writer_t *binary, owf_channel_t *channel, uint32_t size);
bool owf_binary_writer_write_channel(owf_binary_writer_t *binary, owf_channel_t *channel);
bool owf_binary_write(owf_binary_writer_t *binary, owf_t *owf);
bool owf_binary_write_buffer(owf_binary_writer_t *binary, owf_t *owf, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error);

#endif /* OWF_BINARY_WRITER_H */