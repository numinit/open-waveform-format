#include <owf/reader.h>

void owf_reader_init(owf_reader_t *reader, owf_alloc_cb_t alloc_fn, owf_free_cb_t free_fn, owf_read_cb_t read_fn, owf_visit_cb_t visitor, size_t max_alloc, void *data) {
    reader->alloc = alloc_fn;
    reader->free = free_fn;
    reader->read = read_fn;
    reader->visit = visitor;
    reader->max_alloc = max_alloc == 0 ? OWF_READER_DEFAULT_MAX_ALLOC : max_alloc;
    reader->data = data;
    OWF_READER_ERR(*reader, "no error");
}
