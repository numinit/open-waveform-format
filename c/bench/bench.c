#include <owf.h>
#include <owf/types.h>
#include <owf/reader.h>
#include <owf/reader/binary.h>
#include <owf/writer.h>
#include <owf/writer/binary.h>
#include <owf/platform.h>
#include <owf/version.h>
#include <stdio.h>
#include <unistd.h>

#include <math.h>

#if OWF_PLATFORM == OWF_PLATFORM_DARWIN
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/clock.h>

owf_time_t owf_benchmark_time_now() {
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);

    return mts.tv_sec * 10000000LL + mts.tv_nsec / 100LL;
}
#elif OWF_PLATFORM_IS_GNU
#include <time.h>
#include <sys/time.h>

owf_time_t owf_benchmark_time_now() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 10000000LL + ts.tv_nsec / 100LL;
}
#endif

typedef struct owf_bench_config {
    size_t num_messages;
    size_t channels_per_message;
    size_t namespaces_per_channel;
    size_t signals_per_namespace;
    size_t events_per_namespace;
    size_t alarms_per_namespace;
    size_t samples_per_signal;
} owf_bench_config_t;

typedef struct owf_bench_rolling_avg {
    double old_m, new_m, old_s, new_s;
    size_t num_iterations;
} owf_bench_rolling_avg_t;

void owf_bench_rolling_avg_init(owf_bench_rolling_avg_t *avg) {
    avg->old_m = avg->new_m = avg->old_s = avg->old_s = 0;
    avg->num_iterations = 0;
}

void owf_bench_rolling_avg_put(owf_bench_rolling_avg_t *avg, double d) {
    if (OWF_NOEXPECT(avg->num_iterations == 0)) {
        avg->old_m = avg->new_m = d;
        avg->old_s = avg->new_s = 0.0;
    } else {
        avg->new_m = avg->old_m + (d - avg->old_m) / (avg->num_iterations + 1);
        avg->new_s = avg->old_s + (d - avg->old_m) * (d - avg->new_m);
        avg->old_m = avg->new_m;
        avg->old_s = avg->new_s;
    }

    avg->num_iterations++;
}

double owf_bench_rolling_avg_mean(owf_bench_rolling_avg_t *avg) {
    return avg->new_m;
}

double owf_bench_rolling_avg_variance(owf_bench_rolling_avg_t *avg) {
    return avg->new_s / avg->num_iterations;
}

double owf_bench_rolling_avg_stdev(owf_bench_rolling_avg_t *avg) {
    return sqrt(owf_bench_rolling_avg_variance(avg));
}

int owf_bench_rolling_avg_print(owf_bench_rolling_avg_t *avg, FILE *fp, const char *title) {
    double mean = owf_bench_rolling_avg_mean(avg), variance = owf_bench_rolling_avg_variance(avg), stdev = owf_bench_rolling_avg_stdev(avg);
    return fprintf(fp, "===== %s: " OWF_PRINT_SIZE " %s\n"
                   "Mean: %.12f, variance: %.12f, stdev: %.12f\n"
                   "Inv. mean: %.12f, inv. stdev: %.12f\n",
                   title, avg->num_iterations, avg->num_iterations == 1 ? "iteration" : "iterations",
                   mean, variance, stdev, 1.0 / mean, 1.0 / stdev);
}

