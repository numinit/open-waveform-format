#include <owf/types.h>

#include <string.h>

void owf_init(owf_t *owf) {
    owf_memoize_init(&owf->memoize);
    owf_array_init(&owf->channels);
}

void owf_destroy(owf_t *owf, owf_alloc_t *allocator) {
    /* Destroy channels */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(owf->channels); i++) {
        owf_channel_destroy(OWF_ARRAY_PTR(owf->channels, owf_channel_t, i), allocator);
    }
    owf_array_destroy(&owf->channels, allocator);
}

int owf_compare(owf_t *lhs, owf_t *rhs) {
    OWF_ARRAY_SEMANTIC_COMPARE(owf_channel_t, lhs->channels, rhs->channels, owf_channel_compare);
    return 0;
}

bool owf_size(owf_t *owf, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&owf->memoize)) {
        uint32_t size = 0;
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(owf->channels); i++) {
            uint32_t channel_size = 0;
            if (OWF_EXPECT(owf_channel_size(OWF_ARRAY_PTR(owf->channels, owf_channel_t, i), error, &channel_size))) {
                OWF_ARITH_SAFE_ADD32(*error, size, channel_size);
            } else {
                return false;
            }
        }
        *output_size = owf_memoize_cache(&owf->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&owf->memoize);
    }
    return true;
}

void owf_channel_init(owf_channel_t *channel) {
    owf_memoize_init(&channel->memoize);
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

int owf_channel_compare(owf_channel_t *lhs, owf_channel_t *rhs) {
    int ret;
    if ((ret = owf_str_compare(&lhs->id, &rhs->id)) != 0) {
        return ret;
    } else {
        OWF_ARRAY_SEMANTIC_COMPARE(owf_namespace_t, lhs->namespaces, rhs->namespaces, owf_namespace_compare);
        return 0;
    }
}

bool owf_channel_size(owf_channel_t *channel, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&channel->memoize)) {
        uint32_t size = sizeof(uint32_t), id_size = 0;

        if (OWF_EXPECT(owf_str_size(&channel->id, error, &id_size))) {
            OWF_ARITH_SAFE_ADD32(*error, size, id_size);
        } else {
            return false;
        }

        for (uint32_t i = 0; i < OWF_ARRAY_LEN(channel->namespaces); i++) {
            uint32_t namespace_size = 0;
            if (OWF_EXPECT(owf_namespace_size(OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, i), error, &namespace_size))) {
                OWF_ARITH_SAFE_ADD32(*error, size, namespace_size);
            } else {
                return false;
            }
        }

        *output_size = owf_memoize_cache(&channel->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&channel->memoize);
    }

    return true;
}

