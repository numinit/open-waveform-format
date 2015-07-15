#include <owf/reader.h>
#include <owf/platform.h>

void owf_reader_init(owf_reader_t *reader, owf_alloc_t *alloc, owf_error_t *error, owf_read_cb_t read, owf_visit_cb_t visitor, void *data) {
    reader->alloc = alloc;
    reader->error = error;
    reader->read = read;
    reader->visit = visitor;
    reader->data = data;
}

bool owf_reader_materialize_cb(owf_reader_t *reader, owf_reader_ctx_t *ctx, owf_reader_cb_type_t type, void *ptr) {
    owf_package_t *owf = &ctx->owf;
    owf_channel_t *channel;
    owf_namespace_t *ns;
    bool ret = true;

    switch (type) {
        case OWF_READ_CHANNEL:
            owf_array_push(&owf->channels, reader->alloc, reader->error, &ctx->channel, sizeof(owf_channel_t));
            break;
        case OWF_READ_NAMESPACE:
            channel = OWF_ARRAY_PTR(owf->channels, owf_channel_t, OWF_ARRAY_LEN(owf->channels) - 1);
            owf_array_push(&channel->namespaces, reader->alloc, reader->error, &ctx->ns, sizeof(owf_namespace_t));
            break;
        case OWF_READ_SIGNAL:
            channel = OWF_ARRAY_PTR(owf->channels, owf_channel_t, OWF_ARRAY_LEN(owf->channels) - 1);
            ns = OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, OWF_ARRAY_LEN(channel->namespaces) - 1);
            owf_array_push(&ns->signals, reader->alloc, reader->error, &ctx->signal, sizeof(owf_signal_t));
            break;
        case OWF_READ_EVENT:
            channel = OWF_ARRAY_PTR(owf->channels, owf_channel_t, OWF_ARRAY_LEN(owf->channels) - 1);
            ns = OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, OWF_ARRAY_LEN(channel->namespaces) - 1);
            owf_array_push(&ns->events, reader->alloc, reader->error, &ctx->event, sizeof(owf_event_t));
            break;
        case OWF_READ_ALARM:
            channel = OWF_ARRAY_PTR(owf->channels, owf_channel_t, OWF_ARRAY_LEN(owf->channels) - 1);
            ns = OWF_ARRAY_PTR(channel->namespaces, owf_namespace_t, OWF_ARRAY_LEN(channel->namespaces) - 1);
            owf_array_push(&ns->alarms, reader->alloc, reader->error, &ctx->alarm, sizeof(owf_alarm_t));
            break;
    }
    return ret;
}
