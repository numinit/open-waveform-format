#include <owf/platform.h>
#include <stdio.h>
#include <stdarg.h>

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
FILE *owf_platform_windows_fopen(const char *filename, const char *mode) {
    FILE *fp;
    errno_t err = fopen_s(&fp, filename, mode);
    if (OWF_NOEXPECT(err != 0)) {
        return NULL;
    } else {
        return fp;
    }
}

int owf_platform_windows_snprintf(char *dst, size_t size, const char *format, ...) {
    va_list va;
    int ret;
    va_start(va, format);
    ret = vsnprintf_s(dst, size, _TRUNCATE, format, va);
    va_end(va);
    return ret;
}

int owf_platform_windows_vsnprintf(char *dst, size_t size, const char *format, va_list va) {
    return vsnprintf_s(dst, size, _TRUNCATE, format, va);
}
#endif
