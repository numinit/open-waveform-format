#include <owf/version.h>

void owf_version(int *major, int *minor, int *patch) {
    *major = OWF_VERSION_MAJOR;
    *minor = OWF_VERSION_MINOR;
    *patch = OWF_VERSION_PATCH;
}

const char *owf_version_string(void) {
    return OWF_VERSION_STRING;
}
