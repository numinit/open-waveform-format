#include <owf/binary.h>

#define OWF_BINARY_SAFE_ADD32(binary, a, b) \
    do { \
        uint32_t tmp = owf_arith_safe_add32(a, b, &binary->reader.error); \
        if (OWF_NOEXPECT(binary->reader.error.is_error)) { \
            return false; \
        } else { \
            a = tmp; \
        } \
    } while (0)

#define OWF_BINARY_SAFE_SUB32(binary, a, b) \
    do { \
        uint32_t tmp = owf_arith_safe_sub32(a, b, &binary->reader.error); \
        if (OWF_NOEXPECT(binary->reader.error.is_error)) { \
            return false; \
        } else { \
            a = tmp; \
        } \
    } while (0)

#define OWF_BINARY_SAFE_READ(binary, ptr, length) \
    do { \
        /* Length of zero is a no-op */ \
        if (OWF_NOEXPECT(length > 0 && !binary->reader.read(ptr, length, binary->reader.data))) { \
            OWF_READER_ERRF(binary->reader, "read error (" OWF_PRINT_U32 " bytes)", (uint32_t)length); \
            return false; \
        } else { \
            OWF_BINARY_SAFE_SUB32(binary, binary->segment_length, length); \
        } \
    } while (0)

#define OWF_BINARY_SAFE_VARIABLE_READ(binary, arr, len, elem_size, padding) \
    do { \
        uint32_t effective_length = len; \
        OWF_BINARY_SAFE_ADD32(binary, effective_length, padding); \
        \
        /* Length of zero is a no-op */ \
        if (OWF_EXPECT(effective_length > 0)) { \
            if (!owf_array_reserve_exactly((&(arr)), binary->reader.alloc, &binary->reader.error, effective_length, 1)) { \
                return false; \
            } \
            (&(arr))->length = effective_length / (elem_size); \
            \
            if (OWF_NOEXPECT(!binary->reader.read((&(arr))->ptr, len, binary->reader.data))) { \
                OWF_READER_ERRF(binary->reader, "variable read error (" OWF_PRINT_U32 " bytes into buffer of length " OWF_PRINT_U32 ")", (uint32_t)len, (uint32_t)effective_length); \
                owf_array_destroy((&(arr)), binary->reader.alloc); \
                return false; \
            } else { \
                binary->segment_length = owf_arith_safe_sub32(binary->segment_length, len, &binary->reader.error); \
                if (OWF_NOEXPECT(owf_reader_is_error(&binary->reader))) { \
                    OWF_READER_ERRF(binary->reader, "out-of-bounds length (" OWF_PRINT_U32 " bytes for segment length " OWF_PRINT_U32 ")", (uint32_t)len, binary->segment_length); \
                    owf_array_destroy((&(arr)), binary->reader.alloc); \
                    return false; \
                } \
            } \
        } \
    } while (0)

#define OWF_BINARY_READER_VISIT(binary, type) \
    do { \
        if (!OWF_READER_VISIT(binary->reader, type)) { \
            /* Skip the rest of the segment */ \
            binary->skip_length = binary->segment_length; \
            return !owf_reader_is_error(&binary->reader); \
        } \
    } while (0)

void owf_binary_reader_init(owf_binary_reader_t *binary, owf_alloc_t *alloc, owf_reader_read_cb_t read, owf_reader_visit_cb_t visitor, void *data) {
    owf_reader_init(&binary->reader, alloc, read, visitor, data);
    binary->segment_length = binary->skip_length = 0;
}

static bool owf_binary_reader_file_read_cb(void *dest, size_t size, void *data) {
    FILE *ptr = (FILE *)data;
    return fread(dest, sizeof(uint8_t), size, ptr) == size;
}

void owf_binary_reader_init_file(owf_binary_reader_t *binary, FILE *file, owf_alloc_t *alloc, owf_reader_visit_cb_t visitor) {
    owf_reader_init(&binary->reader, alloc, owf_binary_reader_file_read_cb, visitor, file);
}

static bool owf_binary_reader_buffer_read_cb(void *dest, size_t size, void *data) {
    owf_buffer_t *ptr = (owf_buffer_t *)data;
    size_t new_position = ptr->position + size;
    if (OWF_NOEXPECT(new_position > ptr->length)) {
        /* Attempted to read more bytes than this buffer has */
        return false;
    }

    /* Copy memory and update the position in the stream */
    memcpy(dest, (uint8_t *)ptr->ptr + ptr->position, size);
    ptr->position = new_position;
    return true;
}

