#include <owf/types.h>
#include <owf/platform.h>
#include <string.h>

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

void owf_array_destroy(owf_array_t *arr, owf_alloc_t *alloc) {
    owf_free(alloc, arr->ptr);
}

int owf_array_binary_compare(owf_array_t *lhs, owf_array_t *rhs, uint32_t width) {
    uint32_t lhs_len = OWF_ARRAY_LEN(*lhs), rhs_len = OWF_ARRAY_LEN(*rhs);
    if (OWF_EXPECT(lhs_len == rhs_len)) {
        uint32_t bytes = lhs_len * width;
        return memcmp(lhs->ptr, rhs->ptr, bytes);
    } else {
        return lhs_len < rhs_len ? -1 : 1;
    }
}

bool owf_array_reserve(owf_array_t *arr, owf_alloc_t *alloc, owf_error_t *error, uint32_t capacity, uint32_t width) {
    /* Extend the capacity by a factor of 3/2.
     * Do a safe multiply by 3, then divide by 2.
     * Keep 3 elements in this array, at minimum.
     */
    capacity = OWF_MAX(capacity, 3);
    OWF_ARITH_SAFE_MUL32(error, capacity, 3);
    capacity /= 2;

    return owf_array_reserve_exactly(arr, alloc, error, capacity, width);
}

bool owf_array_reserve_exactly(owf_array_t *arr, owf_alloc_t *alloc, owf_error_t *error, uint32_t capacity, uint32_t width) {
    void *ptr = arr->ptr;
    uint32_t new_size = 0;

    /* Calculate the new total size */
    if (OWF_NOEXPECT(!owf_arith_safe_mul32(capacity, width, &new_size, error))) {
        return false;
    } else if (OWF_NOEXPECT(new_size == 0)) {
        OWF_ERROR_SET(error, "tried to reserve zero-byte length");
        return false;
    }

    /* Reallocate */
    if (OWF_NOEXPECT(!owf_realloc(alloc, error, &ptr, new_size))) {
        return false;
    }

    /* Commit and refit the array length */
    arr->ptr = ptr;
    arr->capacity = capacity;
    arr->length = OWF_MIN(arr->length, capacity);
    return true;
}

bool owf_array_push(owf_array_t *arr, owf_alloc_t *alloc, owf_error_t *error, const void *obj, uint32_t width) {
    if (OWF_NOEXPECT(arr->length == arr->capacity && !owf_array_reserve(arr, alloc, error, arr->capacity + 1, width))) {
        return false;
    }

    return owf_array_put(arr, error, obj, arr->length++, width);
}

bool owf_array_put(owf_array_t *arr, owf_error_t *error, const void *obj, uint32_t idx, uint32_t width) {
    void *ptr = owf_array_ptr_for(arr, error, idx, width);
    if (OWF_NOEXPECT(ptr == NULL)) {
        return false;
    }

    /* Commit */
    memcpy(ptr, obj, width);
    return true;
}

void *owf_array_get(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width) {
    if (OWF_EXPECT(idx < arr->length)) {
        return owf_array_ptr_for(arr, error, idx, width);
    } else {
        OWF_ERROR_SETF(error, "array index out of bounds: " OWF_PRINT_U32 " >= " OWF_PRINT_U32, idx, arr->length);
        return NULL;
    }
}

void *owf_array_ptr_for(owf_array_t *arr, owf_error_t *error, uint32_t idx, uint32_t width) {
    uint32_t offset = 0;
    if (OWF_NOEXPECT(!owf_arith_safe_mul32(idx, width, &offset, error))) {
        return NULL;
    } else {
        return (uint8_t *)arr->ptr + offset;
    }
}

void owf_buffer_init(owf_buffer_t *buf, void *ptr, size_t length) {
    buf->ptr = ptr;
    buf->length = length;
    buf->position = 0;
}

void owf_package_init(owf_package_t *owf) {
    owf_memoize_init(&owf->memoize);
    owf_array_init(&owf->channels);
}

