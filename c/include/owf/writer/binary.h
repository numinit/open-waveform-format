#include <owf.h>
#include <owf/types.h>
#include <owf/arith.h>
#include <owf/writer.h>

#include <stdio.h>

#ifndef OWF_BINARY_WRITER_H
#define OWF_BINARY_WRITER_H

/* The size of the lookaside buffer for byteswaps. */
#define OWF_BINARY_WRITER_BYTESWAP_BUFFER_LEN 32

/* A binary writer.
 *
 * Just stores the <owf_writer_t> context for now.
 */
typedef struct owf_binary_writer {
    /* The writer */
    owf_writer_t writer;
} owf_binary_writer_t;

/* A callback used internally by the binary writer. */
typedef bool (*owf_binary_writer_cb_t)(owf_binary_writer_t *, void *);

/* Initializes this binary writer.
 *
 * @binary The writer
 * @alloc The allocator
 * @error The error context
 * @write The write callback
 * @data User data supplied to the write callback
 */
void owf_binary_writer_init(owf_binary_writer_t *binary, owf_alloc_t *alloc, owf_error_t *error, owf_write_cb_t write, void *data);

/* Initializes this binary writer using a FILE pointer.
 *
 * @binary The writer
 * @file The file handle
 * @alloc The allocator
 * @error The error context
 */
void owf_binary_writer_init_file(owf_binary_writer_t *binary, FILE *file, owf_alloc_t *alloc, owf_error_t *error);

/* Initializes this binary writer using an <owf_buffer_t>.
 *
 * @binary The writer
 * @buf The buffer
 * @alloc The allocator
 * @error The error context
 */
void owf_binary_writer_init_buffer(owf_binary_writer_t *binary, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error);

/* Writes the header for an <owf_package_t> to an <owf_binary_writer_t>.
 *
 * @binary The binary writer
 * @owf The package
 * @size The total size
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_write_header(owf_binary_writer_t *binary, owf_package_t *owf, uint32_t size);

/* Writes an <owf_package_t> to an <owf_binary_writer_t>.
 *
 * @binary The binary writer
 * @owf The package
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_write(owf_binary_writer_t *binary, owf_package_t *owf);

/* Writes an <owf_package_t> to an <owf_binary_writer_t> configured to write to an <owf_buffer_t>.
 * When you are done with the buffer, call <owf_free> on buf->ptr.
 *
 * @binary The binary writer
 * @owf The package to write
 * @buf The buffer to write to
 * @alloc The allocator
 * @error The error context
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_write_buffer(owf_binary_writer_t *binary, owf_package_t *owf, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error);

/* Writes the header for an <owf_channel_t> to an <owf_binary_writer_t>.
 * This does not include the array of namespaces.
 *
 * @binary The binary writer
 * @channel The channel
 * @size The total channel size
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_channel_header(owf_binary_writer_t *binary, owf_channel_t *channel, uint32_t size);

/* Writes an <owf_channel_t> to an <owf_binary_writer_t>.
 * This includes the array of namespaces.
 *
 * @binary The binary writer
 * @channel The channel
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_channel(owf_binary_writer_t *binary, owf_channel_t *channel);

/* Writes the header for an <owf_namespace_t> to an <owf_binary_writer_t>.
 * This does not include the arrays of signals, events, and alarms.
 *
 * @binary The binary writer
 * @ns The namespace
 * @size The total namespace size
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_namespace_header(owf_binary_writer_t *binary, owf_namespace_t *ns, uint32_t size);

/* Writes an <owf_namespace_t> to an <owf_binary_writer_t>.
 * This includes the arrays of signals, events, and alarms.
 *
 * @binary The binary writer
 * @ns The namespace
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_namespace(owf_binary_writer_t *binary, owf_namespace_t *ns);

/* Writes the header for an <owf_signal_t> to an <owf_binary_writer_t>.
 *
 * The header does not include the sample array.
 *
 * @binary The binary writer
 * @signal The signal
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_signal_header(owf_binary_writer_t *binary, owf_signal_t *signal);

/* Writes an entire <owf_signal_t> to an <owf_binary_writer_t>.
 *
 * This includes the sample array.
 *
 * @binary The binary writer
 * @signal The signal
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_signal(owf_binary_writer_t *binary, owf_signal_t *signal);

/* Writes the header for an <owf_event_t> to an <owf_binary_writer_t>.
 *
 * @binary The binary writer
 * @ns The enclosing namespace
 * @event The event
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_event_header(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event);

/* Writes an <owf_event_t> to an <owf_binary_writer_t>.
 * This is the same as <owf_binary_writer_write_event_header>, but is provided for completeness.
 * @binary The binary writer
 * @ns The enclosing namespace
 * @event The event
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_event(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event);

/* Writes the header for an <owf_alarm_t> to an <owf_binary_writer_t>.
 *
 * @binary The binary writer
 * @ns The enclosing namespace
 * @alarm The alarm
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_alarm_header(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm);

/* Writes an <owf_alarm_t> to an <owf_binary_writer_t>.
 * This is the same as <owf_binary_writer_write_alarm_header>, but is provided for completeness.
 * @binary The binary writer
 * @ns The enclosing namespace
 * @event The alarm
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_alarm(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm);

/* Writes samples to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @ptr A pointer to samples
 * @count The number of samples
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_samples(owf_binary_writer_t *binary, const double *ptr, uint32_t count);

/* Writes a string to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @str The string
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_str(owf_binary_writer_t *binary, owf_str_t *str);

/* Writes a double to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @val The double to write
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_double(owf_binary_writer_t *binary, double val);

/* Writes a time to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @time The time
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_time(owf_binary_writer_t *binary, owf_time_t time);

/* Writes a size to the <owf_binary_writer_t>, checking for 4-byte alignment.
 *
 * @binary The writer
 * @size The size
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_size(owf_binary_writer_t *binary, uint32_t size);

/* Writes a 32-bit unsigned integer to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @u32 The integer
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_u32(owf_binary_writer_t *binary, uint32_t u32);

/* Writes a 16-bit unsigned integer to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @u16 The integer
 * 
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_u16(owf_binary_writer_t *binary, uint16_t u16);

/* Writes an 8-bit unsigned integer to the <owf_binary_writer_t>.
 *
 * @binary The writer
 * @u8 The integer
 *
 * @return True if the write was successful, false otherwise
 */
bool owf_binary_writer_write_u8(owf_binary_writer_t *binary, uint8_t u8);

#endif /* OWF_BINARY_WRITER_H */
