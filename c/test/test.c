#include <owf.h>
#include <owf/binary.h>
#include <owf/reader.h>
#include <owf/platform.h>
#include <owf/version.h>

#include <stdio.h>
#include <stdarg.h>

#define OWF_TEST_SOFT_FAIL(str) {owf_test_fail(str); return 1;}
#define OWF_TEST_SOFT_FAILF(str, ...) {owf_test_fail(str, __VA_ARGS__); return 1;}
#define OWF_TEST_FAIL(str) {owf_test_fail(str); return 2;}
#define OWF_TEST_FAILF(str, ...) {owf_test_fail(str, __VA_ARGS__); return 2;}
#define OWF_TEST_OK return 0

void owf_test_fail(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    putc(' ', stderr);
    vfprintf(stderr, fmt, va);
    printf("\n");
    va_end(va);
}

typedef struct owf_test {
    const char *name;
    int (*fn)(void);
} owf_test_t;

static bool owf_visitor(owf_reader_ctx_t *ctx, owf_reader_cb_type_t type, void *data) {
    switch (type) {
        case OWF_READ_CHANNEL:
            fprintf(stderr, "[CHANNEL] %s\n", ctx->channel.id.data);
            break;
        case OWF_READ_NAMESPACE:
            fprintf(stderr, "  [NS] %s <t0=%" PRIu64 ", dt=%" PRIu64 ">\n", ctx->ns.id.data, ctx->ns.t0, ctx->ns.dt);
            break;
        case OWF_READ_SIGNAL:
            fprintf(stderr, "    [SIGNAL] %s <units=%s>: [", ctx->signal.id.data, ctx->signal.unit.data);
            for (uint32_t i = 0; i < ctx->signal.num_samples; i++) {
                fprintf(stderr, "%.2f", ctx->signal.samples[i]);
                if (ctx->signal.num_samples > 0 && i < ctx->signal.num_samples - 1) {
                    fprintf(stderr, " ");
                }
            }
            fprintf(stderr, "]\n");
            break;
        case OWF_READ_EVENT:
            fprintf(stderr, "    [EVENT] %s <time=%" PRIu64 ">\n", ctx->event.data.data, ctx->event.time);
            break;
        case OWF_READ_ALARM:
            fprintf(stderr, "    [ALARM] %s <time=%" PRIu64 ">\n", ctx->alarm.data.data, ctx->alarm.time);
            break;
        default:
            break;
    }
    return true;
}

static int owf_test_example_1_binary(void) {
    owf_binary_reader_t reader;
    FILE *f = fopen("test/example/owf1_example_1.owf", "rb");
	if (f == NULL) {
		OWF_TEST_FAIL("couldn't open file");
	}

    owf_binary_reader_init_file(&reader, f, malloc, free, owf_visitor, OWF_READER_DEFAULT_MAX_ALLOC);
    fprintf(stderr, "\n");
    if (!owf_binary_read(&reader)) {
        OWF_TEST_FAILF("unexpected error when reading OWF: %s", owf_binary_reader_strerror(&reader));
    }

    owf_binary_reader_destroy_file(&reader);
    fclose(f);
    OWF_TEST_OK;
}

static int owf_test_example_2_binary(void) {
    owf_binary_reader_t reader;
    FILE *f = fopen("test/example/owf1_example_2.owf", "rb");
	if (f == NULL) {
		OWF_TEST_FAIL("couldn't open file");
	}

    owf_binary_reader_init_file(&reader, f, malloc, free, owf_visitor, OWF_READER_DEFAULT_MAX_ALLOC);
	fprintf(stderr, "\n");
    if (!owf_binary_read(&reader)) {
        OWF_TEST_FAILF("unexpected error when reading OWF: %s", owf_binary_reader_strerror(&reader));
    }

    owf_binary_reader_destroy_file(&reader);
    fclose(f);
    OWF_TEST_OK;
}

static owf_test_t tests[] = {
    {"example_1_binary", owf_test_example_1_binary},
    {"example_2_binary", owf_test_example_2_binary}
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
            fprintf(stderr, " <OK>\n");
        } else if (res == 1) {
            fprintf(stderr, " <SOFT FAIL>\n");
        }

        if (res != 0 && res != 1) {
            ret = res;
        }
    }

    // Display results
    fprintf(stderr, ">> %d/%lu %s successful (%.2f%%)\n", success, OWF_COUNT(tests), OWF_COUNT(tests) != 1 ? "tests" : "test", (float)success / (float)OWF_COUNT(tests) * 100);

	if (strcmp(argv[argc - 1], "--pause") == 0) {
		fprintf(stderr, "(press enter to exit)\n");
		getc(stdin);
	}

    return ret;
}
