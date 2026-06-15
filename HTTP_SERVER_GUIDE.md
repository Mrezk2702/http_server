# Building an HTTP Server from Scratch in C++

## Introduction

This guide will help you build a serious, production-quality HTTP server in C++. We'll go deep into the project, covering core functionality, edge cases, performance optimizations, and advanced features.

## Core Knowledge You'll Need

### 1. Network Programming Fundamentals
- **Socket API**: `socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`
- **TCP/IP**: How TCP connections work, the three-way handshake, connection states
- **Non-blocking I/O**: `select()`, `poll()`, `epoll()` (Linux), `kqueue()` (BSD/macOS)
- **File descriptors**: Managing multiple connections efficiently

### 2. HTTP Protocol Deep Understanding
- **HTTP/1.1 specification** (RFC 7230-7235)
- Request/response structure (start line, headers, body)
- Methods: GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH
- Status codes and their meanings
- Headers: Content-Length, Transfer-Encoding, Connection, Host, etc.
- **Chunked transfer encoding**
- **Persistent connections** (keep-alive)
- **Pipelining** (multiple requests on one connection)

### 3. Parsing & State Machines
- Building a robust HTTP parser that handles malformed requests
- Handling partial reads (data arrives in chunks)
- Buffer management

### 4. Concurrency Models
- **Thread-per-connection**: Simple but doesn't scale
- **Thread pool**: Better resource management
- **Event-driven (epoll/kqueue)**: What production servers use
- **Hybrid approaches**: Event loop + worker threads

---

## Main Functionality (From Basic to Advanced)

### Phase 1: Foundation
- [ ] Basic TCP socket server accepting connections
- [ ] Single-threaded, handling one client at a time
- [ ] Parse simple GET requests
- [ ] Serve static files from a directory
- [ ] Return proper HTTP responses with status codes

### Phase 2: Core HTTP Features
- [ ] Support all major HTTP methods
- [ ] Request header parsing (case-insensitive, multi-line)
- [ ] Query string parsing
- [ ] POST body parsing (URL-encoded and multipart)
- [ ] Response headers (Content-Type, Content-Length, Date, Server)
- [ ] Persistent connections (Connection: keep-alive)
- [ ] Chunked transfer encoding (both parsing and sending)

### Phase 3: Concurrency & Performance
- [ ] Thread pool for handling requests
- [ ] Event-driven I/O with epoll/kqueue
- [ ] Non-blocking sockets
- [ ] Connection timeout handling
- [ ] Request rate limiting per IP

### Phase 4: Advanced Features
- [ ] Virtual hosting (multiple domains on one server)
- [ ] URL routing and middleware architecture
- [ ] Range requests (partial content, resume downloads)
- [ ] Compression (gzip, deflate)
- [ ] WebSocket upgrade support
- [ ] HTTPS/TLS support (with OpenSSL)
- [ ] HTTP/2 support (bonus challenge!)

---

## Critical Edge Cases & Robustness

### Request Parsing Edge Cases
- Malformed requests (missing headers, invalid syntax)
- Extremely large headers (DoS prevention)
- Slowloris attacks (slow header/body transmission)
- Pipeline smuggling attacks
- Null bytes in headers
- Header injection attempts
- Requests larger than buffer size