void owf_namespace_init(owf_namespace_t *ns) {
    owf_memoize_init(&ns->memoize);
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

int owf_namespace_compare(owf_namespace_t *lhs, owf_namespace_t *rhs) {
    if (lhs->t0 != rhs->t0) {
        return lhs->t0 < rhs->t0 ? -1 : 1;
    } else if (rhs->dt != lhs->dt) {
        return lhs->dt < rhs->dt ? -1 : 1;
    }
    OWF_ARRAY_SEMANTIC_COMPARE(owf_signal_t, lhs->signals, rhs->signals, owf_signal_compare);
    OWF_ARRAY_SEMANTIC_COMPARE(owf_event_t, lhs->events, rhs->events, owf_event_compare);
    OWF_ARRAY_SEMANTIC_COMPARE(owf_alarm_t, lhs->alarms, rhs->alarms, owf_alarm_compare);
    return 0;
}

bool owf_namespace_covers(owf_namespace_t *ns, owf_time_t timestamp) {
    register owf_time_t start = ns->t0, end = start + ns->dt;
    return timestamp >= start && timestamp < end;
}

bool owf_namespace_size(owf_namespace_t *ns, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&ns->memoize)) {
        uint32_t size = sizeof(uint32_t) + 2 * sizeof(owf_time_t), id_size = 0;

        if (owf_str_size(&ns->id, error, &id_size)) {
            OWF_ARITH_SAFE_ADD32(*error, size, id_size);
        } else {
            return false;
        }

        uint32_t signals_size = sizeof(uint32_t);
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
            uint32_t signal_size = 0;
            if (OWF_EXPECT(owf_signal_size(OWF_ARRAY_PTR(ns->signals, owf_signal_t, i), error, &signal_size))) {
                OWF_ARITH_SAFE_ADD32(*error, signals_size, signal_size);
            } else {
                return false;
            }
        }
        OWF_ARITH_SAFE_ADD32(*error, size, signals_size);

        uint32_t events_size = sizeof(uint32_t);
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
            uint32_t event_size = 0;
            if (OWF_EXPECT(owf_event_size(OWF_ARRAY_PTR(ns->events, owf_event_t, i), error, &event_size))) {
                OWF_ARITH_SAFE_ADD32(*error, events_size, event_size);
            } else {
                return false;
            }
        }
        OWF_ARITH_SAFE_ADD32(*error, size, events_size);

        uint32_t alarms_size = sizeof(uint32_t);
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
            uint32_t alarm_size = 0;
            if (OWF_EXPECT(owf_alarm_size(OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i), error, &alarm_size))) {
                OWF_ARITH_SAFE_ADD32(*error, alarms_size, alarm_size);
            } else {
                return false;
            }
        }
        OWF_ARITH_SAFE_ADD32(*error, size, alarms_size);

        *output_size = owf_memoize_cache(&ns->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&ns->memoize);
    }

    return true;
}

void owf_signal_init(owf_signal_t *signal) {
    owf_memoize_init(&signal->memoize);
    owf_str_init(&signal->id);
    owf_str_init(&signal->unit);
    owf_array_init(&signal->samples);
}

void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *allocator) {
    owf_str_destroy(&signal->id, allocator);
    owf_str_destroy(&signal->unit, allocator);
    owf_array_destroy(&signal->samples, allocator);
}

int owf_signal_compare(owf_signal_t *lhs, owf_signal_t *rhs) {
    int ret;
    if ((ret = owf_str_compare(&lhs->id, &rhs->id)) != 0) {
        return ret;
    } else if ((ret = owf_str_compare(&lhs->unit, &rhs->unit)) != 0) {
        return ret;
    } else {
        return owf_array_binary_compare(&lhs->samples, &rhs->samples, sizeof(double));
    }
}

bool owf_signal_size(owf_signal_t *signal, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&signal->memoize)) {
        uint32_t size = 0, component_size = 0;

        /* Calculate the ID size */
        if (owf_str_size(&signal->id, error, &component_size)) {
            OWF_ARITH_SAFE_ADD32(*error, size, component_size);
        } else {
            return false;
        }

        /* Calculate the unit size */
        if (owf_str_size(&signal->unit, error, &component_size)) {
            OWF_ARITH_SAFE_ADD32(*error, size, component_size);
        } else {
            return false;
        }

        /* Calculate the samples size */
        component_size = sizeof(double);
        OWF_ARITH_SAFE_ADD32(*error, size, sizeof(uint32_t));
        OWF_ARITH_SAFE_MUL32(*error, component_size, OWF_ARRAY_LEN(signal->samples));
        OWF_ARITH_SAFE_ADD32(*error, size, component_size);

        *output_size = owf_memoize_cache(&signal->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&signal->memoize);
    }

    return true;
}

void owf_event_init(owf_event_t *event) {
    owf_memoize_init(&event->memoize);
    owf_str_init(&event->message);
}

void owf_event_destroy(owf_event_t *event, owf_alloc_t *allocator) {
    owf_str_destroy(&event->message, allocator);
}

int owf_event_compare(owf_event_t *lhs, owf_event_t *rhs) {
    if (lhs->t0 != rhs->t0) {
        return lhs->t0 < rhs->t0 ? -1 : 1;
    } else {
        return owf_str_compare(&lhs->message, &rhs->message);
    }
}

