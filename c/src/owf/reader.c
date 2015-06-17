#include <owf/reader.h>
#include <owf/platform.h>

void owf_reader_init(owf_reader_t *reader, owf_alloc_t *alloc, owf_reader_read_cb_t read, owf_reader_visit_cb_t visitor, void *data) {
    reader->alloc = alloc;
    reader->read = read;
    reader->visit = visitor;
    reader->data = data;

    /* Start with no error */
    strncpy(reader->error.error, "no error", sizeof(reader->error.error));
    reader->error.is_error = false;
}

bool owf_reader_is_error(owf_reader_t *reader) {
    return reader->error.is_error;
}

const char *owf_reader_strerror(owf_reader_t *reader) {
    return reader->error.error;
}
