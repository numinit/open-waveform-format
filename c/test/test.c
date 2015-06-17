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

#define OWF_TEST_FILE(str, result) owf_binary_test_execute("../example/owf1_" str ".owf", result)

typedef struct owf_test {
    const char *name;
    int (*fn)(void);
} owf_test_t;

static size_t owf_total_allocations = 0;
static size_t owf_total_frees = 0;
static ssize_t owf_total_allocated = 0;

static void *owf_test_malloc(size_t size) {
    void *ret = malloc(size + sizeof(size_t));
    if (ret != NULL) {
        size_t *block = ret;
        *block = size;
        owf_total_allocated += size;
        owf_total_allocations++;
    }
    return (void *)((uint8_t *)ret + sizeof(size_t));
}

static void owf_test_free(void *ptr) {
    size_t *block = (size_t *)((uint8_t *)ptr - sizeof(size_t));
    owf_total_allocated -= *block;
    owf_total_frees++;
    free(block);
}

static void *owf_test_realloc(void *ptr, size_t size) {
    size_t *old_block = (size_t *)((uint8_t *)ptr - sizeof(size_t));
    size_t old_size = *old_block;
    void *ret = realloc(old_block, size + sizeof(size_t));
    if (ret != NULL) {
        size_t *block = ret;
        *block = size;
        owf_total_allocated -= old_size;
        owf_total_allocated += size;
    }
    return (void *)((uint8_t *)ret + sizeof(size_t));
}

static void owf_test_fail(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    printf("\n");
    va_end(va);
}

static bool owf_test_visitor(owf_reader_ctx_t *ctx, owf_reader_cb_type_t type, void *data) {
    switch (type) {
        case OWF_READ_CHANNEL:
            fprintf(stderr, "[CHANNEL] %s\n", ctx->channel.id.bytes.ptr);
            break;
        case OWF_READ_NAMESPACE:
            fprintf(stderr, "  [NS] %s <t0=%" PRIu64 ", dt=%" PRIu64 ">\n", ctx->ns.id.bytes.ptr, ctx->ns.t0, ctx->ns.dt);
            break;
        case OWF_READ_SIGNAL:
            fprintf(stderr, "    [SIGNAL] %s <units=%s> [", ctx->signal.id.bytes.ptr, ctx->signal.unit.bytes.ptr);
            for (uint32_t i = 0; i < OWF_ARRAY_LEN(ctx->signal.samples); i++) {
              fprintf(stderr, "%.2f", OWF_ARRAY_GET(ctx->signal.samples, double, i));
              if (OWF_ARRAY_LEN(ctx->signal.samples) > 0 && i < OWF_ARRAY_LEN(ctx->signal.samples) - 1) {
                fprintf(stderr, " ");
              }
            }
            fprintf(stderr, "]\n");
            break;
        case OWF_READ_EVENT:
            fprintf(stderr, "    [EVENT] %s <time=%" PRIu64 ">\n", ctx->event.data.bytes.ptr, ctx->event.time);
            break;
        case OWF_READ_ALARM:
            fprintf(stderr, "    [ALARM] %s <time=%" PRIu64 ">\n", ctx->alarm.data.bytes.ptr, ctx->alarm.time);
            break;
        default:
            break;
    }
    return true;
}

static int owf_binary_test_execute(const char *filename, bool result) {
    owf_binary_reader_t reader;
    owf_alloc_t alloc = {.malloc = owf_test_malloc, .realloc = owf_test_realloc, .free = owf_test_free, .max_alloc = OWF_ALLOC_DEFAULT_MAX_ALLOC};
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        OWF_TEST_FAIL("couldn't open file");
    }

    owf_binary_reader_init_file(&reader, f, &alloc, owf_test_visitor);
    fprintf(stderr, "\n");
    if (owf_binary_read(&reader) != result) {
        OWF_TEST_FAILF("unexpected result when reading OWF: %s", owf_binary_reader_strerror(&reader));
    }
    fprintf(stderr, "** result: %s\n", owf_binary_reader_strerror(&reader));
    fclose(f);
    OWF_TEST_OK;
}

