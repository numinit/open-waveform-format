#include <owf.h>
#include <owf/types.h>
#include <owf/reader.h>
#include <owf/reader/binary.h>
#include <owf/writer.h>
#include <owf/writer/binary.h>
#include <owf/platform.h>
#include <owf/version.h>

#include <stdio.h>
#include <errno.h>

#if OWF_PLATFORM_IS_GNU
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
#error "TODO: Build on Windows"
#endif

#define OWF_SERVER_TCP 0
#define OWF_SERVER_UDP 1

#define OWF_SERVER_MAX_CLIENT_BATCH 10
#define OWF_SERVER_LISTEN_QUEUE 8192

bool owf_server_start(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, const char *host_str, const char *protocol_str, const char *port_str);
bool owf_server_loop_tcp(FILE *restrict logger, owf_alloc_t *restrict alloc, owf_error_t *restrict err, owf_array_t *restrict arr, int sfd);
bool owf_server_loop_udp(FILE *logger, owf_alloc_t *restrict alloc, owf_error_t *restrict err, int sfd);

bool owf_setup_socket(owf_error_t *error, int fd, uint16_t mode) {
    if (fd < 0) {
        OWF_ERROR_SET(error, "invalid fd");
        return false;
    } else if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) != 0) {
        OWF_ERROR_SET(error, "setsockopt for SO_REUSEADDR failed");
        return false;
    } else if (mode == OWF_SERVER_TCP && setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) != 0) {
        OWF_ERROR_SET(error, "setsockopt for TCP_NODELAY failed");
        return false;
    }

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    unsigned long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode) == 0;
#elif OWF_PLATFORM_IS_GNU
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        OWF_ERROR_SET(error, "fcntl failed");
        return false;
    }
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) == 0;
#endif
    return true;
}