void owf_binary_reader_init_buffer(owf_binary_reader_t *binary, owf_buffer_t *buf, owf_alloc_t *alloc, owf_reader_visit_cb_t visitor) {
    owf_reader_init(&binary->reader, alloc, owf_binary_reader_buffer_read_cb, visitor, buf);
}

const char *owf_binary_reader_strerror(owf_binary_reader_t *binary) {
    return binary->reader.error.error;
}

bool owf_binary_unwrap(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, void *ptr) {
    uint32_t length, old_length = binary->segment_length;

    /* Run the unwrap */
    if (OWF_NOEXPECT(!owf_binary_unwrap_top(binary, cb, &length, ptr))) {
        /* Just restore it */
        binary->segment_length = old_length;
        return false;
    } else {
        /* Subtract the length of what we just read, plus space for the length, and restore the modified length */
        OWF_BINARY_SAFE_SUB32(binary, old_length, length);
        binary->segment_length = old_length;
        return true;
    }
}

bool owf_binary_unwrap_top(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb, uint32_t *length_ptr, void *ptr) {
    uint32_t length;

    /* Read the length header */
    OWF_BINARY_SAFE_READ(binary, &length, sizeof(length));
    OWF_HOST32(length);

    /* Verify alignment */
    if (OWF_NOEXPECT(length % sizeof(uint32_t) != 0)) {
        OWF_READER_ERRF(binary->reader, "length was not " OWF_PRINT_SIZE "-byte aligned (got " OWF_PRINT_U32 " bytes)", sizeof(uint32_t), length);
        return false;
    }

    /* Yield to the callback, and ensure that we have no data left to read after the callback executes */
    binary->segment_length = length;
    binary->skip_length = 0;
    if (OWF_NOEXPECT(!cb(binary, ptr))) {
        return false;
    }

    /* Read skipped bytes */
    while (binary->skip_length > 0) {
        const uint32_t to_skip = OWF_MIN(binary->skip_length, sizeof(binary->skip));
        OWF_BINARY_SAFE_READ(binary, binary->skip, to_skip);
        OWF_BINARY_SAFE_SUB32(binary, binary->skip_length, to_skip);
    }

    /* Ensure we have no trailing bytes */
    if (OWF_NOEXPECT(binary->segment_length > 0)) {
        OWF_READER_ERRF(binary->reader, "trailing data when reading segment: " OWF_PRINT_U32 " bytes", binary->segment_length);
        return false;
    }

    /* Emit the total length that we just read */
    *length_ptr = length + sizeof(uint32_t);
    return true;
}

bool owf_binary_unwrap_nested_multi(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb) {
    while (binary->segment_length > 0) {
        if (OWF_NOEXPECT(!owf_binary_unwrap(binary, cb, NULL))) {
            return false;
        }
    }
    return true;
}

bool owf_binary_unwrap_multi(owf_binary_reader_t *binary, owf_binary_reader_cb_t cb) {
    while (binary->segment_length > 0) {
        if (OWF_NOEXPECT(!cb(binary, NULL))) {
            return false;
        }
    }
    return true;
}

bool owf_binary_read_str(owf_binary_reader_t *binary, void *ptr) {
    owf_str_t *str = (owf_str_t *)ptr;
    owf_str_init(str);

    /*
     * Read the actual string.
     * The string's length is currently saved in binary->segment_length if this call is wrapped.
     */
    if (OWF_NOEXPECT(!owf_str_reserve(str, binary->reader.alloc, &binary->reader.error, binary->segment_length))) {
        return false;
    } else {
        OWF_BINARY_SAFE_VARIABLE_READ(binary, str->bytes, binary->segment_length, sizeof(uint8_t), 0);
    }

    /*
     * If we got here, the variable read succeeded and we have a buffer of size
     * str->length with the string in it. Make sure it's NULL-terminated.
     */
    if (OWF_NOEXPECT(OWF_ARRAY_LEN(str->bytes) > 0 && OWF_ARRAY_GET(str->bytes, uint8_t, OWF_ARRAY_LEN(str->bytes) - 1) != 0)) {
        OWF_READER_ERR(binary->reader, "string was not NULL-terminated");
        owf_str_destroy(str, binary->reader.alloc);
        return false;
    }
    return true;
}

