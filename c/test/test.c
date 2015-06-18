#include <owf.h>
#include <owf/types.h>
#include <owf/binary.h>
#include <owf/reader.h>
#include <owf/platform.h>
#include <owf/version.h>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define OWF_TEST_SOFT_FAIL(str) {owf_test_fail(str); return 1;}
#define OWF_TEST_SOFT_FAILF(str, ...) {owf_test_fail(str, __VA_ARGS__); return 1;}
#define OWF_TEST_FAIL(str) {owf_test_fail(str); return 2;}
#define OWF_TEST_FAILF(str, ...) {owf_test_fail(str, __VA_ARGS__); return 2;}
#define OWF_TEST_OK return 0
#define OWF_TEST_PATH_TO(file) ("../example/owf1_" file ".owf")

#define OWF_TEST_VISITOR(str, result) owf_test_binary_visitor_execute(OWF_TEST_PATH_TO(str), result)
#define OWF_TEST_MATERIALIZE(str) owf_test_binary_materialize_execute(OWF_TEST_PATH_TO(str))

typedef struct owf_test {
    const char *name;
    int (*fn)(void);
} owf_test_t;

static void owf_test_fail(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    printf("\n");
    va_end(va);
}

static void owf_test_print_channel(owf_channel_t *channel) {
    fprintf(stderr, "[CHANNEL] %s\n", channel->id.bytes.ptr);
}

static void owf_test_print_namespace(owf_namespace_t *namespace) {
    fprintf(stderr, "  [NS] %s <t0=%" PRIu64 ", dt=%" PRIu64 ">\n", namespace->id.bytes.ptr, namespace->t0, namespace->dt);
}

static void owf_test_print_signal(owf_signal_t *signal) {
    fprintf(stderr, "    [SIGNAL] %s <units=%s> [", signal->id.bytes.ptr, signal->unit.bytes.ptr);
    for (uint32_t i = 0; i < OWF_ARRAY_LEN(signal->samples); i++) {
        double d = OWF_ARRAY_GET(signal->samples, double, i);
        if (isfinite(d)) {
            fprintf(stderr, "\"%.2f\"", d);
        } else if (isnan(d)) {
            fprintf(stderr, "\"NaN\"");
        } else if (d < 0) {
            fprintf(stderr, "\"-Infinity\"");
        } else {
            fprintf(stderr, "\"Infinity\"");
        }
        if (OWF_ARRAY_LEN(signal->samples) > 0 && i < OWF_ARRAY_LEN(signal->samples) - 1) {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "]\n");
}

static void owf_test_print_event(owf_event_t *event) {
    fprintf(stderr, "    [EVENT] %s <time=%" PRIu64 ">\n", event->data.bytes.ptr, event->time);
}

static void owf_test_print_alarm(owf_alarm_t *alarm) {
    fprintf(stderr, "    [ALARM] %s <time=%" PRIu64 ">\n", alarm->data.bytes.ptr, alarm->time);
}

static bool owf_test_visitor(owf_reader_t *reader, owf_reader_ctx_t *ctx, owf_reader_cb_type_t type, void *data) {
    switch (type) {
        case OWF_READ_CHANNEL:
            owf_test_print_channel(&ctx->channel);
            owf_channel_destroy(&ctx->channel, reader->alloc);
            break;
        case OWF_READ_NAMESPACE:
            owf_test_print_namespace(&ctx->ns);
            owf_namespace_destroy(&ctx->ns, reader->alloc);
            break;
        case OWF_READ_SIGNAL:
            owf_test_print_signal(&ctx->signal);
            owf_signal_destroy(&ctx->signal, reader->alloc);
            break;
        case OWF_READ_EVENT:
            owf_test_print_event(&ctx->event);
            owf_event_destroy(&ctx->event, reader->alloc);
            break;
        case OWF_READ_ALARM:
            owf_test_print_alarm(&ctx->alarm);
            owf_alarm_destroy(&ctx->alarm, reader->alloc);
            break;
        default:
            break;
    }
    return true;
}

static bool owf_test_binary_open(owf_binary_reader_t *reader, const char *filename, owf_alloc_t *alloc, owf_reader_visit_cb_t visitor) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        return false;
    }
    owf_binary_reader_init_file(reader, f, alloc, visitor);
    return true;
}

static void owf_test_binary_close(owf_binary_reader_t *reader) {
    fclose((FILE *)reader->reader.data);
}

static int owf_test_binary_visitor_execute(const char *filename, bool result) {
    owf_alloc_t alloc = {.malloc = malloc, .realloc = realloc, .free = free, .max_alloc = OWF_ALLOC_DEFAULT_MAX};
    owf_binary_reader_t reader;

    if (!owf_test_binary_open(&reader, filename, &alloc, owf_test_visitor)) {
        OWF_TEST_FAIL("error opening file");
    }

    fprintf(stderr, "\n");
    if (owf_binary_read(&reader) != result) {
        OWF_TEST_FAILF("unexpected result when reading OWF: %s", owf_binary_reader_strerror(&reader));
    }
    fprintf(stderr, "** result: %s\n", owf_binary_reader_strerror(&reader));
    owf_test_binary_close(&reader);
    OWF_TEST_OK;
}