bool owf_server_start(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, const char *host_str, const char *protocol_str, const char *port_str) {
    struct addrinfo hints, *host = NULL;
    struct addrinfo *ptr = NULL;
    int err, fd = -1;
    bool ret;
    uint16_t protocol;

    // verify the protocol
    if (strcasecmp(protocol_str, "tcp") == 0) {
        protocol = OWF_SERVER_TCP;
    } else if (strcasecmp(protocol_str, "udp") == 0) {
        protocol = OWF_SERVER_UDP;
    } else {
        OWF_ERROR_SETF(error, "invalid protocol `%s`; we support tcp and udp", protocol_str);
        goto fail;
    }

    // set up the addrinfo hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = protocol == OWF_SERVER_TCP ? SOCK_STREAM : SOCK_DGRAM;

    // get address information for the requested socket
    if ((err = getaddrinfo(host_str, port_str, &hints, &host)) != 0) {
        OWF_ERROR_SETF(error, "getaddrinfo failed: %s", gai_strerror(err));
        goto fail;
    }

    // Try to bind on the first socket we can
    for (ptr = host; ptr != NULL; ptr = ptr->ai_next) {
        if ((fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == 1) {
            continue;
        } else if (!owf_setup_socket(error, fd, protocol)) {
            goto fail;
        } else if (bind(fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            // Successfully bound
            const char *type = ptr->ai_socktype == SOCK_STREAM ? "tcp" : (ptr->ai_socktype == SOCK_DGRAM ? "udp" : "<unknown>");
            const char *host = ptr->ai_canonname == NULL ? host_str : ptr->ai_canonname;
            struct sockaddr *addr = ptr->ai_addr;
            int port;

            // Make sure we bound to the right family
            if (addr->sa_family == AF_INET) {
                port = (int)ntohs(((struct sockaddr_in *)addr)->sin_port);
            } else if (addr->sa_family == AF_INET6) {
                port = (int)ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
            } else {
                OWF_ERROR_SET(error, "address family is neither AF_INET nor AF_INET6");
                goto fail;
            }

            // Try to listen
            if (listen(fd, OWF_SERVER_LISTEN_QUEUE) != 0) {
                OWF_ERROR_SET(error, "error listening");
                goto fail;
            }

            fprintf(logger, "listening on %s://%s:%d\n", type, host, port);
            break;
        } else {
            close(fd);
            fd = -1;
        }
    }

    // Did we actually bind, or did we just get to the end of the loop?
    if (ptr == NULL) {
        OWF_ERROR_SET(error, "couldn't bind");
        goto fail;
    }

    // Loop
    if (protocol == OWF_SERVER_TCP) {
        // Make an array of file descriptors
        owf_array_t fds;
        owf_array_init(&fds);

        // Accept/process new descriptors
        while (true) {
            if (!owf_server_loop_tcp(logger, alloc, error, &fds, fd)) {
                goto fail;
            }
        }

        // Destroy the array
        owf_array_destroy(&fds, alloc);
    } else {
        while (true) {
            if (!owf_server_loop_udp(logger, alloc, error, fd)) {
                goto fail;
            }
        }
    }

    goto out;

fail:
    ret = false;
out:
    if (fd != -1) {
        close(fd);
    }
    if (host != NULL) {
        freeaddrinfo(host);
    }

    return ret;
}

bool owf_server_tcp_read_cb(void *dest, const size_t size, void *data) {
    struct pollfd *pfd = (struct pollfd *)data;
    size_t bytes_left = size;
    ssize_t bytes_read;
    uint8_t *buf = (uint8_t *)dest;

    while (bytes_left > 0) {
        if ((bytes_read = read(pfd->fd, buf, bytes_left)) < 0) {
            if (errno == EINTR) {
                bytes_read = 0;
            } else if (errno == EAGAIN) {
                continue;
            } else {
                return false;
            }
        } else if (bytes_read == 0) {
            break;
        }

        bytes_left -= (size_t)bytes_read;
        buf += (size_t)bytes_read;
    }

    return true;
}

bool owf_server_tcp_write_cb(const void *src, const size_t size, void *data) {
    struct pollfd *pfd = (struct pollfd *)data;
    size_t bytes_left = size;
    ssize_t bytes_written;
    uint8_t *buf = (uint8_t *)src;

    while (bytes_left > 0) {
        if ((bytes_written = write(pfd->fd, buf, bytes_left)) <= 0) {
            if (errno == EINTR) {
                bytes_written = 0;
            } else if (errno == EAGAIN) {
                continue;
            } else {
                return false;
            }
        }

        bytes_left -= (size_t)bytes_written;
        buf += (size_t)bytes_written;
    }

    return true;
}

bool owf_server_loop_tcp(FILE *restrict logger, owf_alloc_t *alloc, owf_error_t *err, owf_array_t *arr, int sfd) {
    owf_binary_writer_t writer;
    owf_binary_reader_t reader;
    owf_error_t rw_error;
    owf_t *owf = NULL;
    struct pollfd *pfd;
    uint32_t size = 0;
    int nfds;
    
    // Try to accept new clients
    for (int i = 0; i < OWF_SERVER_MAX_CLIENT_BATCH; i++) {
        int fd;
        struct sockaddr client_addr;
        socklen_t client_len = sizeof(client_addr);

        if ((fd = accept(sfd, &client_addr, &client_len)) == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                // error accepting this client
                OWF_ERROR_SET(err, "error accepting client");
                return false;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // no more clients to accept in this batch
                break;
            }
        } else {
            // create a poll file descriptor
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN | POLLOUT;
            if (!owf_array_push(arr, alloc, err, &pfd, sizeof(pfd))) {
                return false;
            } else {
                fprintf(logger, "accepted client on fd %d\n", fd);
            }
        }
    }

    // Find sockets that we should act upon
    if ((nfds = poll(OWF_ARRAY_PTR(*arr, struct pollfd, 0), OWF_ARRAY_LEN(*arr), 1000)) == -1) {
        return false;
    } else if (nfds > 0) {
        for (uint32_t i = 0; i < OWF_ARRAY_LEN(*arr); i++) {
            pfd = OWF_ARRAY_PTR(*arr, struct pollfd, i);

            if (pfd->revents & POLLIN && !(pfd->revents & POLLHUP)) {
                // Some data is here, hopefully containing an OWF message.
                // Greedily eat the data until we can receive an OWF message.
                owf_error_init(&rw_error);
                owf_binary_reader_init(&reader, alloc, &rw_error, owf_server_tcp_read_cb, NULL, pfd);

                // materialize it
                if ((owf = owf_binary_materialize(&reader)) == NULL) {
                    fprintf(logger, "<= error materializing packet for fd %d: %s\n", pfd->fd, rw_error.error);
                    continue;
                } else {
                    // hooray, we have an OWF packet
                    if (!owf_size(owf, &rw_error, &size)) {
                        fprintf(logger, "<= error getting OWF size for fd %d's packet: %s\n", pfd->fd, rw_error.error);
                    } else {
                        fprintf(logger, "<= got a " OWF_PRINT_U32 "-byte OWF packet from fd %d\n", size, pfd->fd);
                    }
                }
            }

            if (pfd->revents & POLLOUT && !(pfd->revents & POLLHUP) && owf != NULL) {
                // initialize the binary writer
                owf_error_init(&rw_error);
                owf_binary_writer_init(&writer, alloc, &rw_error, owf_server_tcp_write_cb, pfd);

                // write to the buffer
                if (!owf_binary_write(&writer, owf)) {
                    fprintf(logger, "=> error writing packet to fd %d: %s\n", pfd->fd, rw_error.error);
                } else {
                    if (!owf_size(owf, &rw_error, &size)) {
                        fprintf(logger, "=> error getting OWF size for fd %d's packet: %s\n", pfd->fd, rw_error.error);
                    } else {
                        fprintf(logger, "=> wrote a " OWF_PRINT_U32 "-byte OWF packet to fd %d\n", size, pfd->fd);
                    }
                }
            }

            if (owf != NULL) {
                owf_destroy(owf, alloc);
            }
        }
    }

    return true;
}

bool owf_server_loop_udp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, int sfd) {
    return true;
}

int main(int argc, const char **argv) {
    owf_error_t error = OWF_ERROR_DEFAULT;
    owf_alloc_t alloc = {.malloc = malloc, .realloc = realloc, .free = free, .max_alloc = OWF_ALLOC_DEFAULT_MAX};
    char host_str[256], protocol_str[16], port_str[8];
    FILE *logger = stderr;

    if (argc != 2 || sscanf(argv[1], "%15[^:]://%255[^:]:%7[0-9]", protocol_str, host_str, port_str) != 3) {
        fprintf(stderr, "Usage: %s <port-spec>\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  $ %s tcp://127.0.0.1:1234\n", argv[0]);
        fprintf(stderr, "  $ %s udp://localhost:9001\n", argv[0]);
        return 1;
    }

    if (!owf_server_start(logger, &alloc, &error, host_str, protocol_str, port_str)) {
        fprintf(logger, "error starting server on %s://%s:%s: %s\n", protocol_str, host_str, port_str, error.error);
        return 1;
    }

    return 0;
}