void owf_package_destroy(owf_package_t *owf, owf_alloc_t *alloc) {
    /* Destroy channels */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(owf->channels); i++) {
        owf_channel_destroy(OWF_ARRAY_PTR(owf->channels, owf_channel_t, i), alloc);
    }
    owf_array_destroy(&owf->channels, alloc);
}

#define OWF_PACKAGE_PRINT_FMT "#<owf_package_t@%p: [" OWF_PRINT_U32 " %s]>"
#define OWF_PACKAGE_PRINT_ARGS package, \
    OWF_ARRAY_LEN(package->channels), OWF_ARRAY_LEN(package->channels) == 1 ? "channel" : "channels"

int owf_package_print(owf_package_t *package, FILE *fp) {
    return fprintf(fp, OWF_PACKAGE_PRINT_FMT, OWF_PACKAGE_PRINT_ARGS);
}

int owf_package_stringify(owf_package_t *package, char *ptr, size_t size) {
    return owf_snprintf(ptr, size, OWF_PACKAGE_PRINT_FMT, OWF_PACKAGE_PRINT_ARGS);
}

int owf_package_compare(owf_package_t *lhs, owf_package_t *rhs) {
    OWF_ARRAY_SEMANTIC_COMPARE(lhs->channels, rhs->channels, owf_channel_t, owf_channel_compare);
    return 0;
}

bool owf_package_size(owf_package_t *owf, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&owf->memoize)) {
        uint32_t size = sizeof(uint32_t) * 2;
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(owf->channels); i++) {
            uint32_t channel_size = 0;
            if (OWF_EXPECT(owf_channel_size(OWF_ARRAY_PTR(owf->channels, owf_channel_t, i), error, &channel_size))) {
                OWF_ARITH_SAFE_ADD32(error, size, channel_size);
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

bool owf_package_push_channel(owf_package_t *owf, owf_alloc_t *alloc, owf_error_t *error, owf_channel_t *channel) {
    return owf_array_push(&owf->channels, alloc, error, channel, sizeof(owf_channel_t));
}

void owf_channel_init(owf_channel_t *channel) {
    owf_memoize_init(&channel->memoize);
    owf_str_init(&channel->id);
    owf_array_init(&channel->namespaces);
}

bool owf_channel_init_id(owf_channel_t *channel, owf_alloc_t *alloc, owf_error_t *error, const char *id) {
    owf_channel_init(channel);
    return owf_channel_set_id(channel, alloc, error, id);
}

void owf_channel_destroy(owf_channel_t *channel, owf_alloc_t *alloc) {
    owf_str_destroy(&channel->id, alloc);

    /* Destroy namespaces */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(channel->namespaces); i++) {
        owf_namespace_destroy(OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, i), alloc);
    }
    owf_array_destroy(&channel->namespaces, alloc);
}

#define OWF_CHANNEL_PRINT_FMT "#<owf_channel_t@%p: %s [" OWF_PRINT_U32 " %s]>"
#define OWF_CHANNEL_PRINT_ARGS channel, OWF_STR_PTR(channel->id), \
    OWF_ARRAY_LEN(channel->namespaces), OWF_ARRAY_LEN(channel->namespaces) == 1 ? "namespace" : "namespaces"

int owf_channel_print(owf_channel_t *channel, FILE *fp) {
    return fprintf(fp, OWF_CHANNEL_PRINT_FMT, OWF_CHANNEL_PRINT_ARGS);
}

int owf_channel_stringify(owf_channel_t *channel, char *ptr, size_t size) {
    return owf_snprintf(ptr, size, OWF_CHANNEL_PRINT_FMT, OWF_CHANNEL_PRINT_ARGS);
}

int owf_channel_compare(owf_channel_t *lhs, owf_channel_t *rhs) {
    int ret;
    if ((ret = owf_str_binary_compare(&lhs->id, &rhs->id)) != 0) {
        return ret;
    } else {
        OWF_ARRAY_SEMANTIC_COMPARE(lhs->namespaces, rhs->namespaces, owf_namespace_t, owf_namespace_compare);
        return 0;
    }
}

