#include <owf/types.h>

void owf_channel_destroy(owf_channel_t *channel, owf_alloc_t *allocator) {
    owf_str_destroy(&channel->id, allocator);

    /* Destroy namespaces */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(channel->namespaces); i++) {
        owf_namespace_destroy(OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, i), allocator);
    }
    owf_array_destroy(&channel->namespaces, allocator);
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

void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *allocator) {
    owf_str_destroy(&signal->id, allocator);
    owf_str_destroy(&signal->unit, allocator);
    owf_array_destroy(&signal->samples, allocator);
}

void owf_event_destroy(owf_event_t *event, owf_alloc_t *allocator) {
    owf_str_destroy(&event->data, allocator);
}

void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *allocator) {
    owf_str_destroy(&alarm->data, allocator);
}

bool owf_str_init(owf_str_t *str, owf_alloc_t *allocator, owf_error_t *error, uint32_t length) {
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
