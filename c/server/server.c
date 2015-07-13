#include <owf.h>
#include <owf/types.h>
#include <owf/reader.h>
#include <owf/reader/binary.h>
#include <owf/writer.h>
#include <owf/writer/binary.h>
#include <owf/platform.h>
#include <owf/version.h>
#include <errno.h>
#include <signal.h>

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
    typedef int owf_socket_t;
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
    typedef SOCKET owf_socket_t;
    #define owf_socket_close(...) closesocket(__VA_ARGS__)
    #define owf_socket_poll(...) WSAPoll(__VA_ARGS__)
    #define OWF_SOCKET_ERROR WSAGetLastError()
    #define OWF_SOCKET_EINTR WSAEINTR
    #define OWF_SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
    #define OWF_SOCKET_EAGAIN OWF_SOCKET_EWOULDBLOCK
#endif

#define OWF_SERVER_TCP 0
#define OWF_SERVER_UDP 1

#define OWF_SERVER_UDP_BUFFER_SIZE 1048576
#define OWF_SERVER_MAX_CLIENT_BATCH 10
#define OWF_SERVER_LISTEN_QUEUE 8192

static volatile bool owf_server_go = true;

void owf_server_signal(int sig);
bool owf_server_setup_socket(owf_error_t *error, owf_socket_t fd, uint16_t mode, bool master);
bool owf_server_start(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, const char *host_str, const char *protocol_str, const char *port_str);
bool owf_server_loop_tcp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, struct pollfd *pfd, owf_socket_t sfd);
bool owf_server_loop_udp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, uint8_t *buffer, owf_socket_t sfd);

void owf_server_signal(int sig) {
    signal(sig, owf_server_signal);

    switch (sig) {
        case SIGINT:
            owf_server_go = false;
            break;
        default:
            break;
    }
}

bool owf_server_setup_socket(owf_error_t *error, owf_socket_t fd, uint16_t mode, bool master) {
    if (fd < 0) {
        OWF_ERROR_SET(error, "invalid fd");
        return false;
    }
#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    char flag = 1;
#elif OWF_PLATFORM_IS_GNU
    int flag = 1;
#endif

    if (master && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) != 0) {
        OWF_ERROR_SETF(error, "setsockopt for SO_REUSEADDR failed: %d", OWF_SOCKET_ERROR);
        return false;
    } else if (mode == OWF_SERVER_TCP && setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) != 0) {
        OWF_ERROR_SETF(error, "setsockopt for TCP_NODELAY failed: %d", OWF_SOCKET_ERROR);
        return false;
    }

#if OWF_PLATFORM == OWF_PLATFORM_BSD || OWF_PLATFORM == OWF_PLATFORM_DARWIN
    // admonish the socket not to send SIGPIPE
    if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(flag)) != 0) {
        OWF_ERROR_SETF(error, "setsockopt for SO_NOSIGPIPE failed: %d", OWF_SOCKET_ERROR);
        return false;
    }
#endif

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    // enable nonblocking mode on Windows
    unsigned long flags = 1;
    return ioctlsocket(fd, FIONBIO, &flags) == 0;
#elif OWF_PLATFORM_IS_GNU
    // enable nonblocking mode on *NIX
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        OWF_ERROR_SETF(error, "fcntl failed: %d", OWF_SOCKET_ERROR);
        return false;
    }
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) == 0;
#endif
}

