#include <owf.h>
#include <owf/alloc.h>
#include <owf/arith.h>

#include <string.h>

#ifndef OWF_TYPES_H
#define OWF_TYPES_H

/* An OWF timestamp. */
typedef int64_t owf_time_t;

/* An OWF duration. */
typedef uint64_t owf_duration_t;

/* A union between a double and a uint64_t.
 *
 * Used to protect strict aliasing.
 */
typedef union owf_double_union owf_double_union_t;

/* A buffer reader or writer.
 *
 * Essentially a stripped-down FILE struct. Contains a pointer,
 * a length, and a position.
 */
typedef struct owf_buffer owf_buffer_t;

/* A simple struct used in memoization.
 *
 * Used to cache lengths to avoid recomputation.
 */
typedef struct owf_memoize owf_memoize_t;

/* An OWF array.
 *
 * Contains a pointer, number of elements, and total capacity.
 */
typedef struct owf_array owf_array_t;

/* An OWF string.
 *
 * OWF strings are UTF-8 null-terminated strings.
 * libowf does not care about UTF-8 strings, but will provide a null-terminated
 * byte array.
 */
typedef struct owf_str owf_str_t;

/* An OWF package.
 *
 * Packages contain the set of channels making up the OWF packet.
 */
typedef struct owf_package owf_package_t;

/* An OWF channel.
 *
 * A channel contains devices broadcasting on a particular data source.
 */
typedef struct owf_channel owf_channel_t;

/* An OWF namespace.
 *
 * A namespace represents a collection of signals, events, and alarms from a device on a channel.
 */
typedef struct owf_namespace owf_namespace_t;

/* An OWF signal.
 *
 * A signal is a collection of measurements from a sensor on a device.
 */
typedef struct owf_signal owf_signal_t;

/* An OWF event.
 *
 * An event is an object representing an instant in time that something happens,
 * as opposed to an ongoing alarm.
 */
typedef struct owf_event owf_event_t;

/* An OWF alarm.
 *
 * An alarm represents an ongoing device notification measured at a particular point in time.
 */
typedef struct owf_alarm owf_alarm_t;

/* @see owf_double_union_t */
union owf_double_union {
    /* The double value */
    double f64;

    /* The floating-point value */
    uint64_t u64;
};

/* @see owf_buffer_t */
struct owf_buffer {
    /* The pointer to the buffer */
    void *ptr;

    /* The number of bytes in this buffer */
    size_t length;

    /* The current position in the buffer */
    size_t position;
};

/* Initializes an <owf_buffer_t> of size `size` to point to `ptr`.
 * @buf The buffer struct
 * @ptr The pointer
 * @size The size
 */
void owf_buffer_init(owf_buffer_t *buf, void *ptr, size_t size);

/* @see owf_memoize_t */
struct owf_memoize {
    /* The memoized length */
    uint32_t length;
};

/* Initializes a stale <owf_memoize_t>.
 * @memoize The uninitialized <owf_memoize_t> to initialize.
 */
void owf_memoize_init(owf_memoize_t *memoize);

/* Returns whether the <owf_memoize_t> provided is stale.
 * @memoize The <owf_memoize_t>
 *
 * @return True if stale, false otherwise
 */
bool owf_memoize_stale(owf_memoize_t *memoize);

/* Fetches the value from the provided <owf_memoize_t>.
 * @memoize The <owf_memoize_t>
 *
 * @return The value
 */
uint32_t owf_memoize_fetch(owf_memoize_t *memoize);

/* Caches a value in the <owf_memoize_t>, replacing the existing value.
 * @memoize The <owf_memoize_t>
 * @value The value to memoize
 *
 * @return The newly memoized value
 */
uint32_t owf_memoize_cache(owf_memoize_t *memoize, uint32_t value);

/* @see owf_array_t */
struct owf_array {
    /* The pointer to the first element */
    void *ptr;

    /* The length of this array */
    uint32_t length;

    /* The capacity of this array */
    uint32_t capacity;
};

/* Initializes this <owf_array_t> to be empty.
 * @arr The array
 * Empty arrays take up no heap memory.
 */