bool owf_binary_read_samples(owf_binary_reader_t *binary, void *ptr) {
    owf_signal_t *signal = &binary->reader.ctx.signal;
    uint32_t length = binary->segment_length;

    /* Length is stored in segment_length, ensure it's also double aligned */
    if (OWF_NOEXPECT(length % sizeof(double) != 0)) {
        OWF_READER_ERRF(binary->reader, "length of sample array is not " OWF_PRINT_SIZE "-byte aligned (got " OWF_PRINT_U32 " bytes)", sizeof(double), length);
        return false;
    }

    /* Read the double array */
    owf_array_init(&signal->samples);
    OWF_BINARY_SAFE_VARIABLE_READ(binary, signal->samples, length, sizeof(double), 0);

    /* Treat this memory as a union between a double and a uint64_t to protect strict-aliasing */
    union {
        double f64;
        uint64_t u64;
    } val;
    
    /* Byteswap the samples */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(signal->samples); i++) {
        val.u64 = OWF_ARRAY_GET(signal->samples, uint64_t, i);
        OWF_HOST64(val.u64);
        OWF_ARRAY_PUT(signal->samples, double, i, val.f64);
    }

    return true;
}

bool owf_binary_read_signal(owf_binary_reader_t *binary, void *ptr) {
    owf_signal_t *signal = &binary->reader.ctx.signal;
    owf_signal_init(signal);

    if (OWF_NOEXPECT(!owf_binary_unwrap(binary, owf_binary_read_str, &signal->id) ||
            !owf_binary_unwrap(binary, owf_binary_read_str, &signal->unit) ||
            !owf_binary_unwrap(binary, owf_binary_read_samples, NULL))) {
        return false;
    }

    /* Call the visitor */
    OWF_BINARY_READER_VISIT(binary, OWF_READ_SIGNAL);
    return true;
}

bool owf_binary_read_signals(owf_binary_reader_t *binary, void *ptr) {
    return owf_binary_unwrap_multi(binary, owf_binary_read_signal);
}

bool owf_binary_read_event(owf_binary_reader_t *binary, void *ptr) {
    owf_event_t *event = &binary->reader.ctx.event;
    owf_event_init(event);

    /* Read the timestamp */
    OWF_BINARY_SAFE_READ(binary, &event->t0, sizeof(event->t0));
    OWF_HOST64(event->t0);

    /* Ensure that the timestamp is in range */
    if (OWF_NOEXPECT(!owf_namespace_covers(&binary->reader.ctx.ns, event->t0))) {
        OWF_READER_ERRF(binary->reader, "time interval for namespace `%s` [" OWF_PRINT_TIME ", " OWF_PRINT_TIME "):" OWF_PRINT_TIME " did not cover event at " OWF_PRINT_TIME,
            OWF_STR_PTR(binary->reader.ctx.ns.id),
            binary->reader.ctx.ns.t0, binary->reader.ctx.ns.t0 + binary->reader.ctx.ns.dt, binary->reader.ctx.ns.dt, event->t0);
        return false;
    }

    /* Read the data */
    if (OWF_NOEXPECT(!owf_binary_unwrap(binary, owf_binary_read_str, &event->message))) {
        return false;
    }

    /* Call the visitor */
    OWF_BINARY_READER_VISIT(binary, OWF_READ_EVENT);
    return true;
}

bool owf_binary_read_events(owf_binary_reader_t *binary, void *ptr) {
    return owf_binary_unwrap_multi(binary, owf_binary_read_event);
}