static int owf_test_binary_materialize_execute(const char *filename) {
    owf_alloc_t alloc = {.malloc = malloc, .realloc = realloc, .free = free, .max_alloc = OWF_ALLOC_DEFAULT_MAX};
    owf_binary_reader_t reader;
    owf_t *owf;

    if (!owf_test_binary_open(&reader, filename, &alloc, NULL)) {
        OWF_TEST_FAIL("error opening file");
    }
    owf = owf_binary_materialize(&reader);
    if (owf == NULL) {
        OWF_TEST_FAILF("unexpected result when reading OWF: %s", owf_binary_reader_strerror(&reader));
    }

    fprintf(stderr, "\n");
    for (size_t channel_idx = 0; channel_idx < OWF_ARRAY_LEN(owf->channels); channel_idx++) {
        owf_channel_t *channel = OWF_ARRAY_PTR(owf->channels, owf_channel_t, channel_idx);
        owf_test_print_channel(channel);
        for (size_t ns_idx = 0; ns_idx < OWF_ARRAY_LEN(channel->namespaces); ns_idx++) {
            owf_namespace_t *ns = OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, ns_idx);
            owf_test_print_namespace(ns);
            for (size_t sig_idx = 0; sig_idx < OWF_ARRAY_LEN(ns->signals); sig_idx++) {
                owf_signal_t *sig = OWF_ARRAY_PTR(ns->signals, owf_signal_t, sig_idx);
                owf_test_print_signal(sig);
            }

            for (size_t event_idx = 0; event_idx < OWF_ARRAY_LEN(ns->events); event_idx++) {
                owf_event_t *event = OWF_ARRAY_PTR(ns->events, owf_event_t, event_idx);
                owf_test_print_event(event);
            }

            for (size_t alarm_idx = 0; alarm_idx < OWF_ARRAY_LEN(ns->alarms); alarm_idx++) {
                owf_alarm_t *alarm = OWF_ARRAY_PTR(ns->alarms, owf_alarm_t, alarm_idx);
                owf_test_print_alarm(alarm);
            }
        }
    }

    owf_destroy(owf, &alloc);
    fprintf(stderr, "** result: %s\n", owf_binary_reader_strerror(&reader));
    owf_test_binary_close(&reader);
    OWF_TEST_OK;
}

static int owf_test_binary_valid_1(void) {
    return OWF_TEST_VISITOR("binary_valid_1", true);
}

static int owf_test_binary_valid_2(void) {
    return OWF_TEST_VISITOR("binary_valid_2", true);
}

static int owf_test_binary_valid_empty(void) {
    return OWF_TEST_VISITOR("binary_valid_empty", true);
}

static int owf_test_binary_invalid_empty(void) {
    return OWF_TEST_VISITOR("binary_invalid_empty", false);
}

static int owf_test_binary_invalid_magic(void) {
    return OWF_TEST_VISITOR("binary_invalid_magic", false);
}

static int owf_test_binary_invalid_length_short(void) {
    return OWF_TEST_VISITOR("binary_invalid_length_short", false);
}

static int owf_test_binary_invalid_length_long(void) {
    return OWF_TEST_VISITOR("binary_invalid_length_long", false);
}

static int owf_test_binary_invalid_length_really_long(void) {
    return OWF_TEST_VISITOR("binary_invalid_length_really_long", false);
}

static int owf_test_binary_materialize_1(void) {
    return OWF_TEST_MATERIALIZE("binary_valid_1");
}

static int owf_test_binary_materialize_2(void) {
    return OWF_TEST_MATERIALIZE("binary_valid_2");
}

static owf_test_t tests[] = {
    {"binary_valid_1", owf_test_binary_valid_1},
    {"binary_valid_2", owf_test_binary_valid_2},
    {"binary_valid_empty", owf_test_binary_valid_empty},
    {"binary_invalid_empty", owf_test_binary_invalid_empty},
    {"binary_invalid_magic", owf_test_binary_invalid_magic},
    {"binary_invalid_length_short", owf_test_binary_invalid_length_short},
    {"binary_invalid_length_long", owf_test_binary_invalid_length_long},
    {"binary_invalid_length_really_long", owf_test_binary_invalid_length_really_long},
    {"binary_materialize_1", owf_test_binary_materialize_1},
    {"binary_materialize_2", owf_test_binary_materialize_2}
};

int main(int argc, char **argv) {
    char dir[1024];
    int ret = 0, success = 0;

    // Disable output buffering
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // And here we go...
    getcwd(dir, sizeof(dir));
    fprintf(stderr, "--------------------------------\n");
    fprintf(stderr, "libowf " OWF_LIBRARY_VERSION_STRING " test harness starting\n");
    fprintf(stderr, "--------------------------------\n");
    fprintf(stderr, "(in %s)\n\n", dir);
    fprintf(stderr, ">> running %lu %s\n", OWF_COUNT(tests), OWF_COUNT(tests) == 1 ? "test" : "tests");
    for (int i = 0; i < OWF_COUNT(tests); i++) {
        owf_test_t *test = &tests[i];
        fprintf(stderr, ">> test %d/%lu (%s)...", i + 1, OWF_COUNT(tests), test->name);
        int res = test->fn();
        if (res == 0) {
            success++;
            fprintf(stderr, "PASSED\n");
        } else if (res == 1) {
            fprintf(stderr, "FAILED\n");
        }

        if (res != 0 && res != 1) {
            ret = res;
        }

        fprintf(stderr, "--------------------------------\n");
    }

    // Display results
    fprintf(stderr, ">> %d/%lu %s successful (%.2f%%)\n", success, OWF_COUNT(tests), OWF_COUNT(tests) != 1 ? "tests" : "test", (float)success / (float)OWF_COUNT(tests) * 100);

    if (strcmp(argv[argc - 1], "--pause") == 0) {
        fprintf(stderr, "(press enter to exit)\n");
        getc(stdin);
    }

    return ret;
}