void owf_array_init(owf_array_t *arr);

/* Destroys this <owf_array_t>.
 * @arr The array
 * @alloc The allocator
 */
void owf_array_destroy(owf_array_t *arr, owf_alloc_t *alloc);

/* Performs a bytewise comparison of two <owf_array_t> pointers.
 * @lhs The left hand array
 * @rhs The right hand array
 * @width The element width
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_array_binary_compare(owf_array_t *lhs, owf_array_t *rhs, uint32_t width);

/* Reserves space in `arr` to fit at least `capacity` elements.
 * @arr The array
 * @alloc The allocator
 * @error The error context
 * @capacity The new capacity
 * @width The element width
 *
 * @return True if the operation was successful. Sets `error` if unsuccessful.
 */
bool owf_array_reserve(owf_array_t *arr, owf_alloc_t *alloc, owf_error_t *error, uint32_t capacity, uint32_t width);

/* Reserves space in `arr` to fit exactly `capacity` elements.
 * @arr The array
 * @alloc The allocator
 * @error The error context
 * @capacity The new capacity
 * @width The element width
 *
 * @return True if the operation was successful. Sets `error` if unsuccessful.
 */
bool owf_array_reserve_exactly(owf_array_t *arr, owf_alloc_t *alloc, owf_error_t *error, uint32_t capacity, uint32_t width);

/* Pushes the object `obj` onto the end of the array.
 * @arr The array
 * @alloc The allocator
 * @error The error context
 * @obj The object
 * @width The object width
 *
 * @return True if the operation was successful. Sets `error` if unsuccessful.
 */
bool owf_array_push(owf_array_t *arr, owf_alloc_t *alloc, owf_error_t *error, const void *obj, uint32_t width);

/* Puts the object `obj` at index `idx`
 * @arr The array
 * @error The error context
 * @obj The object
 * @idx The index
 * @width The object width
 *
 * @return True if the operation was successful. Sets `error` if unsuccessful.
 */
bool owf_array_put(owf_array_t *arr, owf_error_t *error, const void *obj, uint32_t idx, uint32_t width);

/* Gets the object `obj` at index `idx`
 * @arr The array
 * @error The error context
 * @idx The index
 * @width The object width
 *
 * @return A pointer to the object, or NULL if the index is out-of-bounds
 */
void *owf_array_get(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width);

/* Calculates a pointer to an element at a particular index in an array.
 * @arr The array
 * @error The error context
 * @idx The index
 * @width The object width
 *
 * @return A pointer to the index, or NULL if the index is out of bounds
 */
void *owf_array_ptr_for(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width);

/* Does a semantic comparison of two arrays.
 * @_lhs The left-hand array
 * @_rhs The right-hand array
 * @_type The type of both arrays
 * @_element_fn The function to call for each element
 */
#define OWF_ARRAY_SEMANTIC_COMPARE(_lhs, _rhs, _type, _element_fn) \
    do { \
        uint32_t __lhs_len = OWF_ARRAY_LEN(_lhs), __rhs_len = OWF_ARRAY_LEN(_rhs); \
        if (__lhs_len == __rhs_len) { \
            for (uint32_t __i = 0; __i < __lhs_len; __i++) { \
                _type *__lv = OWF_ARRAY_PTR(_lhs, _type, __i), *__rv = OWF_ARRAY_PTR(_rhs, _type, __i); \
                int __ret = _element_fn(__lv, __rv); \
                if (__ret != 0) { \
                    return __ret; \
                } \
            } \
        } else { \
            return __lhs_len < __rhs_len ? -1 : 1; \
        } \
    } while (0)

/* Returns a typed pointer for an array.
 * @_arr The array
 * @_type The type
 */
#define OWF_ARRAY_TYPED_PTR(_arr, _type) ((_type *)((&(_arr))->ptr))

/* Returns a typed pointer at an index for an array.
 * @_arr The array
 * @_type The type
 * @_idx The index
 */