bool owf_channel_size(owf_channel_t *channel, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&channel->memoize)) {
        uint32_t size = sizeof(uint32_t), id_size = 0;

        if (OWF_EXPECT(owf_str_size(&channel->id, error, &id_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, id_size);
        } else {
            return false;
        }

        for (uint32_t i = 0; i < OWF_ARRAY_LEN(channel->namespaces); i++) {
            uint32_t namespace_size = 0;
            if (OWF_EXPECT(owf_namespace_size(OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, i), error, &namespace_size))) {
                OWF_ARITH_SAFE_ADD32(error, size, namespace_size);
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

bool owf_channel_set_id(owf_channel_t *channel, owf_alloc_t *alloc, owf_error_t *error, const char *id) {
    return owf_str_set(&channel->id, alloc, error, id);
}

bool owf_channel_push_namespace(owf_channel_t *channel, owf_alloc_t *alloc, owf_error_t *error, owf_namespace_t *ns) {
    return owf_array_push(&channel->namespaces, alloc, error, ns, sizeof(owf_namespace_t));
}

void owf_namespace_init(owf_namespace_t *ns) {
    owf_memoize_init(&ns->memoize);
    owf_str_init(&ns->id);
    owf_array_init(&ns->signals);
    owf_array_init(&ns->events);
    owf_array_init(&ns->alarms);
}

bool owf_namespace_init_id(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, const char *id) {
    owf_namespace_init(ns);
    return owf_str_set(&ns->id, alloc, error, id);
}

void owf_namespace_destroy(owf_namespace_t *ns, owf_alloc_t *alloc) {
    owf_str_destroy(&ns->id, alloc);

    /* Destroy signals */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
        owf_signal_destroy(OWF_ARRAY_PTR(ns->signals, owf_signal_t, i), alloc);
    }
    owf_array_destroy(&ns->signals, alloc);

    /* Destroy events */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
        owf_event_destroy(OWF_ARRAY_PTR(ns->events, owf_event_t, i), alloc);
    }
    owf_array_destroy(&ns->events, alloc);

    /* Destroy alarms */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
        owf_alarm_destroy(OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i), alloc);
    }
    owf_array_destroy(&ns->alarms, alloc);
}

#define OWF_NAMESPACE_PRINT_FMT "#<owf_namespace_t@%p: %s [t0=" OWF_PRINT_TIME ", dt=" OWF_PRINT_DURATION ", " \
    OWF_PRINT_U32 " %s, " OWF_PRINT_U32 " %s, " OWF_PRINT_U32 " %s]>"
#define OWF_NAMESPACE_PRINT_ARGS ns, OWF_STR_PTR(ns->id), ns->t0, ns->dt, \
    OWF_ARRAY_LEN(ns->signals), OWF_ARRAY_LEN(ns->signals) == 1 ? "signal" : "signals", \
    OWF_ARRAY_LEN(ns->events), OWF_ARRAY_LEN(ns->events) == 1 ? "event" : "events", \
    OWF_ARRAY_LEN(ns->alarms), OWF_ARRAY_LEN(ns->alarms) == 1 ? "alarm" : "alarms"

int owf_namespace_print(owf_namespace_t *ns, FILE *fp) {
    return fprintf(fp, OWF_NAMESPACE_PRINT_FMT, OWF_NAMESPACE_PRINT_ARGS);
}

int owf_namespace_stringify(owf_namespace_t *ns, char *ptr, size_t size) {
    return owf_snprintf(ptr, size, OWF_NAMESPACE_PRINT_FMT, OWF_NAMESPACE_PRINT_ARGS);
}

