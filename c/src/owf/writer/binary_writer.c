#include <owf/writer/binary.h>
#include <owf/platform.h>

#include <time.h>

#define OWF_BINARY_SAFE_WRITE(_binary, _ptr, _length) \
    do { \
        if (OWF_NOEXPECT(_length > 0 && !_binary->writer.write(_ptr, _length, _binary->writer.data))) { \
            OWF_ERROR_SETF(_binary->writer.error, "write error (" OWF_PRINT_U32 " bytes)", (uint32_t)_length); \
            return false; \
        } \
    } while (0)

void owf_binary_writer_init(owf_binary_writer_t *binary, owf_alloc_t *alloc, owf_error_t *error, owf_write_cb_t write, void *data) {
    owf_writer_init(&binary->writer, alloc, error, write, data);
}

static bool owf_binary_writer_file_write_cb(const void *src, const size_t size, void *data) {
    FILE *ptr = (FILE *)data;
    return fwrite(src, sizeof(uint8_t), size, ptr) == size;
}

void owf_binary_writer_init_file(owf_binary_writer_t *binary, FILE *file, owf_alloc_t *alloc, owf_error_t *error) {
    owf_writer_init(&binary->writer, alloc, error, owf_binary_writer_file_write_cb, file);
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

void owf_binary_writer_init_buffer(owf_binary_writer_t *binary, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error) {
    owf_writer_init(&binary->writer, alloc, error, owf_binary_writer_buffer_write_cb, buf);
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
        OWF_ERROR_SETF(binary->writer.error, "length `" OWF_PRINT_U32 "` was not a multiple of " OWF_PRINT_SIZE " bytes", length, sizeof(uint32_t));
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
    uint32_t full_size = 0;
    if (OWF_NOEXPECT(!owf_str_size(str, binary->writer.error, &full_size))) {
        return false;
    }
    
    /* Write it */
    full_size -= sizeof(uint32_t);
    if (OWF_NOEXPECT(!owf_binary_writer_write_size(binary, full_size))) {
        return false;
    }
    
    if (full_size > 0) {
        // Calculate the number of null bytes to write after the string
        uint32_t length = owf_str_length(str);

        // Write the string and a null-terminator
        OWF_BINARY_SAFE_WRITE(binary, OWF_STR_PTR(*str), length);
        owf_binary_writer_write_u8(binary, 0);
        
        // Subtract what we just wrote
        OWF_ARITH_SAFE_ADD32(binary->writer.error, length, 1);
        OWF_ARITH_SAFE_SUB32(binary->writer.error, full_size, length);
    }

    // Write the null padding
    while (full_size > 0) {
        if (OWF_NOEXPECT(!owf_binary_writer_write_u8(binary, 0))) {
            return false;
        }
        full_size--;
    }
    return true;
}

bool owf_binary_writer_write_samples(owf_binary_writer_t *binary, const double *ptr, uint32_t count) {
    /* Write the samples */
    owf_double_union_t buffer[OWF_BINARY_WRITER_BYTESWAP_BUFFER_LEN];
    register uint32_t i = count, j, stride;
    OWF_ARITH_SAFE_MUL32(binary->writer.error, i, sizeof(double));
    if (OWF_NOEXPECT(!owf_binary_writer_write_size(binary, i))) {
        return false;
    }
    
    for (i = 0; i < count; i += stride) {
        /* Calculate how many elements we are writing */
        stride = OWF_MIN(count - i, OWF_BINARY_WRITER_BYTESWAP_BUFFER_LEN);
        
        /* Byteswap the chunk */
        for (j = 0; j < stride; j++) {
            buffer[j].f64 = ptr[i + j];
            OWF_NET64(buffer[j].u64);
        }
        
        /* Bulk write the chunk to the buffer */
        OWF_BINARY_SAFE_WRITE(binary, &buffer, stride * sizeof(double));
    }
    
    return true;
}

bool owf_binary_writer_write_signal_header(owf_binary_writer_t *binary, owf_signal_t *signal) {
    /* Write the ID and units */
    if (OWF_NOEXPECT(
        !owf_binary_writer_write_str(binary, &signal->id) ||
        !owf_binary_writer_write_str(binary, &signal->unit))) {
        return false;
    }

    return true;
}

bool owf_binary_writer_write_signal(owf_binary_writer_t *binary, owf_signal_t *signal) {
    if (!owf_binary_writer_write_signal_header(binary, signal) ||
        !owf_binary_writer_write_samples(binary, OWF_ARRAY_PTR(signal->samples, double, 0), OWF_ARRAY_LEN(signal->samples))) {
        return false;
    }
    
    return true;
}

bool owf_binary_writer_write_event_header(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event) {
    if (!owf_namespace_covers(ns, event->t0)) {
        OWF_ERROR_SETF(binary->writer.error, "time interval for namespace `%s` [" OWF_PRINT_TIME ", " OWF_PRINT_TIME "):" OWF_PRINT_TIME " did not cover event at " OWF_PRINT_TIME,
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

bool owf_binary_writer_write_event(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_event_t *event) {
    return owf_binary_writer_write_event_header(binary, ns, event);
}

bool owf_binary_writer_write_alarm_header(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm) {
    if (!owf_namespace_covers(ns, alarm->t0)) {
        OWF_ERROR_SETF(binary->writer.error, "time interval for namespace `%s` [" OWF_PRINT_TIME ", " OWF_PRINT_TIME "):" OWF_PRINT_TIME " did not cover alarm at " OWF_PRINT_TIME,
            OWF_STR_PTR(ns->id), ns->t0, ns->t0 + ns->dt, ns->dt, alarm->t0);
        return false;
    }

    if (OWF_NOEXPECT(
        !owf_binary_writer_write_time(binary, alarm->t0) ||
        !owf_binary_writer_write_time(binary, alarm->dt) ||
        !owf_binary_writer_write_u8(binary, alarm->details.u8.level) ||
        !owf_binary_writer_write_u8(binary, alarm->details.u8.volume) ||
        !owf_binary_writer_write_u16(binary, 0) ||
        !owf_binary_writer_write_str(binary, &alarm->type) ||
        !owf_binary_writer_write_str(binary, &alarm->message))) {
        return false;
    }

    return true;
}

bool owf_binary_writer_write_alarm(owf_binary_writer_t *binary, owf_namespace_t *ns, owf_alarm_t *alarm) {
    return owf_binary_writer_write_alarm_header(binary, ns, alarm);
}

bool owf_binary_writer_write_namespace_header(owf_binary_writer_t *binary, owf_namespace_t *ns, uint32_t size) {
    /* Write the namespace header */
    if (OWF_NOEXPECT(
        !owf_binary_writer_write_size(binary, size - sizeof(uint32_t)) ||
        !owf_binary_writer_write_time(binary, ns->t0) ||
        !owf_binary_writer_write_time(binary, ns->dt) ||
        !owf_binary_writer_write_str(binary, &ns->id))) {
        return false;
    }
    
    return true;
}

bool owf_binary_writer_write_namespace(owf_binary_writer_t *binary, owf_namespace_t *ns) {
    uint32_t size = 0, signals_size = 0, events_size = 0, alarms_size = 0;
    
    if (OWF_NOEXPECT(
        !owf_namespace_size(ns, binary->writer.error, &size) ||
        !owf_binary_writer_write_namespace_header(binary, ns, size))) {
        return false;
    }

    /* Get the total sizes of signals, events, and alarms */
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->signals); i++) {
        owf_signal_t *signal = OWF_ARRAY_PTR(ns->signals, owf_signal_t, i);
        uint32_t signal_size = 0;
        if (OWF_NOEXPECT(!owf_signal_size(signal, binary->writer.error, &signal_size))) {
            return false;
        } else {
            OWF_ARITH_SAFE_ADD32(binary->writer.error, signals_size, signal_size);
        }
    }

    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->events); i++) {
        owf_event_t *event = OWF_ARRAY_PTR(ns->events, owf_event_t, i);
        uint32_t event_size = 0;
        if (OWF_NOEXPECT(!owf_event_size(event, binary->writer.error, &event_size))) {
            return false;
        } else {
            OWF_ARITH_SAFE_ADD32(binary->writer.error, events_size, event_size);
        }
    }

    for (uint32_t i = 0; i < OWF_ARRAY_LEN(ns->alarms); i++) {
        owf_alarm_t *alarm = OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, i);
        uint32_t alarm_size = 0;
        if (OWF_NOEXPECT(!owf_alarm_size(alarm, binary->writer.error, &alarm_size))) {
            return false;
        } else {
            OWF_ARITH_SAFE_ADD32(binary->writer.error, alarms_size, alarm_size);
        }
    }

    /* Write the signals, events, and alarms */
    if (OWF_NOEXPECT(!owf_binary_writer_write_size(binary, signals_size))) {
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

bool owf_binary_writer_write_channel_header(owf_binary_writer_t *binary, owf_channel_t *channel, uint32_t size) {
    if (OWF_NOEXPECT(
        !owf_binary_writer_write_size(binary, size - sizeof(uint32_t)) ||
        !owf_binary_writer_write_str(binary, &channel->id))) {
        return false;
    }

    return true;
}

bool owf_binary_writer_write_channel(owf_binary_writer_t *binary, owf_channel_t *channel) {
    uint32_t size;
    if (OWF_NOEXPECT(
        !owf_channel_size(channel, binary->writer.error, &size) ||
        !owf_binary_writer_write_channel_header(binary, channel, size))) {
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

bool owf_binary_write_header(owf_binary_writer_t *binary, owf_t *owf, uint32_t size) {
    if (OWF_NOEXPECT(
        !owf_binary_writer_write_u32(binary, OWF_MAGIC) ||
        !owf_binary_writer_write_size(binary, size - (sizeof(uint32_t) * 2)))) {
        return false;
    }

    return true;
}

bool owf_binary_write(owf_binary_writer_t *binary, owf_t *owf) {
    uint32_t size;
    if (OWF_NOEXPECT(
        !owf_size(owf, binary->writer.error, &size) ||
        !owf_binary_write_header(binary, owf, size))) {
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

bool owf_binary_write_buffer(owf_binary_writer_t *binary, owf_t *owf, owf_buffer_t *buf, owf_alloc_t *alloc, owf_error_t *error) {
    uint32_t size = 0;
    void *ptr;
    if (OWF_NOEXPECT(!owf_size(owf, error, &size))) {
        return false;
    } else {
        ptr = owf_malloc(alloc, error, size);
        if (OWF_NOEXPECT(ptr == NULL)) {
            return false;
        } else {
            owf_buffer_init(buf, ptr, size);
            owf_binary_writer_init_buffer(binary, buf, alloc, error);
            return owf_binary_write(binary, owf);
        }
    }
}
