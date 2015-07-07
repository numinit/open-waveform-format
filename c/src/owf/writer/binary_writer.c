#include <owf/writer/binary.h>

#define OWF_BINARY_SAFE_WRITE(binary, ptr, length) \
    do { \
        if (OWF_NOEXPECT(length > 0 && !binary->writer.write(ptr, length, binary->writer.data))) { \
            OWF_WRITER_ERRF(binary->writer, "write error (" OWF_PRINT_U32 " bytes)", (uint32_t)length); \
            return false; \
        } \
    } while (0)

void owf_binary_writer_init(owf_binary_writer_t *binary, owf_alloc_t *alloc, owf_write_cb_t write, void *data) {
    owf_writer_init(&binary->writer, alloc, write, data);
}

static bool owf_binary_writer_file_write_cb(const void *src, const size_t size, void *data) {
    FILE *ptr = (FILE *)data;
    return fwrite(src, sizeof(uint8_t), size, ptr) == size;
}

void owf_binary_writer_init_file(owf_binary_writer_t *binary, FILE *file, owf_alloc_t *alloc) {
    owf_writer_init(&binary->writer, alloc, owf_binary_writer_file_write_cb, file);
}

static bool owf_binary_writer_buffer_write_cb(const void *src, const size_t size, void *data) {
    owf_buffer_t *ptr = (owf_buffer_t *)data;
    size_t new_position = ptr->position + size;
    if (OWF_NOEXPECT(new_position > ptr->length)) {
        /* Attempted to write more bytes than this buffer has */
        return false;
    }

    /* Copy memory and update the position in the stream */
    memcpy((uint8_t *)ptr->ptr + ptr->position, src, size);
    ptr->position = new_position;
    return true;
}

void owf_binary_writer_init_buffer(owf_binary_writer_t *binary, owf_buffer_t *buf, owf_alloc_t *alloc) {
    owf_writer_init(&binary->writer, alloc, owf_binary_writer_buffer_write_cb, buf);
}

const char *owf_binary_writer_strerror(owf_binary_writer_t *binary) {
    return owf_writer_strerror(&binary->writer);
}

bool owf_binary_writer_write_double(owf_binary_writer_t *binary, double val) {
    owf_double_union_t network = {.f64 = val};
    OWF_NET64(network.u64);
    OWF_BINARY_SAFE_WRITE(binary, &network.u64, sizeof(network.u64));
    return true;
}

bool owf_binary_writer_write_time(owf_binary_writer_t *binary, owf_time_t time) {
    owf_time_t network = time;
    OWF_NET64(network);
    OWF_BINARY_SAFE_WRITE(binary, &network, sizeof(network));
    return true;
}

bool owf_binary_writer_write_size(owf_binary_writer_t *binary, uint32_t length) {
    if (length % sizeof(uint32_t) != 0) {
        OWF_WRITER_ERRF(binary->writer, "length `" OWF_PRINT_U32 "` was not a multiple of " OWF_PRINT_SIZE " bytes", length, sizeof(uint32_t));
        return false;
    }

    return owf_binary_writer_write_u32(binary, length);
}

bool owf_binary_writer_write_u32(owf_binary_writer_t *binary, uint32_t u32) {
    uint32_t network = u32;
    OWF_NET32(network);
    OWF_BINARY_SAFE_WRITE(binary, &network, sizeof(network));
    return true;
}

bool owf_binary_writer_write_u16(owf_binary_writer_t *binary, uint16_t u16) {
    uint16_t network = u16;
    OWF_NET16(network);
    OWF_BINARY_SAFE_WRITE(binary, &network, sizeof(network));
    return true;
}

bool owf_binary_writer_write_u8(owf_binary_writer_t *binary, uint8_t u8) {
    OWF_BINARY_SAFE_WRITE(binary, &u8, sizeof(u8));
    return true;
}