bool owf_benchmark_init_package(owf_package_t *package, owf_alloc_t *alloc, owf_error_t *error, owf_bench_config_t *config) {
    char buffer[16];

    /* Start time is now; end time is now + 1 sec.
     * This package will represent a second of samples.
     */
    const owf_time_t start_time = owf_benchmark_time_now();
    const owf_time_t end_time = start_time + 10000000LL;

    /* A halfway point between the start and end time */
    const owf_time_t half_time = start_time + 5000000LL;

    /* The wave table for the samples */
    double *wave_table = NULL;

    /* The return value */
    bool ret = true;

    /* Allocate a wave table */
    if (config->samples_per_signal > 0) {
        wave_table = owf_malloc(alloc, error, config->samples_per_signal * sizeof(double));
        if (wave_table == NULL) {
            return false;
        }

        /* Fill it with a sine wave */
        double k = (double)config->samples_per_signal;
        for (size_t i = 0; i < config->samples_per_signal; i++) {
            wave_table[i] = sin(k * i * 2 * M_PI);
        }
    }

    // Create a sample OWF packet
    owf_package_init(package);

    for (size_t i = 0; i < config->channels_per_message; i++) {
        owf_channel_t channel;
        owf_snprintf(buffer, sizeof(buffer), "C" OWF_PRINT_SIZE, i);
        if (!owf_channel_init_id(&channel, alloc, error, buffer)) {
            goto fail;
        }

        for (size_t j = 0; j < config->namespaces_per_channel; j++) {
            owf_namespace_t ns;
            owf_snprintf(buffer, sizeof(buffer), "C" OWF_PRINT_SIZE "_N" OWF_PRINT_SIZE, i, j);
            if (!owf_namespace_init_id(&ns, alloc, error, buffer)) {
                goto fail;
            } else {
                ns.t0 = start_time;
                ns.dt = end_time - start_time;
            }

            for (size_t k = 0; k < config->signals_per_namespace; k++) {
                owf_signal_t signal;
                owf_snprintf(buffer, sizeof(buffer), "C" OWF_PRINT_SIZE "_N" OWF_PRINT_SIZE "_S" OWF_PRINT_SIZE, i, j, k);
                if (!owf_signal_init_id_unit(&signal, alloc, error, buffer, "unit") ||
                    !owf_signal_push_samples(&signal, alloc, error, wave_table, (uint32_t)config->samples_per_signal) ||
                    !owf_namespace_push_signal(&ns, alloc, error, &signal)) {
                    goto fail;
                }
            }

            for (size_t k = 0; k < config->events_per_namespace; k++) {
                owf_event_t event;
                owf_snprintf(buffer, sizeof(buffer), "C" OWF_PRINT_SIZE "_N" OWF_PRINT_SIZE "_E" OWF_PRINT_SIZE, i, j, k);
                if (!owf_event_init_message(&event, alloc, error, buffer)) {
                    goto fail;
                } else {
                    /* This event occurs halfway between the start and end time */
                    event.t0 = half_time;
                }

                if (!owf_namespace_push_event(&ns, alloc, error, &event)) {
                    goto fail;
                }
            }

            for (size_t k = 0; k < config->alarms_per_namespace; k++) {
                owf_alarm_t alarm;
                owf_snprintf(buffer, sizeof(buffer), "C" OWF_PRINT_SIZE "_N" OWF_PRINT_SIZE "_A" OWF_PRINT_SIZE, i, j, k);
                if (!owf_alarm_init_type_message(&alarm, alloc, error, buffer, "42")) {
                    goto fail;
                } else {
                    alarm.t0 = half_time;
                    alarm.dt = half_time - start_time;
                    alarm.details.u8.level = 0x00;
                    alarm.details.u8.volume = 0xff;
                }

                if (!owf_namespace_push_alarm(&ns, alloc, error, &alarm)) {
                    goto fail;
                }
            }

            if (!owf_channel_push_namespace(&channel, alloc, error, &ns)) {
                goto fail;
            }
        }

        if (!owf_package_push_channel(package, alloc, error, &channel)) {
            goto fail;
        }
    }

    goto out;

fail:
    owf_package_destroy(package, alloc);
    ret = false;
out:
    owf_free(alloc, wave_table);
    return ret;
}

bool owf_benchmark_run(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, owf_package_t *package_to_encode, uint32_t size, size_t num_iterations) {
    owf_binary_writer_t writer;
    owf_binary_reader_t reader;
    owf_buffer_t buf;
    owf_bench_rolling_avg_t avg;
    owf_time_t start, end;
    owf_package_t *package_from_decode;

    void *ptr = alloca(size);
    fprintf(logger, "Benchmark started at " OWF_PRINT_TIME "\n", owf_benchmark_time_now());

    // Test encoding speed
    owf_bench_rolling_avg_init(&avg);
    for (size_t i = 0; i < num_iterations; i++) {
        start = owf_benchmark_time_now();
        {
            owf_buffer_init(&buf, ptr, size);
            owf_binary_writer_init_buffer(&writer, &buf, alloc, error);
            if (OWF_NOEXPECT(!owf_binary_write(&writer, package_to_encode))) {
                fprintf(logger, "binary write failed\n");
                return false;
            }
        }
        end = owf_benchmark_time_now();
        owf_bench_rolling_avg_put(&avg, (end - start) / 1.0e7);
    }

    owf_bench_rolling_avg_print(&avg, logger, "Encoding");

    // Test decoding speed
    owf_bench_rolling_avg_init(&avg);
    for (size_t i = 0; i < num_iterations; i++) {
        start = owf_benchmark_time_now();
        {
            owf_buffer_init(&buf, ptr, size);
            owf_binary_reader_init_buffer(&reader, &buf, alloc, error, NULL);
            package_from_decode = owf_binary_materialize(&reader);
            if (OWF_NOEXPECT(package_from_decode == NULL)) {
                fprintf(logger, "binary read failed\n");
                return false;
            } else {
                owf_package_destroy(package_from_decode, alloc);
            }
        }
        end = owf_benchmark_time_now();
        owf_bench_rolling_avg_put(&avg, (end - start) / 1.0e7);
    }

    owf_bench_rolling_avg_print(&avg, logger, "Decoding");

    return true;
}

