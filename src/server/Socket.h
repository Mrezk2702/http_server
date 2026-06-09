#pragma once

#include <string>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * RAII wrapper for socket file descriptors
 * Ensures sockets are properly closed even if exceptions occur
 */
class Socket {
private:
    int fd_;
    
    // Delete copy operations - sockets shouldn't be copied
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

public:
    /**
     * Create an invalid socket (fd = -1)
     * Used with move semantics
     */
    Socket() : fd_(-1) {}

    /**
     * Create a socket with the given parameters
     * @param domain AF_INET (IPv4) or AF_INET6 (IPv6)
     * @param type SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
     * @param protocol Usually 0 for auto-select
     */
    Socket(int domain, int type, int protocol = 0) {
        fd_ = socket(domain, type, protocol);
        if (fd_ < 0) {
            throw std::runtime_error("Failed to create socket");
        }
    }

    /**
     * Move constructor - transfer ownership
     */
    Socket(Socket&& other) noexcept : fd_(other.release()) {}

    /**
     * Move assignment - transfer ownership
     */
    Socket& operator=(Socket&& other) noexcept {
        close_internal();
        fd_ = other.release();
        return *this;
    }

    /**
     * Destructor - automatically closes the socket
     */
    ~Socket() {
        close_internal();
    }

    /**
     * Get the raw file descriptor
     * Use with caution - prefer using Socket methods
     */
    int get() const { return fd_; }

    /**
     * Check if socket is valid
     */
    bool is_valid() const { return fd_ >= 0; }

    /**
     * Release ownership of the socket
     * Caller is responsible for closing it
     */
    int release() {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }

    /**
     * Set socket option
     * @param level SOL_SOCKET, IPPROTO_TCP, etc.
     * @param optname SO_REUSEADDR, TCP_NODELAY, etc.
     * @param optval Pointer to option value
     * @param optlen Size of optval
     */
    void set_option(int level, int optname, const void* optval, socklen_t optlen) {
        if (setsockopt(fd_, level, optname, optval, optlen) < 0) {
            throw std::runtime_error("Failed to set socket option");
        }
    }

    /**
     * Convenience function to set int socket option
     */
    void set_option_int(int level, int optname, int value) {
        set_option(level, optname, &value, sizeof(value));
    }

    /**
     * Bind socket to address and port
     * @param address sockaddr_in structure with family, port, and address
     */
    void bind(const struct sockaddr_in& address) {
        if (::bind(fd_, (const struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }
    }

    /**
     * Listen for incoming connections
     * @param backlog Maximum number of pending connections
     */
    void listen(int backlog = 10) {
        if (::listen(fd_, backlog) < 0) {
            throw std::runtime_error("Failed to listen on socket");
        }
    }

    /**
     * Accept incoming connection
     * @param client_addr (optional) Will be filled with client address info
     * @return New Socket for the accepted connection
     */
    Socket accept(struct sockaddr_in* client_addr = nullptr) {
        struct sockaddr_in addr = {};
        socklen_t addr_len = sizeof(addr);

        int client_fd = ::accept(fd_, (struct sockaddr*)&addr, &addr_len);
        if (client_fd < 0) {
            throw std::runtime_error("Failed to accept connection");
        }

        if (client_addr) {
            *client_addr = addr;
        }

        // Create Socket from raw fd
        Socket client;
        client.fd_ = client_fd;
        return client;  // Uses move semantics
    }

    /**
     * Receive data from socket
     * @param buffer Destination buffer
     * @param length Maximum bytes to read
     * @return Number of bytes received (0 = connection closed, -1 = error)
     */
    ssize_t recv(void* buffer, size_t length) {
        return ::recv(fd_, buffer, length, 0);
    }

    /**
     * Receive data with flags
     * @param buffer Destination buffer
     * @param length Maximum bytes to read
     * @param flags Flags (MSG_PEEK, MSG_DONTWAIT, etc.)
     * @return Number of bytes received
     */
    ssize_t recv(void* buffer, size_t length, int flags) {
        return ::recv(fd_, buffer, length, flags);
    }

    /**
     * Send data to socket
     * @param buffer Source buffer
     * @param length Number of bytes to send
     * @return Number of bytes sent (or -1 on error)
     */
    ssize_t send(const void* buffer, size_t length) {
        return ::send(fd_, buffer, length, 0);
    }

    /**
     * Send all data, retrying if partial send
     * @param buffer Source buffer
     * @param length Number of bytes to send
     * @return Total bytes sent (or -1 on error)
     */
    ssize_t send_all(const void* buffer, size_t length) {
        size_t total_sent = 0;
        while (total_sent < length) {
            /*passing 0 as the flag means if the client disconnects 
            mid-send, the kernel raises SIGPIPE which kills your
             process by default. MSG_NOSIGNAL turns that into a returnable 
             -1 + errno = EPIPE instead.*/

             
            ssize_t sent = ::send(fd_,
                                 static_cast<const char*>(buffer) + total_sent,
                                 length - total_sent,
                                 MSG_NOSIGNAL);
            if (sent <= 0) {
                return -1;
            }
            total_sent += sent;
        }
        return total_sent;
    }

    /**
     * Close the socket
     */
    void close() {
        close_internal();
    }

private:
    void close_internal() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }
};