bool owf_binary_read_alarm(owf_binary_reader_t *binary, void *ptr) {
    owf_alarm_t *alarm = &binary->reader.ctx.alarm;
    owf_alarm_init(alarm);

    /* Read the timestamp */
    OWF_BINARY_SAFE_READ(binary, &alarm->t0, sizeof(alarm->t0));
    OWF_HOST64(alarm->t0);

    /* Ensure that the timestamp is in range */
    if (OWF_NOEXPECT(!owf_namespace_covers(&binary->reader.ctx.ns, alarm->t0))) {
        OWF_READER_ERRF(binary->reader, "time interval for namespace `%s` [" OWF_PRINT_TIME ", " OWF_PRINT_TIME "):" OWF_PRINT_TIME " did not cover alarm at " OWF_PRINT_TIME,
            OWF_STR_PTR(binary->reader.ctx.ns.id),
            binary->reader.ctx.ns.t0, binary->reader.ctx.ns.t0 + binary->reader.ctx.ns.dt, binary->reader.ctx.ns.dt, alarm->t0);
        return false;
    }

    /* Read the duration */
    OWF_BINARY_SAFE_READ(binary, &alarm->dt, sizeof(alarm->dt));
    OWF_HOST64(alarm->dt);

    /* Read the level, volume, etc */
    OWF_BINARY_SAFE_READ(binary, &alarm->details, sizeof(alarm->details));

    /* Read the type */
    if (OWF_NOEXPECT(!owf_binary_unwrap(binary, owf_binary_read_str, &alarm->type))) {
        return false;
    }

    /* Read the data */
    if (OWF_NOEXPECT(!owf_binary_unwrap(binary, owf_binary_read_str, &alarm->message))) {
        return false;
    }

    /* Call the visitor */
    OWF_BINARY_READER_VISIT(binary, OWF_READ_ALARM);
    return true;
}

bool owf_binary_read_alarms(owf_binary_reader_t *binary, void *ptr) {
    return owf_binary_unwrap_multi(binary, owf_binary_read_alarm);
}

bool owf_binary_read_namespace(owf_binary_reader_t *binary, void *ptr) {
    owf_namespace_t *ns = &binary->reader.ctx.ns;
    owf_namespace_init(ns);

    /* Read timestamps */
    OWF_BINARY_SAFE_READ(binary, &ns->t0, sizeof(ns->t0));
    OWF_HOST64(ns->t0);
    OWF_BINARY_SAFE_READ(binary, &ns->dt, sizeof(ns->dt));
    OWF_HOST64(ns->dt);

    /* Read the ID */
    if (OWF_NOEXPECT(!owf_binary_unwrap(binary, owf_binary_read_str, &ns->id))) {
        return false;
    }

    /* Call the visitor */
    OWF_BINARY_READER_VISIT(binary, OWF_READ_NAMESPACE);

    /* Read children */
    return OWF_EXPECT(
        owf_binary_unwrap(binary, owf_binary_read_signals, NULL) &&
        owf_binary_unwrap(binary, owf_binary_read_events, NULL) &&
        owf_binary_unwrap(binary, owf_binary_read_alarms, NULL));
}

bool owf_binary_read_channel(owf_binary_reader_t *binary, void *ptr) {
    owf_channel_t *channel = &binary->reader.ctx.channel;
    owf_channel_init(channel);

    /* Read the channel id */
    if (OWF_NOEXPECT(!owf_binary_unwrap(binary, owf_binary_read_str, &channel->id))) {
        return false;
    }

    /* Call the visitor */
    OWF_BINARY_READER_VISIT(binary, OWF_READ_CHANNEL);

    /* Read children */
    return owf_binary_unwrap_nested_multi(binary, owf_binary_read_namespace);
}

bool owf_binary_read_channels(owf_binary_reader_t *binary, void *ptr) {
    return owf_binary_unwrap_nested_multi(binary, owf_binary_read_channel);
}

bool owf_binary_read(owf_binary_reader_t *binary) {
    owf_t *owf = &binary->reader.ctx.owf;
    uint32_t magic, length;

    /* Initialize the owf_t */
    owf_init(owf);

    /* Read the implicitly-sized header */
    binary->segment_length = sizeof(magic);
    OWF_BINARY_SAFE_READ(binary, &magic, sizeof(magic));
    OWF_HOST32(magic);

    /* Make sure that the magic is correct */
    if (OWF_NOEXPECT(magic != OWF_MAGIC)) {
        OWF_READER_ERRF(binary->reader, "invalid magic header: %#08x", magic);
        return false;
    }

    /* Reset the segment length again, and start walking the tree */
    binary->segment_length = sizeof(length);
    return owf_binary_unwrap_top(binary, owf_binary_read_channels, &length, NULL);
}

owf_t *owf_binary_materialize(owf_binary_reader_t *binary) {
    owf_reader_visit_cb_t old_cb = binary->reader.visit;
    binary->reader.visit = owf_reader_materialize_cb;
    if (OWF_NOEXPECT(!owf_binary_read(binary))) {
        return NULL;
    }
    binary->reader.visit = old_cb;
    return &binary->reader.ctx.owf;
}