bool owf_binary_writer_write_str(owf_binary_writer_t *binary, owf_str_t *str) {
    /* Figure out the string's size */
    uint32_t string_size = owf_str_length(str), string_total_size = 0, null_padding = 0;
    if (!owf_str_size(str, &binary->writer.error, &string_total_size)) {
        return false;
    }

    // Calculate the number of null bytes to write after the string
    null_padding = string_total_size;
    OWF_ARITH_SAFE_SUB32(binary->writer.error, null_padding, string_size);
    OWF_ARITH_SAFE_SUB32(binary->writer.error, null_padding, sizeof(uint32_t));

    // Write the length
    if (OWF_NOEXPECT(!owf_binary_writer_write_size(binary, string_total_size))) {
        return false;
    }

    // Write the string
    OWF_BINARY_SAFE_WRITE(binary, OWF_STR_PTR(*str), string_size);

    // Write the null padding
    while (null_padding > 0) {
        if (OWF_NOEXPECT(!owf_binary_writer_write_u8(binary, 0))) {
            return false;
        }
        null_padding--;
    }
    return true;
}

bool owf_binary_writer_write_signal(owf_binary_writer_t *binary, owf_signal_t *signal) {
    uint32_t sample_size = OWF_ARRAY_LEN(signal->samples);
    OWF_ARITH_SAFE_MUL32(binary->writer.error, sample_size, sizeof(double));

    /* Write the ID and units */
    if (OWF_NOEXPECT(
        !owf_binary_writer_write_str(binary, &signal->id) ||
        !owf_binary_writer_write_str(binary, &signal->unit) ||
        !owf_binary_writer_write_size(binary, sample_size))) {
        return false;
    }

    /* Write the samples */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(signal->samples); i++) {
        if (OWF_NOEXPECT(!owf_binary_writer_write_double(binary, OWF_ARRAY_GET(signal->samples, double, i)))) {
            return false;
        }
    }

    return true;
}

bool owf_binary_writer_write_event(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event) {
    if (!owf_namespace_covers(ns, event->t0)) {
        OWF_WRITER_ERRF(binary->writer, "time interval for namespace `%s` [" OWF_PRINT_TIME ", " OWF_PRINT_TIME "):" OWF_PRINT_TIME " did not cover event at " OWF_PRINT_TIME,
            OWF_STR_PTR(ns->id), ns->t0, ns->t0 + ns->dt, ns->dt, event->t0);
        return false;
    }

    if (OWF_NOEXPECT(
        !owf_binary_writer_write_time(binary, event->t0) ||
        !owf_binary_writer_write_str(binary, &event->message))) {
        return false;
    }

    return true;
}

bool owf_binary_writer_write_alarm(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm) {
    if (!owf_namespace_covers(ns, alarm->t0)) {
        OWF_WRITER_ERRF(binary->writer, "time interval for namespace `%s` [" OWF_PRINT_TIME ", " OWF_PRINT_TIME "):" OWF_PRINT_TIME " did not cover alarm at " OWF_PRINT_TIME,
            OWF_STR_PTR(ns->id), ns->t0, ns->t0 + ns->dt, ns->dt, alarm->t0);
        return false;
    }

    if (OWF_NOEXPECT(
        !owf_binary_writer_write_time(binary, alarm->t0) ||
        !owf_binary_writer_write_time(binary, alarm->dt) ||
        !owf_binary_writer_write_u8(binary, alarm->details.level) ||
        !owf_binary_writer_write_u8(binary, alarm->details.volume) ||
        !owf_binary_writer_write_u16(binary, 0) ||
        !owf_binary_writer_write_str(binary, &alarm->type) ||
        !owf_binary_writer_write_str(binary, &alarm->message))) {
        return false;
    }

    return true;
}

