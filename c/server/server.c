#include <owf.h>
#include <owf/types.h>
#include <owf/reader.h>
#include <owf/reader/binary.h>
#include <owf/writer.h>
#include <owf/writer/binary.h>
#include <owf/platform.h>
#include <owf/version.h>
#include <errno.h>

#include <stdio.h>

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
    #define owf_socket_close(...) close(__VA_ARGS__)
    #define owf_socket_poll(...) poll(__VA_ARGS__)
    #define OWF_SOCKET_ERROR errno
    #define OWF_SOCKET_EAGAIN EAGAIN
    #define OWF_SOCKET_EINTR EINTR
    #define OWF_SOCKET_EWOULDBLOCK EWOULDBLOCK
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define owf_socket_close(...) closesocket(__VA_ARGS__)
    #define owf_socket_poll(...) WSAPoll(__VA_ARGS__)
    #define OWF_SOCKET_ERROR WSAGetLastError()
    #define OWF_SOCKET_EAGAIN EAGAIN
    #define OWF_SOCKET_EINTR WSAEINTR
    #define OWF_SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#endif

#define OWF_SERVER_TCP 0
#define OWF_SERVER_UDP 1

#define OWF_SERVER_UDP_BUFFER_SIZE 1048576
#define OWF_SERVER_MAX_CLIENT_BATCH 10
#define OWF_SERVER_LISTEN_QUEUE 8192

bool owf_server_start(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, const char *host_str, const char *protocol_str, const char *port_str);
bool owf_server_loop_tcp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, struct pollfd *pfd, int sfd);
bool owf_server_loop_udp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, uint8_t *buffer, int sfd);

bool owf_setup_socket(owf_error_t *error, int fd, uint16_t mode) {
    if (fd < 0) {
        OWF_ERROR_SET(error, "invalid fd");
        return false;
    }
#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(char){ 1 }, sizeof(int)) != 0) {
        OWF_ERROR_SET(error, "setsockopt for SO_REUSEADDR failed: %d", OWF_SOCKET_ERROR);
        return false;
    } else if (mode == OWF_SERVER_TCP && setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &(char){ 1 }, sizeof(int)) != 0) {
        OWF_ERROR_SET(error, "setsockopt for TCP_NODELAY failed: %d", OWF_SOCKET_ERROR);
        return false;
    }
#elif OWF_PLATFORM_IS_GNU
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) != 0) {
        OWF_ERROR_SET(error, "setsockopt for SO_REUSEADDR failed");
        return false;
    } else if (mode == OWF_SERVER_TCP && setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) != 0) {
        OWF_ERROR_SET(error, "setsockopt for TCP_NODELAY failed");
        return false;
    }
#endif

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    unsigned long flags = 1;
    return ioctlsocket(fd, FIONBIO, &flags) == 0;
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
#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    WSADATA ws;
#endif
    owf_array_t fds;
    struct addrinfo hints, *host = NULL;
    struct addrinfo *ptr = NULL;
    uint8_t *buffer = NULL;
    int err = 0, fd = -1, nfds = 0;
    bool ret = true;
    uint16_t protocol;

    // verify the protocol
    if (owf_strcasecmp(protocol_str, "tcp") == 0) {
        protocol = OWF_SERVER_TCP;
    } else if (owf_strcasecmp(protocol_str, "udp") == 0) {
        protocol = OWF_SERVER_UDP;
    } else {
        OWF_ERROR_SETF(error, "invalid protocol `%s`; we support tcp and udp", protocol_str);
        goto fail;
    }

    // set up the addrinfo hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = protocol == OWF_SERVER_TCP ? SOCK_STREAM : SOCK_DGRAM;

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    // init winsock
    if (WSAStartup(MAKEWORD(2, 0), &ws) != 0) {
        OWF_ERROR_SETF(error, "WSAStartup failed");
        goto fail;
    }