int owf_namespace_compare(owf_namespace_t *lhs, owf_namespace_t *rhs) {
    if (lhs->t0 != rhs->t0) {
        return lhs->t0 < rhs->t0 ? -1 : 1;
    } else if (rhs->dt != lhs->dt) {
        return lhs->dt < rhs->dt ? -1 : 1;
    }
    OWF_ARRAY_SEMANTIC_COMPARE(lhs->signals, rhs->signals, owf_signal_t, owf_signal_compare);
    OWF_ARRAY_SEMANTIC_COMPARE(lhs->events, rhs->events, owf_event_t, owf_event_compare);
    OWF_ARRAY_SEMANTIC_COMPARE(lhs->alarms, rhs->alarms, owf_alarm_t, owf_alarm_compare);
    return 0;
}

bool owf_namespace_covers(owf_namespace_t *ns, owf_time_t timestamp) {
    register owf_time_t start = ns->t0, end = start + ns->dt;
    return timestamp >= start && timestamp < end;
}

bool owf_namespace_size(owf_namespace_t *ns, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&ns->memoize)) {
        uint32_t size = sizeof(uint32_t) + 2 * sizeof(owf_time_t), id_size = 0;

        if (OWF_EXPECT(owf_str_size(&ns->id, error, &id_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, id_size);
        } else {
            return false;
        }

        uint32_t signals_size = sizeof(uint32_t);
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
            uint32_t signal_size = 0;
            if (OWF_EXPECT(owf_signal_size(OWF_ARRAY_PTR(ns->signals, owf_signal_t, i), error, &signal_size))) {
                OWF_ARITH_SAFE_ADD32(error, signals_size, signal_size);
            } else {
                return false;
            }
        }
        OWF_ARITH_SAFE_ADD32(error, size, signals_size);

        uint32_t events_size = sizeof(uint32_t);
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
            uint32_t event_size = 0;
            if (OWF_EXPECT(owf_event_size(OWF_ARRAY_PTR(ns->events, owf_event_t, i), error, &event_size))) {
                OWF_ARITH_SAFE_ADD32(error, events_size, event_size);
            } else {
                return false;
            }
        }
        OWF_ARITH_SAFE_ADD32(error, size, events_size);

        uint32_t alarms_size = sizeof(uint32_t);
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
            uint32_t alarm_size = 0;
            if (OWF_EXPECT(owf_alarm_size(OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i), error, &alarm_size))) {
                OWF_ARITH_SAFE_ADD32(error, alarms_size, alarm_size);
            } else {
                return false;
            }
        }
        OWF_ARITH_SAFE_ADD32(error, size, alarms_size);

        *output_size = owf_memoize_cache(&ns->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&ns->memoize);
    }

    return true;
}

bool owf_namespace_set_id(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, const char *id) {
    return owf_str_set(&ns->id, alloc, error, id);
}

bool owf_namespace_push_signal(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, owf_signal_t *signal) {
    return owf_array_push(&ns->signals, alloc, error, signal, sizeof(owf_signal_t));
}

bool owf_namespace_push_event(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, owf_event_t *event) {
    return owf_array_push(&ns->events, alloc, error, event, sizeof(owf_event_t));
}

bool owf_namespace_push_alarm(owf_namespace_t *ns, owf_alloc_t *alloc, owf_error_t *error, owf_alarm_t *alarm) {
    return owf_array_push(&ns->alarms, alloc, error, alarm, sizeof(owf_alarm_t));
}

void owf_signal_init(owf_signal_t *signal) {
    owf_memoize_init(&signal->memoize);
    owf_str_init(&signal->id);
    owf_str_init(&signal->unit);
    owf_array_init(&signal->samples);
}