bool owf_benchmark_start(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, owf_bench_config_t *config) {
    char buf[128];
    owf_package_t package;
    size_t num_signals, num_samples;
    uint32_t package_size;
    bool ret;

    if (config->num_messages == 0) {
        OWF_ERROR_SET(error, "invalid number of messages");
        return false;
    }

    num_signals = config->channels_per_message * config->namespaces_per_channel * config->signals_per_namespace;
    num_samples = num_signals * config->samples_per_signal;
    fprintf(logger, OWF_PRINT_SIZE " %s, " OWF_PRINT_SIZE " %s per message, " OWF_PRINT_SIZE " %s per channel,\n"
            OWF_PRINT_SIZE " %s per namespace, " OWF_PRINT_SIZE " %s per namespace, " OWF_PRINT_SIZE " %s per namespace,\n"
            OWF_PRINT_SIZE " %s per signal\n"
            "Total of " OWF_PRINT_SIZE " %s per message, for " OWF_PRINT_SIZE " %s (" OWF_PRINT_SIZE " %s) per message\n",
            config->num_messages, config->num_messages == 1 ? "message" : "messages",
            config->channels_per_message, config->channels_per_message == 1 ? "channel" : "channels",
            config->namespaces_per_channel, config->namespaces_per_channel == 1 ? "namespace" : "namespaces",
            config->signals_per_namespace, config->signals_per_namespace == 1 ? "signal" : "signals",
            config->events_per_namespace, config->events_per_namespace == 1 ? "event" : "events",
            config->alarms_per_namespace, config->alarms_per_namespace == 1 ? "alarm" : "alarms",
            config->samples_per_signal, config->samples_per_signal == 1 ? "sample" : "samples",
            num_signals, num_signals == 1 ? "signal" : "signals",
            num_samples, num_samples == 1 ? "sample" : "samples",
            num_samples * sizeof(double), "bytes");

    if (!owf_benchmark_init_package(&package, alloc, error, config)) {
        return false;
    } else if (!owf_package_size(&package, error, &package_size)) {
        ret = false;
    } else {
        owf_package_stringify(&package, buf, sizeof(buf));
        fprintf(logger, "Package %s, length: " OWF_PRINT_U32 " bytes\n", buf, package_size);
        ret = owf_benchmark_run(logger, alloc, error, &package, package_size, config->num_messages);
    }

    owf_package_destroy(&package, alloc);
    return ret;
}

int main(int argc, const char **argv) {
    owf_error_t error = OWF_ERROR_DEFAULT;
    owf_alloc_t alloc = {.malloc = malloc, .realloc = realloc, .free = free, .max_alloc = OWF_ALLOC_DEFAULT_MAX};
    owf_bench_config_t config;
    FILE *logger = stderr;

    if (argc != 8) {
        fprintf(logger, "Usage: %s <num-messages> <channels-per-message> <namespaces-per-channel> "
                "<signals-per-namespace> <events-per-namespace> <alarms-per-namespace> <samples-per-signal>\n", argv[0]);
        return 1;
    }

    config.num_messages = strtoull(argv[1], NULL, 10);
    config.channels_per_message = strtoull(argv[2], NULL, 10);
    config.namespaces_per_channel = strtoull(argv[3], NULL, 10);
    config.signals_per_namespace = strtoull(argv[4], NULL, 10);
    config.events_per_namespace = strtoull(argv[5], NULL, 10);
    config.alarms_per_namespace = strtoull(argv[6], NULL, 10);
    config.samples_per_signal = strtoull(argv[7], NULL, 10);

    fprintf(logger, "--------------------------------\n");
    fprintf(logger, "libowf %s benchmark starting\n", owf_version_string());
    fprintf(logger, "--------------------------------\n");

    if (!owf_benchmark_start(logger, &alloc, &error, &config)) {
        fprintf(logger, "error with benchmark: %s\n", owf_error_strerror(&error));
        return 1;
    }

    return 0;
}