static int owf_test_binary_valid_1(void) {
    return OWF_TEST_FILE("binary_valid_1", true);
}

static int owf_test_binary_valid_2(void) {
    return OWF_TEST_FILE("binary_valid_2", true);
}

static int owf_test_binary_valid_empty(void) {
    return OWF_TEST_FILE("binary_valid_empty", true);
}

static int owf_test_binary_invalid_empty(void) {
    return OWF_TEST_FILE("binary_invalid_empty", false);
}

static int owf_test_binary_invalid_magic(void) {
    return OWF_TEST_FILE("binary_invalid_magic", false);
}

static int owf_test_binary_invalid_length_short(void) {
    return OWF_TEST_FILE("binary_invalid_length_short", false);
}

static int owf_test_binary_invalid_length_long(void) {
    return OWF_TEST_FILE("binary_invalid_length_long", false);
}

static int owf_test_binary_invalid_length_really_long(void) {
    return OWF_TEST_FILE("binary_invalid_length_really_long", false);
}

static owf_test_t tests[] = {
    {"binary_valid_1", owf_test_binary_valid_1},
    {"binary_valid_2", owf_test_binary_valid_2},
    {"binary_valid_empty", owf_test_binary_valid_empty},
    {"binary_invalid_empty", owf_test_binary_invalid_empty},
    {"binary_invalid_magic", owf_test_binary_invalid_magic},
    {"binary_invalid_length_short", owf_test_binary_invalid_length_short},
    {"binary_invalid_length_long", owf_test_binary_invalid_length_long},
    {"binary_invalid_length_really_long", owf_test_binary_invalid_length_really_long}
};

int main(int argc, char **argv) {
    char dir[1024];
    int ret = 0, success = 0, leaks = 0;

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
        size_t diff = owf_total_allocations - owf_total_frees;
        fprintf(stderr, ">> test %d/%lu (%s)...", i + 1, OWF_COUNT(tests), test->name);
        int res = test->fn();
        if (res == 0) {
            success++;
            fprintf(stderr, "<OK>\n");
        } else if (res == 1) {
            fprintf(stderr, "<SOFT FAIL>\n");
        }

        if (owf_total_allocations != owf_total_frees + diff) {
            fprintf(stderr, ">> MEMORY LEAK! Test `%s' didn't free %lu %s! (allocations: %lu, frees: %lu)\n", tests[i].name, owf_total_allocations - owf_total_frees + diff, owf_total_allocations - owf_total_frees + diff != 1 ? "objects" : "object", owf_total_allocations, owf_total_frees);
            leaks += owf_total_allocations - owf_total_frees + diff;
        }

        if (res != 0 && res != 1) {
            ret = res;
        }
    }

    // Display results
    fprintf(stderr, ">> %d/%lu %s successful, %d %s, %lu %s allocated (%.2f%%)\n", success, OWF_COUNT(tests), OWF_COUNT(tests) != 1 ? "tests" : "test", leaks, leaks != 1 ? "leaks" : "leak", owf_total_allocated, owf_total_allocated != 1 ? "bytes" : "byte", (float)success / (float)OWF_COUNT(tests) * 100);

    // Leak advice
    if (leaks > 0) {
        fprintf(stderr, ">> LEAK ALERT: There %s %d memory %s among %lu %s allocated. Fix %s!\n", leaks != 1 ? "were" : "was", leaks, leaks != 1 ? "leaks" : "leak", owf_total_allocated, owf_total_allocated != 1 ? "bytes" : "byte", leaks != 1 ? "them" : "it");
    }

    if (strcmp(argv[argc - 1], "--pause") == 0) {
        fprintf(stderr, "(press enter to exit)\n");
        getc(stdin);
    }

    return ret;
}