#endif
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
            const char *type = ptr->ai_socktype == SOCK_STREAM ? "tcp" : (ptr->ai_socktype == SOCK_DGRAM ? "udp" : NULL);
            const char *host = ptr->ai_canonname == NULL ? host_str : ptr->ai_canonname;
            struct sockaddr *addr = ptr->ai_addr;
            int port;
            
            // Make sure we're a TCP or UDP socket
            if (type == NULL) {
                OWF_ERROR_SET(error, "type is neither SOCK_STREAM not SOCK_DGRAM");
                goto fail;
            }
            
            // Try to listen
            if (ptr->ai_socktype == SOCK_STREAM && listen(fd, OWF_SERVER_LISTEN_QUEUE) != 0) {
                OWF_ERROR_SET(error, "error listening: %d", OWF_SOCKET_ERROR);
                goto fail;
            }

            // Make sure we bound to the right family
            if (addr->sa_family == AF_INET) {
                port = (int)ntohs(((struct sockaddr_in *)addr)->sin_port);
                fprintf(logger, "bound to socket on %s://%s:%d\n", type, host, port);
            } else if (addr->sa_family == AF_INET6) {
                port = (int)ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
                fprintf(logger, "bound to IPv6 socket on %s://%s:%d\n", type, host, port);
            } else {
                OWF_ERROR_SET(error, "address family is neither AF_INET nor AF_INET6");
                goto fail;
            }

            break;
        } else {
            owf_socket_close(fd);
            fd = -1;
        }
    }

    // Did we actually bind, or did we just get to the end of the loop?
    if (ptr == NULL) {
        OWF_ERROR_SET(error, "couldn't bind");
        goto fail;
    }
    
    // Init an array of file descriptors
    owf_array_init(&fds);

    // Loop
    if (protocol == OWF_SERVER_TCP) {
        while (true) {
            // Try to accept new clients
            for (int i = 0; i < OWF_SERVER_MAX_CLIENT_BATCH; i++) {
                int cfd;
                struct sockaddr client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                if ((cfd = accept(fd, &client_addr, &client_len)) == -1) {
                    if (OWF_SOCKET_ERROR != OWF_SOCKET_EAGAIN && OWF_SOCKET_ERROR != OWF_SOCKET_EWOULDBLOCK) {
                        // error accepting this client
                        OWF_ERROR_SETF(error, "error accepting client: %d", OWF_SOCKET_ERROR);
                        goto fail;
                    } else if (OWF_SOCKET_ERROR == OWF_SOCKET_EAGAIN || OWF_SOCKET_ERROR == OWF_SOCKET_EWOULDBLOCK) {
                        // no more clients to accept in this batch
                        break;
                    }
                } else {
                    // create a poll file descriptor
                    struct pollfd pfd;
                    pfd.fd = cfd;
                    pfd.events = POLLIN | POLLOUT;
                    if (!owf_array_push(&fds, alloc, error, &pfd, sizeof(pfd))) {
                        goto fail;
                    } else {
                        fprintf(logger, "accepted client on fd %d\n", fd);
                    }
                }
            }
            
            // Find sockets that we should act upon
            if (OWF_ARRAY_LEN(fds) > 0 && (nfds = owf_socket_poll(OWF_ARRAY_PTR(fds, struct pollfd, 0), OWF_ARRAY_LEN(fds), 100)) == -1) {
                OWF_ERROR_SETF(error, "poll error: %d", OWF_SOCKET_ERROR);
                return false;
            } else if (OWF_ARRAY_LEN(fds) > 0 && nfds > 0) {
                for (uint32_t i = 0; i < OWF_ARRAY_LEN(fds); i++) {
                    // Process existing clients
                    if (!owf_server_loop_tcp(logger, alloc, error, OWF_ARRAY_PTR(fds, struct pollfd, i), fd)) {
                        goto fail;
                    }
                }
            }
            
            //usleep(10000);
        }
    } else {
        // Allocate a buffer
        buffer = owf_malloc(alloc, error, OWF_SERVER_UDP_BUFFER_SIZE);
        if (buffer == NULL) {
            goto fail;
        }

        while (true) {
            // Process existing clients
            if (!owf_server_loop_udp(logger, alloc, error, buffer, fd)) {
                goto fail;
            }
            
           // usleep(10000);
        }
    }
    goto out;

fail:
    ret = false;

out:
    // Destroy the array
    owf_array_destroy(&fds, alloc);
    
    if (fd != -1) {
        owf_socket_close(fd);
    }
    if (host != NULL) {
        freeaddrinfo(host);
    }

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    // Clean up winsock
    WSACleanup();
#endif

    return ret;
}

