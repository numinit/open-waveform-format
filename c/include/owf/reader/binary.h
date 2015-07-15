#include <owf.h>
#include <owf/types.h>
#include <owf/arith.h>
#include <owf/reader.h>

#include <stdio.h>

#ifndef OWF_BINARY_READER_H
#define OWF_BINARY_READER_H

/* The size of the skip buffer. */
#define OWF_BINARY_READER_SKIP_BUF_SIZE 256

/* A binary reader.
 *
 * Stores the <owf_reader_t> context, a skip buffer for scratch space,
 * and length markers.
 */
typedef struct owf_binary_reader {
    /* The reader */
    owf_reader_t reader;

    /* A skip buffer */
    char skip[OWF_BINARY_READER_SKIP_BUF_SIZE];

    /* Internal segment and skip length accounting variables */
    uint32_t segment_length, skip_length;
} owf_binary_reader_t;

/* A callback used internally by the binary reader. */
typedef bool (*owf_binary_reader_cb_t)(owf_binary_reader_t *, void *);

/* Initializes this binary reader.
 *
 * @binary The reader
 * @alloc The allocator
 * @error The error context
 * @read The read callback
 * @visitor The visit callback
 * @data User data supplied to both the read callback and the visit callback
 */
void owf_binary_reader_init(owf_binary_reader_t *binary, owf_alloc_t *alloc, owf_error_t *error, owf_read_cb_t read, owf_visit_cb_t visitor, void *data);

/* Initializes this binary reader using a FILE pointer.
 *
 * @binary The reader
 * @file The file handle
 * @alloc The allocator
 * @error The error context
 * @visitor The visit callback
 */
void owf_binary_reader_init_file(owf_binary_reader_t *binary, FILE *file, owf_alloc_t *alloc, owf_error_t *error, owf_visit_cb_t visitor);

/* Initializes this binary reader using an <owf_buffer_t>.
 *
 * @binary The reader
 * @buf The buffer
 * @alloc The allocator
 * @error The error context
 * @visitor The visit callback
 */
void owf_binary_reader_init_buffer(owf_binary_reader_t *binary, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error, owf_visit_cb_t visitor);

/* Reads the entire <owf_binary_reader_t>, invoking visitor callbacks for each node.
 *
 * @binary The reader
 *
 * @return Whether the read was successful
 */
bool owf_binary_read(owf_binary_reader_t *binary);

/* Materializes an entire OWF packet to an <owf_package_t>.
 *
 * @binary The reader
 *
 * @return A pointer to an <owf_package_t> if successful, NULL otherwise.
 *         When you are finished, use <owf_package_destroy> to free the returned
 *         package.
 */
owf_package_t *owf_binary_materialize(owf_binary_reader_t *binary);

/* Reads a channel from the <owf_binary_reader_t> into an <owf_channel_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_channel_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_channel(owf_binary_reader_t *binary, void *ptr);

/* Reads multiple channels from the <owf_binary_reader_t> into an <owf_channel_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_channel_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_channels(owf_binary_reader_t *binary, void *ptr);

/* Reads a namespace from the <owf_binary_reader_t> into an <owf_namespace_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_namespace_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_namespace(owf_binary_reader_t *binary, void *ptr);

/* Reads a signal from the <owf_binary_reader_t> into an <owf_signal_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_signal_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_signal(owf_binary_reader_t *binary, void *ptr);

/* Reads multiple signals from the <owf_binary_reader_t> into an <owf_signal_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_signal_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_signals(owf_binary_reader_t *binary, void *ptr);

/* Reads an event from the <owf_binary_reader_t> into an <owf_event_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_event_t>.
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_event(owf_binary_reader_t *binary, void *ptr);

/* Reads multiple events from the <owf_binary_reader_t> into an <owf_event_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_event_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_events(owf_binary_reader_t *binary, void *ptr);

/* Reads an alarm from the <owf_binary_reader_t> into an <owf_alarm_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_alarm_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_alarm(owf_binary_reader_t *binary, void *ptr);

/* Reads multiple alarms from the <owf_binary_reader_t> into an <owf_event_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_event_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_alarms(owf_binary_reader_t *binary, void *ptr);

/* Reads samples from the <owf_binary_reader_t> into an <owf_array_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_array_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_samples(owf_binary_reader_t *binary, void *ptr);

/* Reads an <owf_str_t>.
 *
 * @binary The reader
 * @ptr A pointer to an <owf_str_t>
 *
 * @return Whether the read was successful
 */
bool owf_binary_reader_read_str(owf_binary_reader_t *binary, void *ptr);

/* Unwraps a length from the current reader state, calling <owf_binary_reader_unwrap_top>
 * and modifying the current segment length.
 *
 * @binary The reader
 * @cb The callback
 * @ptr Data to pass to the callback
 *
 * @return Whether the unwrap was successful
 */
bool owf_binary_reader_unwrap(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, void *ptr);

/* Performs the actual unwrap operation: reads the length heade, verifies alignment,
 * calls `cb`, reads skipped bytes, checks for trailing bytes, and writes the total length read
 * to `length_ptr`.
 *
 * @binary The reader
 * @cb The callback
 * @length_ptr A pointer to a uint32_t to store the length
 * @ptr Data to pass to the callback
 *
 * @return Whether the unwrap was successful
 */
bool owf_binary_reader_unwrap_top(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, uint32_t *length_ptr, void *ptr);

/* Unwraps multiple objects prefixed with a length.
 *
 * @binary The reader
 * @cb The callback
 * @ptr Data to pass to the callback
 *
 * @return Whether the unwrap was successful
 */
bool owf_binary_reader_unwrap_nested_multi(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, void *ptr);

/* Unwraps multiple objects that aren't prefixed with lengths.
 *
 * @binary The reader
 * @cb The callback
 * @ptr Data to pass to the callback
 * 
 * @return Whether the unwrap was successful
 */
bool owf_binary_reader_unwrap_multi(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, void *ptr);

#endif /* OWF_BINARY_READER_H */