#define OWF_ARRAY_PTR(_arr, _type, _idx) (&OWF_ARRAY_TYPED_PTR(_arr, _type)[_idx])

/* Returns an object at an index for an array.
 * @_arr The array
 * @_type The type
 * @_idx The index
 */
#define OWF_ARRAY_GET(_arr, _type, _idx) (OWF_ARRAY_TYPED_PTR(_arr, _type)[_idx])

/* Puts an object at an index for an array.
 * @_arr The array
 * @_type The type
 * @_idx The index
 * @_value The value
 */
#define OWF_ARRAY_PUT(_arr, _type, _idx, _value) do {OWF_ARRAY_GET(_arr, _type, _idx) = _value;} while (0)

/* Returns the length for an array.
 * @_arr The array
 */
#define OWF_ARRAY_LEN(_arr) ((&(_arr))->length)

/* @see owf_str_t */
struct owf_str {
    /* Memoization for the string's size and the total size in bytes */
    owf_memoize_t string_size, total_size;

    /* The byte array */
    owf_array_t bytes;
};

/* Initializes a string to be empty.
 * @str The string to initialize
 * Like arrays, empty strings take up no heap memory.
 */
void owf_str_init(owf_str_t *str);

/* Sets an <owf_str_t> to `value`.
 * @str The string
 * @alloc The allocator
 * @error The error context
 * @value The value to copy
 * If `str` is longer than 0 bytes, the null-terminated value will be copied into `str`.
 * Otherwise, no heap allocations will be made.
 *
 * @return True if the operation was successful
 */
bool owf_str_set(owf_str_t *str, owf_alloc_t *alloc, owf_error_t *error, const char *value);

/* Reserves `length` bytes for a string.
 * @str The string
 * @alloc The allocator
 * @error The error context
 * @length The length to reserve
 *
 * @return True if the operation was successful
 */
bool owf_str_reserve(owf_str_t *str, owf_alloc_t *alloc, owf_error_t *error, uint32_t length);

/* Destroys an <owf_str_t>.
 * @str The string
 * @alloc The allocator
 */
void owf_str_destroy(owf_str_t *str, owf_alloc_t *alloc);

/* Lexicographically compares two strings.
 * @lhs The left hand string
 * @rhs The right hand string
 * Note that this comparison function does not take UTF-8 lexicographical ordering
 * into account. It is only a byte comparison
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_str_binary_compare(owf_str_t *lhs, owf_str_t *rhs);

/* Returns the length for an <owf_str_t>.
 * @str The string
 *
 * @return The length
 */
uint32_t owf_str_length(owf_str_t *str);

/* Computes the total size in bytes of an <owf_str_t>.
 * @str The string
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return Whether the size calculation was successful
 */
bool owf_str_size(owf_str_t *str, owf_error_t *error, uint32_t *output_size);

/* Returns a pointer to the underlying NULL-terminated string. */
#define OWF_STR_PTR(_str) ((const char *)((&(_str))->bytes.ptr))

/* @see owf_package_t */
struct owf_package {
    /* Memoization for the total size in bytes */
    owf_memoize_t memoize;

    /* An array of channels */
    owf_array_t channels;
};

/* Initializes an <owf_package_t>.
 * @owf The <owf_package_t> to initialize
 */
void owf_package_init(owf_package_t *owf);

/* Destroys an <owf_package_t> recursively.
 * @owf The <owf_package_t> to destroy
 * @alloc The allocator
 */
void owf_package_destroy(owf_package_t *owf, owf_alloc_t *alloc);

/* Compares two <owf_package_t> instances
 * @lhs The left hand package
 * @rhs The right hand package
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_package_compare(owf_package_t *lhs, owf_package_t *rhs);

/* Computes the total size in bytes of an <owf_package_t>.
 * @owf The package
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return Whether the size calculation was successful
 */
bool owf_package_size(owf_package_t *owf, owf_error_t *error, uint32_t *output_size);

/* Pushes an <owf_channel_t> onto an <owf_package_t>.
 * @owf The package
 * @alloc The allocator
 * @error The error context
 * @channel The <owf_channel_t>
 *
 * @return True if the operation was successful
 */