bool owf_event_size(owf_event_t *event, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&event->memoize)) {
        uint32_t size = sizeof(owf_time_t), message_size = 0;

        /* Calculate the message size */
        if (owf_str_size(&event->message, error, &message_size)) {
            OWF_ARITH_SAFE_ADD32(*error, size, message_size);
        } else {
            return false;
        }

        *output_size = owf_memoize_cache(&event->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&event->memoize);
    }

    return true;
}

void owf_alarm_init(owf_alarm_t *alarm) {
    owf_memoize_init(&alarm->memoize);
    owf_str_init(&alarm->type);
    owf_str_init(&alarm->message);
}

void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *allocator) {
    owf_str_destroy(&alarm->type, allocator);
    owf_str_destroy(&alarm->message, allocator);
}

int owf_alarm_compare(owf_alarm_t *lhs, owf_alarm_t *rhs) {
    int ret;

    if (lhs->t0 != rhs->t0) {
        return lhs->t0 < rhs->t0 ? -1 : 1;
    } else if (lhs->dt != rhs->dt) {
        return lhs->dt < rhs->dt ? -1 : 1;
    } else if (lhs->details.level != rhs->details.level) {
        return lhs->details.level < rhs->details.level ? -1 : 1;
    } else if (lhs->details.volume != rhs->details.volume) {
        return lhs->details.volume < rhs->details.volume ? -1 : 1;
    } else if ((ret = owf_str_compare(&lhs->type, &rhs->type)) != 0) {
        return ret;
    } else {
        return owf_str_compare(&lhs->message, &rhs->message);
    }
}

bool owf_alarm_size(owf_alarm_t *alarm, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&alarm->memoize)) {
        uint32_t size = sizeof(owf_time_t) * 2 + sizeof(uint8_t) * 2 + sizeof(uint16_t), component_size = 0;

        /* Calculate the type size */
        if (owf_str_size(&alarm->type, error, &component_size)) {
            OWF_ARITH_SAFE_ADD32(*error, size, component_size);
        } else {
            return false;
        }

        /* Calculate the message size */
        if (owf_str_size(&alarm->message, error, &component_size)) {
            OWF_ARITH_SAFE_ADD32(*error, size, component_size);
        } else {
            return false;
        }

        *output_size = owf_memoize_cache(&alarm->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&alarm->memoize);
    }

    return true;
}

void owf_str_init(owf_str_t *str) {
    owf_memoize_init(&str->string_size);
    owf_memoize_init(&str->total_size);
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

int owf_str_compare(owf_str_t *lhs, owf_str_t *rhs) {
    uint32_t lhs_len = owf_str_length(lhs), rhs_len = owf_str_length(rhs);
    if (lhs_len == rhs_len) {
        const char *p1 = OWF_STR_PTR(*lhs), *p2 = OWF_STR_PTR(*rhs);
        return strncmp(p1, p2, lhs_len);
    } else {
        return lhs_len < rhs_len ? -1 : 1;
    }
}

uint32_t owf_str_length(owf_str_t *str) {
    if (owf_memoize_stale(&str->string_size)) {
        return owf_memoize_cache(&str->string_size, (uint32_t)strnlen(str->bytes.ptr, OWF_ARRAY_LEN(str->bytes)));
    } else {
        return owf_memoize_fetch(&str->string_size);
    }
}

uint32_t owf_str_padding(uint32_t length) {
    uint32_t tmp = length % sizeof(uint32_t);
    return tmp == 0 ? 0 : sizeof(uint32_t) - tmp;
}

bool owf_str_size(owf_str_t *str, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&str->total_size)) {
        // Get the length in bytes of the string's byte array, not including a trailing null byte
        uint32_t length = owf_str_length(str);

        // Add the size of the length header, plus a trailing null byte
        uint32_t padding = owf_str_padding(length) + sizeof(uint32_t) + 1;
        OWF_ARITH_SAFE_ADD32(*error, length, padding);

        // Cache the new total size
        return owf_memoize_cache(&str->total_size, length);
    } else {
        return owf_memoize_fetch(&str->total_size);
    }
}

