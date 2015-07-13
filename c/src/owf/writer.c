#include <owf/writer.h>

void owf_writer_init(owf_writer_t *writer, owf_alloc_t *alloc, owf_error_t *error, owf_write_cb_t write, void *data) {
    writer->alloc = alloc;
    writer->error = error;
    writer->write = write;
    writer->data = data;

    /* Start with no error */
    writer->error->is_error = false;
}

bool owf_writer_is_error(owf_writer_t *writer) {
    return writer->error->is_error;
}

const char *owf_writer_strerror(owf_writer_t *writer) {
    return writer->error->error;
}