bool owf_server_start(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, const char *host_str, const char *protocol_str, const char *port_str) {
#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    WSADATA ws;
#endif
    owf_array_t fds;
    struct addrinfo hints, *host = NULL;
    struct addrinfo *ptr = NULL;
    uint8_t *buffer = NULL;
    owf_socket_t fd = -1;
    int err = 0, nfds = 0;
    bool ret = true;
    uint16_t protocol;

    // verify the protocol
    if (owf_strcasecmp(protocol_str, "tcp") == 0) {
        protocol = OWF_SERVER_TCP;
    } else if (owf_strcasecmp(protocol_str, "udp") == 0) {
        protocol = OWF_SERVER_UDP;
    } else {
        OWF_ERROR_SETF(error, "invalid protocol `%s`; we support tcp and udp", protocol_str);
        return false;
    }

    // set up the addrinfo hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICSERV | AI_CANONNAME;
    hints.ai_socktype = protocol == OWF_SERVER_TCP ? SOCK_STREAM : SOCK_DGRAM;

#if OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    // initialize winsock
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        OWF_ERROR_SETF(error, "WSAStartup failed: %d", OWF_SOCKET_ERROR);
        return false;
    } else if (LOBYTE(ws.wVersion) != 2 || HIBYTE(ws.wVersion) != 2) {
        OWF_ERROR_SETF(error, "WSAStartup returned an invalid version of Winsock");
        goto fail;
    } else {
        fprintf(logger, "(running on WinSock " OWF_PRINT_SIZE "." OWF_PRINT_SIZE ")\n\n", LOBYTE(ws.wVersion), HIBYTE(ws.wVersion));
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
        } else if (!owf_server_setup_socket(error, fd, protocol, true)) {
            goto fail;
        } else if (bind(fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            // Successfully bound
            const char *typename = ptr->ai_socktype == SOCK_STREAM ? "tcp" : (ptr->ai_socktype == SOCK_DGRAM ? "udp" : NULL);
            const char *hostname = ptr->ai_canonname == NULL ? host_str : ptr->ai_canonname;
            struct sockaddr *addr = ptr->ai_addr;
            int port;
            
            // Make sure we're a TCP or UDP socket
            if (typename == NULL) {
                OWF_ERROR_SET(error, "type is neither SOCK_STREAM not SOCK_DGRAM");
                goto fail;
            }
            
            // Try to listen
            if (ptr->ai_socktype == SOCK_STREAM && listen(fd, OWF_SERVER_LISTEN_QUEUE) != 0) {
                OWF_ERROR_SETF(error, "error listening: %d", OWF_SOCKET_ERROR);
                goto fail;
            }

            // Make sure we bound to the right family
            if (addr->sa_family == AF_INET) {
                port = (int)ntohs(((struct sockaddr_in *)addr)->sin_port);
                fprintf(logger, "bound to socket on %s://%s:%d\n", typename, hostname, port);
            } else if (addr->sa_family == AF_INET6) {
                port = (int)ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
                fprintf(logger, "bound to IPv6 socket on %s://%s:%d\n", typename, hostname, port);
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
        while (owf_server_go) {
            // Try to accept new clients
            for (int i = 0; i < OWF_SERVER_MAX_CLIENT_BATCH; i++) {
                owf_socket_t cfd;
                
                if ((cfd = accept(fd, NULL, NULL)) == -1) {
                    if (OWF_SOCKET_ERROR != OWF_SOCKET_EAGAIN && OWF_SOCKET_ERROR != OWF_SOCKET_EWOULDBLOCK) {
                        // error accepting this client
                        OWF_ERROR_SETF(error, "error accepting client: %d", OWF_SOCKET_ERROR);
                        goto fail;
                    } else if (OWF_SOCKET_ERROR == OWF_SOCKET_EAGAIN || OWF_SOCKET_ERROR == OWF_SOCKET_EWOULDBLOCK) {
                        // no more clients to accept in this batch
                        break;
                    }
                } else {
                    // set up the client socket
                    if (!owf_server_setup_socket(error, cfd, protocol, false)) {
                        goto fail;
                    }
                    
                    // create a poll file descriptor
                    struct pollfd pfd;
                    pfd.fd = cfd;
                    pfd.events = POLLRDNORM | POLLWRNORM;
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
        }
    } else {
        // Allocate a buffer
        buffer = owf_malloc(alloc, error, OWF_SERVER_UDP_BUFFER_SIZE);
        if (buffer == NULL) {
            goto fail;
        }

        while (owf_server_go) {
            // Process existing clients
            if (!owf_server_loop_udp(logger, alloc, error, buffer, fd)) {
                goto fail;
            }
        }
    }
    goto out;

fail:
    ret = false;
out:
    fprintf(logger, "Exiting\n");
    
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

#if OWF_PLATFORM == OWF_PLATFORM_LINUX
    // covers SIGPIPE on Linux
    const int flags = MSG_NOSIGNAL;
#else
    // BSD/Darwin are covered by setsockopt, Windows doesn't have the SIGPIPE issue
    const int flags = 0;
#endif

    while (bytes_left > 0) {
        if ((bytes_read = recv(pfd->fd, buf, bytes_left, flags)) < 0) {
            if (OWF_NOEXPECT(OWF_SOCKET_ERROR == OWF_SOCKET_EINTR)) {
                bytes_read = 0;
            } else if (OWF_NOEXPECT(OWF_SOCKET_ERROR == OWF_SOCKET_EWOULDBLOCK)) {
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

    return bytes_left == 0;
}

bool owf_server_tcp_write_cb(const void *src, const size_t size, void *data) {
    struct pollfd *pfd = (struct pollfd *)data;
    size_t bytes_left = size;
    ssize_t bytes_written;
    uint8_t *buf = (uint8_t *)src;
    
#if OWF_PLATFORM == OWF_PLATFORM_LINUX
    // covers SIGPIPE on Linux
    const int flags = MSG_NOSIGNAL;
#else
    // BSD/Darwin are covered by setsockopt, Windows doesn't have the SIGPIPE issue
    const int flags = 0;
#endif

    while (bytes_left > 0) {
        if ((bytes_written = send(pfd->fd, buf, bytes_left, flags)) <= 0) {
            if (OWF_NOEXPECT(OWF_SOCKET_ERROR == OWF_SOCKET_EINTR)) {
                bytes_written = 0;
            } else if (OWF_NOEXPECT(OWF_SOCKET_ERROR == OWF_SOCKET_EAGAIN)) {
                continue;
            } else {
                return false;
            }
        }

        bytes_left -= (size_t)bytes_written;
        buf += (size_t)bytes_written;
    }

    return bytes_left == 0;
}

bool owf_server_loop_tcp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, struct pollfd *pfd, owf_socket_t sfd) {
    owf_binary_reader_t reader;
    owf_binary_writer_t writer;
    owf_error_t rw_error;
    owf_t *owf = NULL;
    uint32_t size = 0;

    if (pfd->revents & POLLRDNORM && pfd->revents & POLLWRNORM && !(pfd->revents & POLLHUP) && !(pfd->revents & POLLERR) && !(pfd->revents & POLLNVAL)) {
        // Some data is here, hopefully containing an OWF message.
        // Greedily eat the data until we can receive an OWF message.
        owf_error_init(&rw_error);
        owf_binary_reader_init(&reader, alloc, &rw_error, owf_server_tcp_read_cb, NULL, pfd);

        if ((owf = owf_binary_materialize(&reader)) == NULL) {
            fprintf(logger, "<= error materializing packet for fd %d: %s\n", pfd->fd, rw_error.error);
            owf_socket_close(pfd->fd);
            pfd->fd = 0;
            pfd->events = 0;
            pfd->revents = 0;
        } else {
            // hooray, we have an OWF packet
            if (!owf_size(owf, &rw_error, &size)) {
                fprintf(logger, "<= error getting OWF size for fd %d's packet: %s\n", pfd->fd, rw_error.error);
                owf_socket_close(pfd->fd);
                pfd->fd = 0;
                pfd->events = 0;
                pfd->revents = 0;
            } else {
                fprintf(logger, "<= got a " OWF_PRINT_U32 "-byte OWF packet from fd %d\n", size, pfd->fd);
            }
        }
    }

    if (pfd->revents & POLLWRNORM && !(pfd->revents & POLLHUP) && !(pfd->revents & POLLERR) && !(pfd->revents & POLLNVAL) && owf != NULL) {
        // initialize the binary writer
        owf_error_init(&rw_error);
        owf_binary_writer_init(&writer, alloc, &rw_error, owf_server_tcp_write_cb, pfd);

        // write to the buffer
        if (!owf_binary_write(&writer, owf)) {
            fprintf(logger, "=> error writing packet to fd %d: %s\n", pfd->fd, rw_error.error);
            owf_socket_close(pfd->fd);
            pfd->fd = 0;
            pfd->events = 0;
            pfd->revents = 0;
        } else {
            if (!owf_size(owf, &rw_error, &size)) {
                fprintf(logger, "=> error getting OWF size for fd %d's packet: %s\n", pfd->fd, rw_error.error);
                owf_socket_close(pfd->fd);
                pfd->fd = 0;
                pfd->events = 0;
                pfd->revents = 0;
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

bool owf_server_loop_udp(FILE *logger, owf_alloc_t *alloc, owf_error_t *error, uint8_t *buffer, owf_socket_t sfd) {
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
    
    // install signal handlers - XXX: we could use sigaction here, but have SIGPIPE handled fairly well
    signal(SIGINT, owf_server_signal);
#if OWF_PLATFORM_IS_GNU
    signal(SIGPIPE, SIG_IGN);
#endif

    fprintf(logger, "--------------------------------\n");
    fprintf(logger, "libowf " OWF_LIBRARY_VERSION_STRING " net server starting\n");
    fprintf(logger, "--------------------------------\n");
    if (!owf_server_start(logger, &alloc, &error, argv[2], argv[1], argv[3])) {
        fprintf(logger, "error starting server on %s://%s:%s: %s\n", argv[1], argv[2], argv[3], error.error);
        return 1;
    }
    
    return 0;
}
