#ifndef UNISTD_H
#define UNISTD_H

#include <stdint.h>

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  0x100
#define O_TRUNC  0x200

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

typedef int pid_t;
typedef unsigned int size_t;
typedef int ssize_t;

static inline void exit(int status) {
    register uint64_t a0 asm("a0") = status;
    register uint64_t a7 asm("a7") = 1;
    asm volatile("ecall" :: "r"(a0), "r"(a7));
    __builtin_unreachable();
}

static inline pid_t fork(void) {
    register uint64_t a0 asm("a0");
    register uint64_t a7 asm("a7") = 2;
    asm volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return (pid_t)a0;
}

static inline ssize_t read(int fd, void *buf, size_t count) {
    register uint64_t a0 asm("a0") = fd;
    register uint64_t a1 asm("a1") = (uint64_t)buf;
    register uint64_t a2 asm("a2") = count;
    register uint64_t a7 asm("a7") = 3;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return (ssize_t)a0;
}

static inline ssize_t write(int fd, const void *buf, size_t count) {
    register uint64_t a0 asm("a0") = fd;
    register uint64_t a1 asm("a1") = (uint64_t)buf;
    register uint64_t a2 asm("a2") = count;
    register uint64_t a7 asm("a7") = 4;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return (ssize_t)a0;
}

static inline int open(const char *pathname, int flags) {
    register uint64_t a0 asm("a0") = (uint64_t)pathname;
    register uint64_t a1 asm("a1") = flags;
    register uint64_t a7 asm("a7") = 5;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return (int)a0;
}

static inline int close(int fd) {
    register uint64_t a0 asm("a0") = fd;
    register uint64_t a7 asm("a7") = 6;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return (int)a0;
}

static inline pid_t wait(int *status) {
    register uint64_t a0 asm("a0") = (uint64_t)status;
    register uint64_t a7 asm("a7") = 7;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return (pid_t)a0;
}

static inline pid_t getpid(void) {
    register uint64_t a0 asm("a0");
    register uint64_t a7 asm("a7") = 9;
    asm volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return (pid_t)a0;
}

static inline size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

#endif

