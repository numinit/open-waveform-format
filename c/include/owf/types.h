#include <owf.h>
#include <owf/array.h>
#include <owf/alloc.h>
#include <owf/arith.h>

#ifndef OWF_TYPES_H
#define OWF_TYPES_H

typedef int64_t owf_time_t;

typedef struct owf_str {
    owf_array_t bytes;
} owf_str_t;

typedef struct owf_signal {
    owf_str_t id;
    owf_str_t unit;
    owf_array_t samples;
} owf_signal_t;

typedef struct owf_event {
    owf_time_t time;
    owf_str_t data;
} owf_event_t;

typedef struct owf_alarm {
    owf_time_t time;
    owf_str_t data;
} owf_alarm_t;

typedef struct owf_namespace {
    owf_str_t id;
    owf_time_t t0, dt;
    owf_array_t signals, events, alarms;
} owf_namespace_t;

typedef struct owf_channel {
    owf_str_t id;
    owf_array_t namespaces;
} owf_channel_t;

typedef struct owf {
    owf_array_t channels;
} owf_t;

void owf_init(owf_t *owf);
void owf_destroy(owf_t *owf, owf_alloc_t *allocator);
void owf_channel_init(owf_channel_t *channel);
void owf_channel_destroy(owf_channel_t *channel, owf_alloc_t *allocator);
void owf_namespace_init(owf_namespace_t *ns);
void owf_namespace_destroy(owf_namespace_t *ns, owf_alloc_t *allocator);
void owf_signal_init(owf_signal_t *signal);
void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *allocator);
void owf_event_init(owf_event_t *event);
void owf_event_destroy(owf_event_t *event, owf_alloc_t *allocator);
void owf_alarm_init(owf_alarm_t *alarm);
void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *allocator);
void owf_str_init(owf_str_t *str);
bool owf_str_reserve(owf_str_t *str, owf_alloc_t *allocator, owf_error_t *error, uint32_t length);
void owf_str_destroy(owf_str_t *str, owf_alloc_t *allocator);
uint32_t owf_str_bytesize(owf_str_t *str);

#endif /* OWF_TYPES_H */