bool owf_package_push_channel(owf_package_t *owf, owf_alloc_t *alloc, owf_error_t *error, owf_channel_t *channel);

/* @see owf_channel_t */
struct owf_channel {
    /* Memoization for the total size in bytes */
    owf_memoize_t memoize;

    /* The channel ID */
    owf_str_t id;

    /* An array of namespaces */
    owf_array_t namespaces;
};

/* Initializes an <owf_channel_t>.
 * @channel The channel to initialize
 */
void owf_channel_init(owf_channel_t *channel);

/* Initializes an <owf_channel_t> with the specified ID.
 * @channel The channel to initialize
 * @alloc The allocator
 * @error The error context
 * @id The ID of the channel
 *
 * @return True if the operation was successful
 */
bool owf_channel_init_id(owf_channel_t *channel, owf_alloc_t *alloc, owf_error_t *error, const char *id);

/* Destroys an <owf_channel_t>.
 * @channel The channel to destroy
 * @alloc The allocator
 */
void owf_channel_destroy(owf_channel_t *channel, owf_alloc_t *alloc);

/* Writes a description of a channel to a FILE pointer.
 * @channel The channel
 * @fp The file to write it to
 *
 * @return The number of bytes written
 */
int owf_channel_print(owf_channel_t *channel, FILE *fp);

/* Writes a description of a channel to a string.
 * @channel The channel
 * @ptr The string pointer
 * @size The length of the buffer
 *
 * @return The number of bytes written
 */
int owf_channel_stringify(owf_channel_t *channel, char *ptr, size_t size);

/* Compares two <owf_channel_t> instances
 * @lhs The left hand channel
 * @rhs The right hand channel
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_channel_compare(owf_channel_t *lhs, owf_channel_t *rhs);

/* Computes the total size in bytes of an <owf_channel_t>.
 * @channel The channel
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return Whether the size calculation was successful
 */
bool owf_channel_size(owf_channel_t *channel, owf_error_t *error, uint32_t *output_size);

/* Sets the ID of an <owf_channel_t> by copying `id`.
 * @channel The channel
 * @alloc The allocator
 * @error The error context
 * @id The ID
 *
 * @return True if the operation was successful
 */
bool owf_channel_set_id(owf_channel_t *channel, owf_alloc_t *alloc, owf_error_t *error, const char *id);

/* Pushes a namespace onto this <owf_channel_t>.
 * @channel The channel
 * @alloc The allocator
 * @error The error context
 * @ns The namespace
 *
 * @return True if the operation was successful
 */
bool owf_channel_push_namespace(owf_channel_t *channel, owf_alloc_t *alloc, owf_error_t *error, owf_namespace_t *ns);

/* @see owf_namespace_t */
struct owf_namespace {
    /* Memoization for the total size in bytes */
    owf_memoize_t memoize;

    /* The namespace ID */
    owf_str_t id;

    /* The namespace timestamp */
    owf_time_t t0;
    
    /* The duration */
    owf_duration_t dt;

    /* Arrays of signals, events, and alarms */
    owf_array_t signals, events, alarms;
};

/* Initializes this <owf_namespace_t>.
 * @ns The namespace to initialize
 */
void owf_namespace_init(owf_namespace_t *ns);

/* Initializes this <owf_namespace_t> with an id.
 * @ns The namespace to initialize
 * @alloc The allocator
 * @error The error context
 * @id The ID of the namespace
 *
 * @return True if the operation was successful
 */
bool owf_namespace_init_id(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, const char *id);

/* Destroys this <owf_namespace_t>.
 * @ns The namespace to destroy
 * @alloc The allocator
 */
void owf_namespace_destroy(owf_namespace_t *ns, owf_alloc_t *alloc);

/* Writes a description of a namespace to a FILE pointer.
 * @ns The namespace
 * @fp The file to write it to
 *
 * @return The number of bytes written
 */
int owf_namespace_print(owf_namespace_t *ns, FILE *fp);

