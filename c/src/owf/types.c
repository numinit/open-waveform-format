#include <owf/types.h>

#include <string.h>

void owf_init(owf_t *owf) {
    owf_array_init(&owf->channels);
}

void owf_destroy(owf_t *owf, owf_alloc_t *allocator) {
    /* Destroy channels */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(owf->channels); i++) {
        owf_channel_destroy(OWF_ARRAY_PTR(owf->channels, owf_channel_t, i), allocator);
    }
    owf_array_destroy(&owf->channels, allocator);
}

void owf_channel_init(owf_channel_t *channel) {
    owf_str_init(&channel->id);
    owf_array_init(&channel->namespaces);
}

void owf_channel_destroy(owf_channel_t *channel, owf_alloc_t *allocator) {
    owf_str_destroy(&channel->id, allocator);

    /* Destroy namespaces */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(channel->namespaces); i++) {
        owf_namespace_destroy(OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, i), allocator);
    }
    owf_array_destroy(&channel->namespaces, allocator);
}

void owf_namespace_init(owf_namespace_t *ns) {
    owf_str_init(&ns->id);
    owf_array_init(&ns->signals);
    owf_array_init(&ns->events);
    owf_array_init(&ns->alarms);
}

void owf_namespace_destroy(owf_namespace_t *ns, owf_alloc_t *allocator) {
    owf_str_destroy(&ns->id, allocator);

    /* Destroy signals */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
        owf_signal_destroy(OWF_ARRAY_PTR(ns->signals, owf_signal_t, i), allocator);
    }
    owf_array_destroy(&ns->signals, allocator);

    /* Destroy events */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
        owf_event_destroy(OWF_ARRAY_PTR(ns->events, owf_event_t, i), allocator);
    }
    owf_array_destroy(&ns->events, allocator);

    /* Destroy alarms */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
        owf_alarm_destroy(OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i), allocator);
    }
    owf_array_destroy(&ns->alarms, allocator);
}

void owf_signal_init(owf_signal_t *signal) {
    owf_str_init(&signal->id);
    owf_str_init(&signal->unit);
    owf_array_init(&signal->samples);
}

void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *allocator) {
    owf_str_destroy(&signal->id, allocator);
    owf_str_destroy(&signal->unit, allocator);
    owf_array_destroy(&signal->samples, allocator);
}

void owf_event_init(owf_event_t *event) {
    owf_str_init(&event->data);
}

void owf_event_destroy(owf_event_t *event, owf_alloc_t *allocator) {
    owf_str_destroy(&event->data, allocator);
}

void owf_alarm_init(owf_alarm_t *alarm) {
    owf_str_init(&alarm->data);
}

void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *allocator) {
    owf_str_destroy(&alarm->data, allocator);
}

void owf_str_init(owf_str_t *str) {
    owf_array_init(&str->bytes);
}

bool owf_str_reserve(owf_str_t *str, owf_alloc_t *allocator, owf_error_t *error, uint32_t length) {
    uint32_t size;

    /* Initialize the array */
    owf_array_init(&str->bytes);
    size = owf_arith_safe_add32(length, 1, error);
    if (OWF_NOEXPECT(error->is_error)) {
        return false;
    }

    return owf_array_reserve_exactly(&str->bytes, allocator, error, sizeof(uint8_t), size);
}

void owf_str_destroy(owf_str_t *str, owf_alloc_t *allocator) {
    owf_array_destroy(&str->bytes, allocator);
}

uint32_t owf_str_bytesize(owf_str_t *str) {
    /* Extra for the trailing null byte that was added */
    return (uint32_t)strnlen(str->bytes.ptr, str->bytes.length - 1);
}

void owf_buffer_init(owf_buffer_t *buf, void *ptr, size_t length) {
    buf->ptr = ptr;
    buf->length = length;
    buf->position = 0;
}