### Connection Handling
- Client disconnects mid-request
- Client sends data too slowly
- Server backpressure (can't write to client socket fast enough)
- File descriptor exhaustion
- Memory leaks from unclosed connections

### File Serving
- Path traversal attacks (`../../../etc/passwd`)
- Serving files outside document root
- Reading files that don't exist
- Reading directories vs files
- Symbolic link handling
- File permissions
- Large file streaming without loading into memory

### Resource Management
- Proper cleanup on errors
- RAII for sockets and file handles
- Graceful shutdown
- Signal handling (SIGPIPE, SIGINT)

---

## Recommended Architecture

```
http-server/
├── src/
│   ├── main.cpp
│   ├── server/
│   │   ├── tcp_server.cpp/h        # Socket management
│   │   ├── http_server.cpp/h       # HTTP-specific logic
│   │   ├── connection.cpp/h        # Per-connection state
│   │   └── thread_pool.cpp/h       # Worker threads
│   ├── http/
│   │   ├── request.cpp/h           # Request representation
│   │   ├── response.cpp/h          # Response builder
│   │   ├── parser.cpp/h            # HTTP parser
│   │   └── router.cpp/h            # URL routing
│   ├── io/
│   │   ├── file_handler.cpp/h      # Safe file operations
│   │   ├── buffer.cpp/h            # Dynamic buffers
│   │   └── event_loop.cpp/h        # epoll/kqueue wrapper
│   └── utils/
│       ├── logger.cpp/h
│       ├── config.cpp/h
│       └── string_utils.cpp/h
├── tests/
├── benchmarks/
└── docs/
```

---

## Development Roadmap

### Week 1: Basic Single-Threaded Server
**Goals:**
- Create a TCP socket that listens on a port
- Accept incoming connections
- Read HTTP request line and headers
- Serve static files from a directory
- Send proper HTTP responses

**Key Learning:**
- Socket API basics
- Understanding HTTP request/response format
- File I/O in C++
- Basic error handling

### Week 2: Robust HTTP Parser
**Goals:**
- Parse all HTTP methods correctly
- Handle headers (case-insensitive, multi-line values)
- Parse query strings and URL parameters
- Handle POST body (URL-encoded)
- Implement proper error responses (400, 404, 500, etc.)

**Key Learning:**
- State machine design for parsing
- Handling partial reads
- Buffer management
- Edge case handling

### Week 3: Thread Pool and Concurrent Handling
**Goals:**
- Implement a thread pool
- Handle multiple clients simultaneously
- Thread-safe logging
- Connection keep-alive support

**Key Learning:**
- C++ threading primitives
- Mutexes and condition variables
- Work queue design
- Thread synchronization

### Week 4: Event-Driven I/O
**Goals:**
- Implement epoll (Linux) or kqueue (macOS/BSD) wrapper
- Non-blocking sockets
- Hybrid model: event loop + worker threads
- Timeout handling

**Key Learning:**
- I/O multiplexing
- Non-blocking I/O patterns
- State management for async operations
- Performance optimization

### Week 5+: Advanced Features
**Choose based on interest:**
- HTTPS/TLS with OpenSSL
- WebSocket support
- HTTP/2 protocol
- Compression (gzip)
- Virtual hosting
- Custom middleware/plugin system

---

## Important Design Decisions

### 1. Buffer Management Strategy
**Options:**
- Fixed-size buffers (simple, but limiting)
- Dynamic buffers that grow (flexible, but needs careful management)
- Ring buffers (efficient for streaming)

**Recommendation:** Start with fixed-size, move to dynamic as needed.

### 2. Concurrency Model
**Options:**
- Thread-per-connection (simple, doesn't scale)
- Thread pool (good balance)
- Pure event-driven (most scalable, complex)
- Hybrid: event loop + thread pool (production-ready)

**Recommendation:** Start with thread pool, evolve to hybrid.

### 3. Error Handling Philosophy
**Critical:** Every system call can fail. Always check return values.

```cpp
// BAD
int sock = socket(AF_INET, SOCK_STREAM, 0);

// GOOD
int sock = socket(AF_INET, SOCK_STREAM, 0);
if (sock < 0) {
    perror("socket creation failed");
    return -1;
}
```

### 4. Resource Management (RAII)
Use RAII wrappers for all resources:
- Socket wrapper that auto-closes on destruction
- File handle wrapper
- Lock guards for mutexes

---

## Testing Strategy

### Unit Tests
- HTTP parser with various inputs
- Buffer operations
- URL decoding/encoding
- Header parsing

### Integration Tests
- Send actual HTTP requests with curl/telnet
- Test keep-alive connections
- Test concurrent requests
- Test large file uploads/downloads

### Stress Tests
- Apache Bench (ab)
- wrk (modern HTTP benchmarking tool)
- Custom scripts to test edge cases

### Security Tests
- Path traversal attempts
- Header injection
- Large header DoS
- Slowloris attack simulation

---

## Performance Benchmarks

**Target metrics:**
- Requests per second (RPS)
- Latency (p50, p95, p99)
- Memory usage under load
- CPU utilization
- Connection handling capacity

**Compare against:**
- nginx (C, event-driven)
- Apache (C, multi-process)
- Lighttpd (C, event-driven)

---

## Learning Resources

### Essential Reading
1. **Beej's Guide to Network Programming** - Socket programming fundamentals
2. **RFC 7230-7235** - HTTP/1.1 specification
3. **The Linux Programming Interface** - System programming deep dive
4. **C++ Concurrency in Action** - Threading and synchronization

### Example Code to Study
- **Mongoose** - Embedded web server in C
- **Tiny HTTPd** - Minimal HTTP server for learning
- **nginx** - Production event-driven server (complex but gold standard)

---

## Next Steps

**Where to start:**

1. **Option A - Socket Fundamentals First**
   - Build a basic TCP echo server
   - Understand socket lifecycle
   - Then add HTTP on top

2. **Option B - HTTP Parser First**
   - Build the parser as a standalone component
   - Test it thoroughly with unit tests
   - Then integrate with socket server

3. **Option C - Minimal Working Example**
   - Build simplest possible HTTP server (100 lines)
   - Incrementally add features
   - Refactor as complexity grows

**Recommended: Option C** - Get something working quickly, then improve iteratively.

---

## Your Current Level?

To calibrate the explanations and starting point:
- What's your comfort level with C++? (beginner/intermediate/advanced)
- Have you worked with sockets before?
- Have you done multi-threaded programming?
- What operating system will you develop on? (Linux/macOS/Windows)

Let's build this together! 🚀