/* Writes a description of a namespace to a string.
 * @ns The namespace
 * @ptr The string pointer
 * @size The length of the buffer
 *
 * @return The number of bytes written
 */
int owf_namespace_stringify(owf_namespace_t *ns, char *ptr, size_t size);

/* Compares two <owf_namespace_t> instances.
 * @lhs The left hand namespace
 * @rhs The right hand namespace
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_namespace_compare(owf_namespace_t *lhs, owf_namespace_t *rhs);

/* Returns whether this namespace covers a timestamp.
 * @ns The namespace
 * @timestamp The timestamp
 * That is, whether ns->t0 <= timestamp < ns->t0 + ns->dt.
 *
 * @return True if the timestamp is in range
 */
bool owf_namespace_covers(owf_namespace_t *ns, owf_time_t timestamp);

/* Computes the total size in bytes of an <owf_namespace_t>.
 * @ns The namespace
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return True if the size calculation was successful
 */
bool owf_namespace_size(owf_namespace_t *ns, owf_error_t *error, uint32_t *output_size);

/* Sets the ID of an <owf_namespace_t> by copying `id`.
 * @ns The namespace
 * @alloc The allocator
 * @error The error context
 * @id The ID
 *
 * @return True if the operation was successful
 */
bool owf_namespace_set_id(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, const char *id);

/* Pushes a signal onto this <owf_namespace_t>.
 * @ns The namespace
 * @alloc The allocator
 * @error The error context
 * @signal The signal
 *
 * @return True if the operation was successful
 */
bool owf_namespace_push_signal(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, owf_signal_t *signal);

/* Pushes an event onto this <owf_namespace_t>.
 * @ns The namespace
 * @alloc The allocator
 * @error The error context
 * @event The event
 *
 * @return True if the operation was successful
 */
bool owf_namespace_push_event(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, owf_event_t *event);

/* Pushes an alarm onto this <owf_namespace_t>.
 * @ns The namespace
 * @alloc The allocator
 * @error The error context
 * @alarm The alarm
 *
 * @return True if the operation was successful
 */
bool owf_namespace_push_alarm(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, owf_alarm_t *alarm);

/* @see owf_signal_t */
struct owf_signal {
    /* Memoization for the total size in bytes */
    owf_memoize_t memoize;

    /* The signal ID */
    owf_str_t id;

    /* The signal unit */
    owf_str_t unit;

    /* An array of samples */
    owf_array_t samples;
};

/* Initializes this <owf_signal_t>.
 * @signal The signal
 */
void owf_signal_init(owf_signal_t *signal);

/* Initializes this <owf_signal_t> with an id and unit.
 * @signal The signal
 * @alloc The allocator
 * @error The error context
 * @id The signal ID
 * @unit The signal units
 *
 * @return True if the operation was successful
 */
bool owf_signal_init_id_unit(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const char *id, const char *unit);

/* Destroys an <owf_signal_t>.
 * @signal The signal
 * @alloc The allocator
 */
void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *alloc);

/* Writes a description of a signal to a FILE pointer
 * @signal The signal
 * @fp The file to write it to
 *
 * @return The number of bytes written
 */
int owf_signal_print(owf_signal_t *signal, FILE *fp);

/* Writes a description of a signal to a string.
 * @signal The signal
 * @ptr The string pointer
 * @size The length of the buffer
 *
 * @return The number of bytes written
 */
int owf_signal_stringify(owf_signal_t *signal, char *ptr, size_t size);

/* Compares two <owf_signal_t> instances.
 * @lhs The left hand signal
 * @rhs The right hand signal
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_signal_compare(owf_signal_t *lhs, owf_signal_t *rhs);

/* Computes the total size in bytes of an <owf_signal_t>.
 * @signal The signal
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return True if the size calculation was successful
 */
bool owf_signal_size(owf_signal_t *signal, owf_error_t *error, uint32_t *output_size);

/* Sets the ID of an <owf_signal_t> by copying `id`.
 * @signal The signal
 * @alloc The allocator
 * @error The error context
 * @id The ID
 *
 * @return True if the operation was successful
 */
bool owf_signal_set_id(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const char *id);

