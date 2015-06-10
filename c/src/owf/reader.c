#include <owf/reader.h>

void owf_reader_init(owf_reader_t *reader, owf_alloc_cb_t alloc_fn, owf_free_cb_t free_fn, owf_read_cb_t read_fn, owf_visit_cb_t visitor, void *data) {
    reader->alloc = alloc_fn;
    reader->free = free_fn;
    reader->read = read_fn;
    reader->visit = visitor;
    reader->data = data;
    OWF_READER_ERR(*reader, "no error");
}
