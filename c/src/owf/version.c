#include <owf/version.h>

void owf_version(int *major, int *minor, int *patch) {
    *major = (unsigned)(OWF_LIBRARY_VERSION & 0xf000) >> 24;
    *minor = (unsigned)(OWF_LIBRARY_VERSION & 0x0ff0) >> 8;
    *patch = (unsigned)(OWF_LIBRARY_VERSION & 0x000f);
}

uint16_t owf_version_numeric() {
    return OWF_LIBRARY_VERSION;
}

const char *owf_version_string() {
    return OWF_LIBRARY_VERSION_STRING;
}