/* Sets the unit of an <owf_signal_t> by copying `unit`.
 * @signal The signal
 * @alloc The allocator
 * @error The error context
 * @unit The units
 *
 * @return True if the operation was successful
 */
bool owf_signal_set_unit(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const char *unit);

/* Copies samples into this owf_signal_t, appending them to the array of samples.
 * @signal The signal
 * @alloc The allocator
 * @error The error context
 * @samples The sample array
 * @count The number of samples to push
 */
bool owf_signal_push_samples(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const double *samples, uint32_t count);

/* @see owf_event_t */
struct owf_event {
    /* Memoization for the total size in bytes */
    owf_memoize_t memoize;

    /* The timestamp */
    owf_time_t t0;

    /* The message */
    owf_str_t message;
};

/* Initializes this <owf_event_t>.
 * @event The event
 */
void owf_event_init(owf_event_t *event);

/* Destroys this <owf_event_t>.
 * @event The event
 * @alloc The allocator
 */
void owf_event_destroy(owf_event_t *event, owf_alloc_t *alloc);

/* Writes a description of an event to a FILE pointer
 * @event The event
 * @fp The file to write it to
 *
 * @return The number of bytes written
 */
int owf_event_print(owf_event_t *event, FILE *fp);

/* Writes a description of an event to a string.
 * @event The event
 * @ptr The string pointer
 * @size The length of the buffer
 *
 * @return The number of bytes written
 */
int owf_event_stringify(owf_event_t *event, char *ptr, size_t size);

/* Compares two <owf_event_t> instances
 * @lhs The left hand event
 * @rhs The right hand event
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_event_compare(owf_event_t *lhs, owf_event_t *rhs);

/* Computes the total size in bytes of an <owf_event_t>.
 * @event The event
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return True if the size calculation was successful
 */
bool owf_event_size(owf_event_t *event, owf_error_t *error, uint32_t *output_size);

/* @see owf_alarm_t */
struct owf_alarm {
    /* Memoization for the total size in bytes */
    owf_memoize_t memoize;

    /* The measurement timestamp */
    owf_time_t t0;
    
    /* The total time this alarm has been sounding */
    owf_duration_t dt;

    /* Union between a 32-bit int and the level and volume */
    union {
        /* 8-bit values */
        struct {
            /* Level and volume */
            uint8_t level, volume, _reserved_0, _reserved_1;
        } u8;

        /* 32-bit value */
        uint32_t u32;
    } details;

    /* The type of the alarm */
    owf_str_t type;

    /* The alarm's message */
    owf_str_t message;
};

/* Initializes this <owf_alarm_t>
 * @alarm The alarm
 */
void owf_alarm_init(owf_alarm_t *alarm);

/* Destroys this <owf_alarm_t>
 * @alarm The alarm
 * @alloc The allocator
 */
void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *alloc);

/* Writes a description of an alarm to a FILE pointer
 * @alarm The alarm
 * @fp The file to write it to
 *
 * @return The number of bytes written
 */
int owf_alarm_print(owf_alarm_t *alarm, FILE *fp);

/* Writes a description of an alarm to a string.
 * @alarm The alarm
 * @ptr The string pointer
 * @size The length of the buffer
 *
 * @return The number of bytes written
 */
int owf_alarm_stringify(owf_alarm_t *ns, char *ptr, size_t size);

/* Compares two <owf_alarm_t> instances
 * @lhs The left hand alarm
 * @rhs The right hand alarm
 *
 * @return < 0 if lhs < rhs; = 0 if lhs == rhs; > 0 if lhs > rhs
 */
int owf_alarm_compare(owf_alarm_t *lhs, owf_alarm_t *rhs);

/* Computes the total size in bytes of an <owf_alarm_t>.
 * @alarm The alarm
 * @error The error context
 * @output_size A pointer to a uint32_t to store the size in
 *
 * @return True if the size calculation was successful
 */
bool owf_alarm_size(owf_alarm_t *alarm, owf_error_t *error, uint32_t *output_size);

#endif /* OWF_TYPES_H */
