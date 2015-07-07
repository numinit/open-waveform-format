#include <owf.h>
#include <owf/alloc.h>
#include <owf/arith.h>

#ifndef OWF_TYPES_H
#define OWF_TYPES_H

typedef int64_t owf_time_t;

typedef union owf_double_union {
    double f64;
    uint64_t u64;
} owf_double_union_t;

typedef struct owf_buffer {
    void *ptr;
    size_t length, position;
} owf_buffer_t;

typedef struct owf_array {
    void *ptr;
    uint32_t length, capacity;
} owf_array_t;

typedef struct owf_memoize {
    uint32_t length;
} owf_memoize_t;

typedef struct owf_str {
    owf_memoize_t string_size, total_size;
    owf_array_t bytes;
} owf_str_t;

typedef struct owf_signal {
    owf_memoize_t memoize;
    owf_str_t id;
    owf_str_t unit;
    owf_array_t samples;
} owf_signal_t;

typedef struct owf_event {
    owf_memoize_t memoize;
    owf_time_t t0;
    owf_str_t message;
} owf_event_t;

typedef struct owf_alarm {
    owf_memoize_t memoize;
    owf_time_t t0, dt;
    union {
        uint8_t  level, volume, _reserved_0, _reserved_1;
        uint32_t u32;
    } details;
    owf_str_t type, message;
} owf_alarm_t;

typedef struct owf_namespace {
    owf_memoize_t memoize;
    owf_str_t id;
    owf_time_t t0, dt;
    owf_array_t signals, events, alarms;
} owf_namespace_t;

typedef struct owf_channel {
    owf_memoize_t memoize;
    owf_str_t id;
    owf_array_t namespaces;
} owf_channel_t;

typedef struct owf {
    owf_memoize_t memoize;
    owf_array_t channels;
} owf_t;

void owf_init(owf_t *owf);
void owf_destroy(owf_t *owf, owf_alloc_t *allocator);
int owf_compare(owf_t *lhs, owf_t *rhs);
bool owf_size(owf_t *owf, owf_error_t *error, uint32_t *output_size);

void owf_channel_init(owf_channel_t *channel);
int owf_channel_compare(owf_channel_t *lhs, owf_channel_t *rhs);
void owf_channel_destroy(owf_channel_t *channel, owf_alloc_t *allocator);
bool owf_channel_size(owf_channel_t *channel, owf_error_t *error, uint32_t *output_size);

void owf_namespace_init(owf_namespace_t *ns);
void owf_namespace_destroy(owf_namespace_t *ns, owf_alloc_t *allocator);
bool owf_namespace_covers(owf_namespace_t *ns, owf_time_t timestamp);
bool owf_namespace_size(owf_namespace_t *ns, owf_error_t *error, uint32_t *output_size);

void owf_signal_init(owf_signal_t *signal);
void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *allocator);
bool owf_signal_size(owf_signal_t *signal, owf_error_t *error, uint32_t *output_size);

void owf_event_init(owf_event_t *event);
void owf_event_destroy(owf_event_t *event, owf_alloc_t *allocator);
bool owf_event_size(owf_event_t *event, owf_error_t *error, uint32_t *output_size);

void owf_alarm_init(owf_alarm_t *alarm);
void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *allocator);
bool owf_alarm_size(owf_alarm_t *alarm, owf_error_t *error, uint32_t *output_size);

void owf_str_init(owf_str_t *str);
bool owf_str_reserve(owf_str_t *str, owf_alloc_t *allocator, owf_error_t *error, uint32_t length);
void owf_str_destroy(owf_str_t *str, owf_alloc_t *allocator);
uint32_t owf_str_length(owf_str_t *str);
uint32_t owf_str_padding(uint32_t length);
bool owf_str_size(owf_str_t *str, owf_error_t *error, uint32_t *output_size);

#define OWF_STR_PTR(str) ((const char *)((&(str))->bytes.ptr))

void owf_memoize_init(owf_memoize_t *memoize);
bool owf_memoize_stale(owf_memoize_t *memoize);
uint32_t owf_memoize_fetch(owf_memoize_t *memoize);
uint32_t owf_memoize_cache(owf_memoize_t *memoize, uint32_t value);

void owf_array_init(owf_array_t *arr);
void owf_array_destroy(owf_array_t *arr, owf_alloc_t *allocator);
bool owf_array_reserve(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width);
bool owf_array_reserve_exactly(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width);
bool owf_array_push(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, void *obj, uint32_t width);
bool owf_array_put(owf_array_t *arr, owf_error_t *error, void *obj, uint32_t idx, uint32_t width);
void *owf_array_at(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width);
void *owf_array_ptr_for(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width);

#define OWF_ARRAY_TYPED_PTR(arr, type) ((type *)((&(arr))->ptr))
#define OWF_ARRAY_PTR(arr, type, idx) (&OWF_ARRAY_TYPED_PTR(arr, type)[idx])
#define OWF_ARRAY_GET(arr, type, idx) (OWF_ARRAY_TYPED_PTR(arr, type)[idx])
#define OWF_ARRAY_PUT(arr, type, idx, value) do {OWF_ARRAY_GET(arr, type, idx) = value;} while (0)
#define OWF_ARRAY_LEN(arr) ((&(arr))->length)

void owf_buffer_init(owf_buffer_t *buf, void *ptr, size_t size);

#endif /* OWF_TYPES_H */