bool owf_server_tcp_read_cb(void *dest, const size_t size, void *data) {
    struct pollfd *pfd = (struct pollfd *)data;
    size_t bytes_left = size;
    ssize_t bytes_read;
    uint8_t *buf = (uint8_t *)dest;

    while (bytes_left > 0) {
        if ((bytes_read = recv(pfd->fd, buf, bytes_left, 0)) < 0) {
            if (OWF_SOCKET_ERROR == OWF_SOCKET_EINTR) {
                bytes_read = 0;
            } else if (OWF_SOCKET_ERROR == OWF_SOCKET_EWOULDBLOCK) {
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
        if ((bytes_written = send(pfd->fd, buf, bytes_left, 0)) <= 0) {
            if (OWF_SOCKET_ERROR == OWF_SOCKET_EINTR) {
                bytes_written = 0;
            } else if (OWF_SOCKET_ERROR == OWF_SOCKET_EAGAIN) {
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

bool owf_server_loop_tcp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, struct pollfd *pfd, int sfd) {
    owf_binary_reader_t reader;
    owf_binary_writer_t writer;
    owf_error_t rw_error;
    owf_t *owf = NULL;
    uint32_t size = 0;

    if (pfd->revents & POLLIN && !(pfd->revents & POLLHUP)) {
        // Some data is here, hopefully containing an OWF message.
        // Greedily eat the data until we can receive an OWF message.
        owf_error_init(&rw_error);
        owf_binary_reader_init(&reader, alloc, &rw_error, owf_server_tcp_read_cb, NULL, pfd);

        if ((owf = owf_binary_materialize(&reader)) == NULL) {
            fprintf(logger, "<= error materializing packet for fd %d: %s\n", pfd->fd, rw_error.error);
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

    return true;
}

bool owf_server_loop_udp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, uint8_t *buffer, int sfd) {
    owf_binary_reader_t reader;
    owf_binary_writer_t writer;
    owf_buffer_t input_buffer, output_buffer;
    owf_error_t rw_error;
    
    struct sockaddr client_addr;
    socklen_t client_len = sizeof(client_addr);
    owf_t *owf = NULL;
    ssize_t len;
    uint32_t tmp;
    
    // Read the packet
    if ((len = recvfrom(sfd, buffer, sizeof(buffer), 0, &client_addr, &client_len)) > 0) {
        // We've got the UDP packet in a buffer. Read it.
        owf_error_init(&rw_error);
        owf_buffer_init(&input_buffer, buffer, (size_t)len);
        owf_binary_reader_init_buffer(&reader, &input_buffer, alloc, &rw_error, NULL);
        
        if ((owf = owf_binary_materialize(&reader)) == NULL) {
            fprintf(logger, "<= error materializing packet: %s\n", rw_error.error);
        } else {
            if (!owf_size(owf, &rw_error, &tmp)) {
                fprintf(logger, "<= error getting OWF size packet: %s\n", rw_error.error);
            } else {
                fprintf(logger, "<= got a " OWF_PRINT_U32 "-byte OWF packet\n", tmp);
            }
        }
    } else if (OWF_SOCKET_ERROR != OWF_SOCKET_EAGAIN && OWF_SOCKET_ERROR != OWF_SOCKET_EWOULDBLOCK) {
        fprintf(logger, "recvfrom error: %d\n", OWF_SOCKET_ERROR);
    }
    
    if (owf != NULL) {
        // Allocate a buffer that's as large as the OWF packet we're about to write back
        uint32_t size;
        owf_error_init(&rw_error);
        if (!owf_size(owf, &rw_error, &size)) {
            fprintf(logger, "=> error getting OWF size: %s\n", rw_error.error);
        } else {
            void *ptr = owf_malloc(alloc, error, size);
            if (ptr == NULL) {
                return false;
            }
            
            // Write out the packet
            owf_buffer_init(&output_buffer, ptr, size);
            owf_binary_writer_init_buffer(&writer, &output_buffer, alloc, &rw_error);
            if (!owf_binary_write(&writer, owf)) {
                fprintf(logger, "=> error writing packet to buffer: %s\n", rw_error.error);
            } else {
                // Write the buffer to the UDP socket
                if ((len = sendto(sfd, output_buffer.ptr, output_buffer.length, 0, &client_addr, client_len)) > 0) {
                    fprintf(logger, "=> wrote a " OWF_PRINT_U32 "-byte OWF packet\n", size);
                } else {
                    fprintf(logger, "=> error writing buffer to socket: %d\n", OWF_SOCKET_ERROR);
                }
            }
            
            owf_free(alloc, ptr);
        }
        
        owf_destroy(owf, alloc);
    }
    
    return true;
}

int main(int argc, const char **argv) {
    owf_error_t error = OWF_ERROR_DEFAULT;
    owf_alloc_t alloc = {.malloc = malloc, .realloc = realloc, .free = free, .max_alloc = OWF_ALLOC_DEFAULT_MAX};
    FILE *logger = stderr;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <tcp|udp> <host> <port>\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  $ %s tcp 127.0.0.1 1234\n", argv[0]);
        fprintf(stderr, "  $ %s udp localhost 9001\n", argv[0]);
        return 1;
    }

    if (!owf_server_start(logger, &alloc, &error, argv[2], argv[1], argv[3])) {
        fprintf(logger, "error starting server on %s://%s:%s: %s\n", argv[1], argv[2], argv[3], error.error);
        return 1;
    }
    
    return 0;
}