bool owf_binary_writer_write_namespace(owf_binary_writer_t *binary, owf_namespace_t *ns) {
    uint32_t size = 0, signals_size = 0, events_size = 0, alarms_size = 0;

    /* Get the total sizes of signals, events, and alarms */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
        owf_signal_t *signal = OWF_ARRAY_PTR(ns->signals, owf_signal_t, i);
        uint32_t signal_size = 0;
        if (OWF_NOEXPECT(!owf_signal_size(signal, &binary->writer.error, &signal_size))) {
            return false;
        } else {
            OWF_ARITH_SAFE_ADD32(binary->writer.error, signals_size, signal_size);
        }
    }

    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
        owf_event_t *event = OWF_ARRAY_PTR(ns->events, owf_event_t, i);
        uint32_t event_size = 0;
        if (OWF_NOEXPECT(!owf_event_size(event, &binary->writer.error, &event_size))) {
            return false;
        } else {
            OWF_ARITH_SAFE_ADD32(binary->writer.error, events_size, event_size);
        }
    }

    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
        owf_alarm_t *alarm = OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i);
        uint32_t alarm_size = 0;
        if (OWF_NOEXPECT(!owf_alarm_size(alarm, &binary->writer.error, &alarm_size))) {
            return false;
        } else {
            OWF_ARITH_SAFE_ADD32(binary->writer.error, alarms_size, alarm_size);
        }
    }
    
    /* Write the namespace header */
    if (OWF_NOEXPECT(
        !owf_namespace_size(ns, &binary->writer.error, &size) ||
        !owf_binary_writer_write_size(binary, size) ||
        !owf_binary_writer_write_time(binary, ns->t0) ||
        !owf_binary_writer_write_time(binary, ns->dt) ||
        !owf_binary_writer_write_str(binary, &ns->id))) {
        return false;
    }

    /* Write the signals, events, and alarms */
    if (!owf_binary_writer_write_size(binary, signals_size)) {
        return false;
    } else {
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
            if (OWF_NOEXPECT(!owf_binary_writer_write_signal(binary, OWF_ARRAY_PTR(ns->signals, owf_signal_t, i)))) {
                return false;
            }
        }
    }

    if (OWF_NOEXPECT(!owf_binary_writer_write_size(binary, events_size))) {
        return false;
    } else {
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
            if (OWF_NOEXPECT(!owf_binary_writer_write_event(binary, ns, OWF_ARRAY_PTR(ns->events, owf_event_t, i)))) {
                return false;
            }
        }
    }

    if (OWF_NOEXPECT(!owf_binary_writer_write_size(binary, alarms_size))) {
        return false;
    } else {
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
            if (OWF_NOEXPECT(!owf_binary_writer_write_alarm(binary, ns, OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i)))) {
                return false;
            }
        }
    }

    return true;
}

bool owf_binary_writer_write_channel(owf_binary_writer_t *binary, owf_channel_t *channel) {
    uint32_t size = 0;
    if (OWF_NOEXPECT(
        !owf_channel_size(channel, &binary->writer.error, &size) ||
        !owf_binary_writer_write_size(binary, size) ||
        !owf_binary_writer_write_str(binary, &channel->id))) {
        return false;
    }

    /* Write each namespace */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(channel->namespaces); i++) {
        if (OWF_NOEXPECT(!owf_binary_writer_write_namespace(binary, OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, i)))) {
            return false;
        }
    }

    return true;
}

bool owf_binary_write(owf_binary_writer_t *binary, owf_t *owf) {
    uint32_t size = 0;
    if (OWF_NOEXPECT(
        !owf_binary_writer_write_u32(binary, OWF_MAGIC) ||
        !owf_size(owf, &binary->writer.error, &size) ||
        !owf_binary_writer_write_size(binary, size))) {
        return false;
    }

    /* Write each channel */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(owf->channels); i++) {
        if (OWF_NOEXPECT(!owf_binary_writer_write_channel(binary, OWF_ARRAY_PTR(owf->channels, owf_channel_t, i)))) {
            return false;
        }
    }

    return true;
}
