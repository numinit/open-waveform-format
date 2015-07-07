#include <owf/platform.h>
#include <stdio.h>
#include <stdarg.h>

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
FILE *owf_platform_windows_fopen(const char *filename, const char *mode) {
    FILE *fp;
    errno_t err = fopen_s(&fp, filename, mode);
    if (err != 0) {
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

char *owf_platform_windows_getcwd(char *buf, size_t size) {
    return _getcwd(buf, (int)size);
}
#endif