bool owf_signal_init_id_unit(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const char *id, const char *unit) {
    owf_signal_init(signal);
    if (OWF_EXPECT(owf_str_set(&signal->id, alloc, error, id))) {
        if (OWF_NOEXPECT(!owf_str_set(&signal->unit, alloc, error, unit))) {
            owf_str_destroy(&signal->id, alloc);
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

void owf_signal_destroy(owf_signal_t *signal, owf_alloc_t *alloc) {
    owf_str_destroy(&signal->id, alloc);
    owf_str_destroy(&signal->unit, alloc);
    owf_array_destroy(&signal->samples, alloc);
}

#define OWF_SIGNAL_PRINT_FMT "#<owf_signal_t@%p: [id=%s, unit=%s, " OWF_PRINT_U32 " %s]>"
#define OWF_SIGNAL_PRINT_ARGS signal, OWF_STR_PTR(signal->id), OWF_STR_PTR(signal->unit), \
    OWF_ARRAY_LEN(signal->samples), OWF_ARRAY_LEN(signal->samples) == 1 ? "sample" : "samples"

int owf_signal_print(owf_signal_t *signal, FILE *fp) {
    return fprintf(fp, OWF_SIGNAL_PRINT_FMT, OWF_SIGNAL_PRINT_ARGS);
}

int owf_signal_stringify(owf_signal_t *signal, char *ptr, size_t size) {
    return owf_snprintf(ptr, size, OWF_SIGNAL_PRINT_FMT, OWF_SIGNAL_PRINT_ARGS);
}

int owf_signal_compare(owf_signal_t *lhs, owf_signal_t *rhs) {
    int ret;
    if ((ret = owf_str_binary_compare(&lhs->id, &rhs->id)) != 0) {
        return ret;
    } else if ((ret = owf_str_binary_compare(&lhs->unit, &rhs->unit)) != 0) {
        return ret;
    } else {
        return owf_array_binary_compare(&lhs->samples, &rhs->samples, sizeof(double));
    }
}

bool owf_signal_size(owf_signal_t *signal, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&signal->memoize)) {
        uint32_t size = 0, component_size = 0;

        /* Calculate the ID size */
        if (OWF_EXPECT(owf_str_size(&signal->id, error, &component_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, component_size);
        } else {
            return false;
        }

        /* Calculate the unit size */
        if (OWF_EXPECT(owf_str_size(&signal->unit, error, &component_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, component_size);
        } else {
            return false;
        }

        /* Calculate the samples size */
        component_size = sizeof(double);
        OWF_ARITH_SAFE_ADD32(error, size, sizeof(uint32_t));
        OWF_ARITH_SAFE_MUL32(error, component_size, OWF_ARRAY_LEN(signal->samples));
        OWF_ARITH_SAFE_ADD32(error, size, component_size);

        *output_size = owf_memoize_cache(&signal->memoize, size);
    } else {
        *output_size = owf_memoize_fetch(&signal->memoize);
    }

    return true;
}

bool owf_signal_set_id(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const char *id) {
    return owf_str_set(&signal->id, alloc, error, id);
}

bool owf_signal_set_unit(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const char *unit) {
    return owf_str_set(&signal->unit, alloc, error, unit);
}

bool owf_signal_push_samples(owf_signal_t *signal, owf_alloc_t *alloc, owf_error_t *error, const double *samples, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (OWF_NOEXPECT(!owf_array_push(&signal->samples, alloc, error, &samples[i], sizeof(samples[i])))) {
            return false;
        }
    }

    return true;
}

void owf_event_init(owf_event_t *event) {
    owf_memoize_init(&event->memoize);
    owf_str_init(&event->message);
}

bool owf_event_init_message(owf_event_t *event, owf_alloc_t *alloc, owf_error_t *error, const char *message) {
    owf_event_init(event);
    return owf_str_set(&event->message, alloc, error, message);
}

void owf_event_destroy(owf_event_t *event, owf_alloc_t *alloc) {
    owf_str_destroy(&event->message, alloc);
}

#define OWF_EVENT_PRINT_FMT "#<owf_event_t@%p: [message=%s, t0=" OWF_PRINT_TIME "]>"
#define OWF_EVENT_PRINT_ARGS event, OWF_STR_PTR(event->message), event->t0

int owf_event_print(owf_event_t *event, FILE *fp) {
    return fprintf(fp, OWF_EVENT_PRINT_FMT, OWF_EVENT_PRINT_ARGS);
}

int owf_event_stringify(owf_event_t *event, char *ptr, size_t size) {
    return owf_snprintf(ptr, size, OWF_EVENT_PRINT_FMT, OWF_EVENT_PRINT_ARGS);
}

int owf_event_compare(owf_event_t *lhs, owf_event_t *rhs) {
    if (lhs->t0 != rhs->t0) {
        return lhs->t0 < rhs->t0 ? -1 : 1;
    } else {
        return owf_str_binary_compare(&lhs->message, &rhs->message);
    }
}

bool owf_event_size(owf_event_t *event, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&event->memoize)) {
        uint32_t size = sizeof(owf_time_t), message_size = 0;

        /* Calculate the message size */
        if (OWF_EXPECT(owf_str_size(&event->message, error, &message_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, message_size);
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

bool owf_alarm_init_type_message(owf_alarm_t *alarm, owf_alloc_t *alloc, owf_error_t *error, const char *type, const char *message) {
    owf_alarm_init(alarm);
    if (OWF_EXPECT(owf_str_set(&alarm->type, alloc, error, type))) {
        if (OWF_NOEXPECT(!owf_str_set(&alarm->message, alloc, error, message))) {
            owf_str_destroy(&alarm->type, alloc);
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

void owf_alarm_destroy(owf_alarm_t *alarm, owf_alloc_t *alloc) {
    owf_str_destroy(&alarm->type, alloc);
    owf_str_destroy(&alarm->message, alloc);
}

#define OWF_ALARM_PRINT_FMT "#<owf_alarm_t@%p: [type=%s, message=%s, t0=" OWF_PRINT_TIME ", dt=" OWF_PRINT_DURATION \
    ", type=" OWF_PRINT_U8 ", volume=" OWF_PRINT_U8 "]>"
#define OWF_ALARM_PRINT_ARGS alarm, OWF_STR_PTR(alarm->type), OWF_STR_PTR(alarm->message), \
    alarm->t0, alarm->dt, alarm->details.u8.level, alarm->details.u8.volume

int owf_alarm_print(owf_alarm_t *alarm, FILE *fp) {
    return fprintf(fp, OWF_ALARM_PRINT_FMT, OWF_ALARM_PRINT_ARGS);
}

int owf_alarm_stringify(owf_alarm_t *alarm, char *ptr, size_t size) {
    return owf_snprintf(ptr, size, OWF_ALARM_PRINT_FMT, OWF_ALARM_PRINT_ARGS);
}

int owf_alarm_compare(owf_alarm_t *lhs, owf_alarm_t *rhs) {
    int ret;

    if (lhs->t0 != rhs->t0) {
        return lhs->t0 < rhs->t0 ? -1 : 1;
    } else if (lhs->dt != rhs->dt) {
        return lhs->dt < rhs->dt ? -1 : 1;
    } else if (lhs->details.u8.level != rhs->details.u8.level) {
        return lhs->details.u8.level < rhs->details.u8.level ? -1 : 1;
    } else if (lhs->details.u8.volume != rhs->details.u8.volume) {
        return lhs->details.u8.volume < rhs->details.u8.volume ? -1 : 1;
    } else if ((ret = owf_str_binary_compare(&lhs->type, &rhs->type)) != 0) {
        return ret;
    } else {
        return owf_str_binary_compare(&lhs->message, &rhs->message);
    }
}

bool owf_alarm_size(owf_alarm_t *alarm, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&alarm->memoize)) {
        uint32_t size = sizeof(owf_time_t) * 2 + sizeof(uint8_t) * 2 + sizeof(uint16_t), component_size = 0;

        /* Calculate the type size */
        if (OWF_EXPECT(owf_str_size(&alarm->type, error, &component_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, component_size);
        } else {
            return false;
        }

        /* Calculate the message size */
        if (OWF_EXPECT(owf_str_size(&alarm->message, error, &component_size))) {
            OWF_ARITH_SAFE_ADD32(error, size, component_size);
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

bool owf_str_set(owf_str_t *str, owf_alloc_t *alloc, owf_error_t *error, const char *value) {
    size_t size = strlen(value);
    owf_str_destroy(str, alloc);
    owf_str_init(str);

    if (OWF_NOEXPECT(size >= UINT32_MAX)) {
        /* No truncation, please */
        OWF_ERROR_SETF(error, "strlen of input string (" OWF_PRINT_SIZE ") was greater than UINT32_MAX (" OWF_PRINT_U32 ")", size, UINT32_MAX);
        return false;
    } else if (OWF_EXPECT(size > 0)) {
        uint32_t truncated_size = (uint32_t)size;

        /* Safely truncate the size and reserve that many bytes */
        if (OWF_NOEXPECT(!owf_str_reserve(str, alloc, error, truncated_size))) {
            return false;
        } else {
            memcpy(str->bytes.ptr, value, truncated_size);
            ((uint8_t *)str->bytes.ptr)[truncated_size] = 0;
            str->bytes.length = truncated_size + 1;
            return true;
        }
    } else {
        /* Zero-length string, nothing more to be done. */
        return true;
    }
}

bool owf_str_reserve(owf_str_t *str, owf_alloc_t *alloc, owf_error_t *error, uint32_t length) {
    uint32_t size;

    /* Initialize the array */
    owf_array_init(&str->bytes);
    if (OWF_NOEXPECT(!owf_arith_safe_add32(length, 1, &size, error))) {
        return false;
    }

    return owf_array_reserve_exactly(&str->bytes, alloc, error, sizeof(uint8_t), size);
}

void owf_str_destroy(owf_str_t *str, owf_alloc_t *alloc) {
    owf_array_destroy(&str->bytes, alloc);
}

int owf_str_binary_compare(owf_str_t *lhs, owf_str_t *rhs) {
    uint32_t lhs_len = owf_str_length(lhs), rhs_len = owf_str_length(rhs);
    if (OWF_EXPECT(lhs_len == rhs_len)) {
        const char *p1 = OWF_STR_PTR(*lhs), *p2 = OWF_STR_PTR(*rhs);
        return strncmp(p1, p2, lhs_len);
    } else {
        return lhs_len < rhs_len ? -1 : 1;
    }
}

uint32_t owf_str_length(owf_str_t *str) {
    if (owf_memoize_stale(&str->string_size)) {
        /* OWF_ARRAY_LEN(str->bytes) returns a uint32_t, so we can truncate the return value of strnlen */
        return owf_memoize_cache(&str->string_size, OWF_ARRAY_LEN(str->bytes) == 0 ? 0 : (uint32_t)strnlen(str->bytes.ptr, OWF_ARRAY_LEN(str->bytes)));
    } else {
        return owf_memoize_fetch(&str->string_size);
    }
}

static uint32_t owf_str_padding(uint32_t length) {
    uint32_t tmp = length % sizeof(uint32_t);
    return tmp == 0 ? 0 : sizeof(uint32_t) - tmp;
}

bool owf_str_size(owf_str_t *str, owf_error_t *error, uint32_t *output_size) {
    if (owf_memoize_stale(&str->total_size)) {
        // Get the length in bytes of the string's byte array, not including a trailing null byte
        uint32_t length = owf_str_length(str), padding;

        if (OWF_EXPECT(length > 0)) {
            // Add the trailing null byte
            OWF_ARITH_SAFE_ADD32(error, length, 1);

            // Pad out the length, counting the null byte
            padding = owf_str_padding(length);

            // Add the padding to the length
            OWF_ARITH_SAFE_ADD32(error, length, padding);
        }

        // Add size for the header
        OWF_ARITH_SAFE_ADD32(error, length, sizeof(uint32_t));

        // Cache the new total size
        *output_size = owf_memoize_cache(&str->total_size, length);
    } else {
        *output_size = owf_memoize_fetch(&str->total_size);
    }

    return true;
}