void owf_memoize_init(owf_memoize_t *memoize) {
    memoize->length = UINT32_MAX;
}

bool owf_memoize_stale(owf_memoize_t *memoize) {
    return memoize->length == UINT32_MAX;
}

uint32_t owf_memoize_fetch(owf_memoize_t *memoize) {
    return memoize->length;
}

uint32_t owf_memoize_cache(owf_memoize_t *memoize, uint32_t value) {
    memoize->length = value;
    return value;
}

void owf_array_init(owf_array_t *arr) {
    arr->ptr = NULL;
    arr->length = 0;
    arr->capacity = 0;
}

void owf_array_destroy(owf_array_t *arr, owf_alloc_t *allocator) {
    owf_free(allocator, arr->ptr);
}

int owf_array_binary_compare(owf_array_t *lhs, owf_array_t *rhs, uint32_t width) {
    uint32_t lhs_len = OWF_ARRAY_LEN(*lhs), rhs_len = OWF_ARRAY_LEN(*rhs);
    if (lhs_len == rhs_len) {
        uint32_t bytes = lhs_len * width;
        return memcmp(lhs->ptr, rhs->ptr, bytes);
    } else {
        return lhs_len < rhs_len ? -1 : 1;
    }
}

bool owf_array_reserve(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width) {
    /*
     * Extend the capacity by a factor of 3/2.
     * Do a safe multiply by 3, then divide by 2.
     */
    OWF_ARITH_SAFE_MUL32(*error, capacity, 3);
    capacity /= 2;

    return owf_array_reserve_exactly(arr, allocator, error, capacity, width);
}

bool owf_array_reserve_exactly(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, uint32_t capacity, uint32_t width) {
    void *ptr = arr->ptr;
    uint32_t new_size;

    /* Calculate the new total size */
    new_size = owf_arith_safe_mul32(capacity, width, error);
    if (OWF_NOEXPECT(error->is_error)) {
        return false;
    }
    else if (OWF_NOEXPECT(new_size == 0)) {
        OWF_ERR_SET(*error, "tried to reserve zero-byte length");
        return false;
    }

    /* Reallocate */
    ptr = owf_realloc(allocator, error, ptr, new_size);
    if (OWF_NOEXPECT(ptr == NULL)) {
        return false;
    }

    /* Commit and refit the array length */
    arr->ptr = ptr;
    arr->capacity = capacity;
    arr->length = OWF_MIN(arr->length, capacity);
    return true;
}

bool owf_array_push(owf_array_t *arr, owf_alloc_t *allocator, owf_error_t *error, void *obj, uint32_t width) {
    if (OWF_NOEXPECT(arr->length == arr->capacity && !owf_array_reserve(arr, allocator, error, arr->capacity + 1, width))) {
        return false;
    }

    return owf_array_put(arr, error, obj, arr->length++, width);
}

bool owf_array_put(owf_array_t *arr, owf_error_t *error, void *obj, uint32_t idx, uint32_t width) {
    void *ptr = owf_array_ptr_for(arr, error, idx, width);
    if (OWF_NOEXPECT(ptr == NULL)) {
        return false;
    }

    /* Commit */
    memcpy(ptr, obj, width);
    return true;
}

void *owf_array_at(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width) {
    if (OWF_EXPECT(idx < arr->length)) {
        return owf_array_ptr_for(arr, error, idx, width);
    }
    else {
        OWF_ERR_SETF(*error, "array index out of bounds: " OWF_PRINT_U32 " >= " OWF_PRINT_U32, idx, arr->length);
        return NULL;
    }
}

void *owf_array_ptr_for(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width) {
    uint32_t offset = owf_arith_safe_mul32(idx, width, error);
    if (OWF_NOEXPECT(error->is_error)) {
        return NULL;
    }
    else {
        return (uint8_t *)arr->ptr + offset;
    }
}

void owf_buffer_init(owf_buffer_t *buf, void *ptr, size_t length) {
    buf->ptr = ptr;
    buf->length = length;
    buf->position = 0;
}