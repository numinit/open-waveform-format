#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#ifndef OWF_H
#define OWF_H

#if CHAR_MAX == INT8_MAX || CHAR_MAX == UINT8_MAX
#define OWF_INT8_BITS 8
#define OWF_CHAR_BITS 8
#define OWF_CHAR_BYTES 1
#else
#error invalid CHAR_MAX value
#endif

#if SHRT_MAX == INT16_MAX || SHRT_MAX == UINT16_MAX
#define OWF_INT16_BITS 16
#define OWF_SHORT_BITS 16
#define OWF_SHORT_BYTES 2
#else
#error invalid SHRT_MAX value
#endif

#if INT_MAX == INT32_MAX || INT_MAX == UINT32_MAX
#define OWF_INT32_BITS 32
#define OWF_INT_BITS 32
#define OWF_INT_BYTES 4
#else
#error invalid INT_MAX value
#endif

#if LLONG_MAX == INT64_MAX || LLONG_MAX == UINT64_MAX
#define OWF_INT64_BITS 64
#define OWF_LLONG_BITS 64
#define OWF_LLONG_BYTES 8
#else
#error invalid LLONG_MAX value
#endif

#if SIZE_MAX == INT32_MAX || SIZE_MAX == UINT32_MAX
#define OWF_SIZE_BITS 32
#define OWF_SIZE_BYTES 4
#elif SIZE_MAX == INT64_MAX || SIZE_MAX == UINT64_MAX
#define OWF_SIZE_BITS 64
#define OWF_SIZE_BYTES 8
#else
#error invalid SIZE_MAX value
#endif

/**
 * Token concatenation
 */
#define OWF_CONCAT2(a, b) a ## b
#define OWF_CONCAT(a, b) OWF_CONCAT2(a, b)

/**
 * Attributes
 */
#define OWF_PACK __attribute__ ((packed))
#define OWF_NORETURN __attribute__ ((noreturn))

/**
 * Branch prediction optimizations
 */
#define OWF_EXPECT(expr) (__builtin_expect((expr), true))
#define OWF_NOEXPECT(expr) (__builtin_expect((expr), false))

/**
 * Min/max
 */
#define OWF_MIN(a, b) (a < b ? a : b)
#define OWF_MAX(a, b) (a > b ? a : b)

/**
 * Array counts. From Chromium source.
 */
#define OWF_COUNT(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

typedef int64_t owf_time_t;

typedef struct owf_str {
    uint32_t length;
    void *data;
} owf_str_t;

typedef struct owf_signal {
    owf_str_t id;
    owf_str_t unit;

    double *samples;
    uint32_t num_samples;
} owf_signal_t;

typedef struct owf_event {
    owf_time_t time;
    owf_str_t data;
} owf_event_t;

typedef struct owf_alarm {
    owf_time_t time;
    owf_str_t data;
} owf_alarm_t;

typedef struct owf_namespace {
    owf_str_t id;
    owf_time_t t0, dt;

    owf_signal_t *signals;
    owf_event_t  *events;
    owf_alarm_t  *alarms;

    uint32_t num_signals, num_events, num_alarms;
} owf_namespace_t;

typedef struct owf_channel {
    owf_str_t id;
    owf_namespace_t *namespaces;
    uint32_t num_namespaces;
} owf_channel_t;

typedef struct owf {
    owf_channel_t *channels;
    uint32_t num_channels;
} owf_t;

#endif /* OWF_H */
