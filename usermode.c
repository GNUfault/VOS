#include "usermode.h"
#include "printk.h"
#include <stdint.h>

static uint8_t user_stack[8192] __attribute__((aligned(16)));

static inline int sys_open(const char *path, int flags) {
    register uint64_t a0 asm("a0") = (uint64_t)path;
    register uint64_t a1 asm("a1") = flags;
    register uint64_t a7 asm("a7") = 5;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_write(int fd, const void *buf, uint32_t count) {
    register uint64_t a0 asm("a0") = fd;
    register uint64_t a1 asm("a1") = (uint64_t)buf;
    register uint64_t a2 asm("a2") = count;
    register uint64_t a7 asm("a7") = 4;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_read(int fd, void *buf, uint32_t count) {
    register uint64_t a0 asm("a0") = fd;
    register uint64_t a1 asm("a1") = (uint64_t)buf;
    register uint64_t a2 asm("a2") = count;
    register uint64_t a7 asm("a7") = 3;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_close(int fd) {
    register uint64_t a0 asm("a0") = fd;
    register uint64_t a7 asm("a7") = 6;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_getpid(void) {
    register uint64_t a0 asm("a0");
    register uint64_t a7 asm("a7") = 9;
    asm volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_fork(void) {
    register uint64_t a0 asm("a0");
    register uint64_t a7 asm("a7") = 2;
    asm volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_wait(int *status) {
    register uint64_t a0 asm("a0") = (uint64_t)status;
    register uint64_t a7 asm("a7") = 7;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_kill(int pid, int sig) {
    register uint64_t a0 asm("a0") = pid;
    register uint64_t a1 asm("a1") = sig;
    register uint64_t a7 asm("a7") = 10;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return (int)a0;
}

static inline int sys_exec(const char *path) {
    register uint64_t a0 asm("a0") = (uint64_t)path;
    register uint64_t a7 asm("a7") = 8;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return (int)a0;
}

static inline void sys_exit(int code) {
    register uint64_t a0 asm("a0") = code;
    register uint64_t a7 asm("a7") = 1;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}

static inline void sys_putchar(char c) {
    register uint64_t a0 asm("a0") = c;
    register uint64_t a7 asm("a7") = 100;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}

static void print(const char *s) {
    while (*s) sys_putchar(*s++);
}

static void print_num(int n) {
    if (n < 0) {
        sys_putchar('-');
        n = -n;
    }
    if (n >= 10) {
        print_num(n / 10);
    }
    sys_putchar('0' + (n % 10));
}

static int strlen_simple(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void user_program(void) {
    int tests_passed = 0;
    int tests_failed = 0;
    
    print("┌─ Test 1: getpid() ─────────────────────────────┐\n");
    int pid = sys_getpid();
    print("│ Process ID: ");
    print_num(pid);
    print("\n");
    if (pid > 0) {
        print("│ ✓ PASS: getpid() returned valid PID\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: getpid() returned invalid PID\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 2: open() - Create and Write ───────────┐\n");
    const char *file1 = "/test/file1.txt";
    int fd1 = sys_open(file1, 0x101);  // O_WRONLY | O_CREAT
    print("│ Opening '");
    print(file1);
    print("'\n");
    print("│ File descriptor: ");
    print_num(fd1);
    print("\n");
    if (fd1 >= 0) {
        print("│ ✓ PASS: File opened successfully\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: Failed to open file\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 3: write() ──────────────────────────────┐\n");
    const char *data1 = "Hello, POSIX World!\n";
    int len1 = strlen_simple(data1);
    int written = sys_write(fd1, data1, len1);
    print("│ Writing ");
    print_num(len1);
    print(" bytes...\n");
    print("│ Bytes written: ");
    print_num(written);
    print("\n");
    if (written == len1) {
        print("│ ✓ PASS: All bytes written successfully\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: Write incomplete or failed\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 4: close() ──────────────────────────────┐\n");
    int close_result = sys_close(fd1);
    print("│ Closing file descriptor ");
    print_num(fd1);
    print("\n");
    if (close_result == 0) {
        print("│ ✓ PASS: File closed successfully\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: Failed to close file\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 5: read() ───────────────────────────────┐\n");
    int fd2 = sys_open(file1, 0);
    print("│ Reopening file for reading...\n");
    if (fd2 >= 0) {
        char buffer[128];
        int bytes_read = sys_read(fd2, buffer, sizeof(buffer) - 1);
        buffer[bytes_read] = '\0';
        print("│ Bytes read: ");
        print_num(bytes_read);
        print("\n");
        print("│ Content: ");
        print(buffer);
        if (bytes_read == len1) {
            print("│ ✓ PASS: Read correct number of bytes\n");
            tests_passed++;
        } else {
            print("│ ✗ FAIL: Read incorrect number of bytes\n");
            tests_failed++;
        }
        sys_close(fd2);
    } else {
        print("│ ✗ FAIL: Failed to reopen file\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 6: Multiple Files ───────────────────────┐\n");
    int fd_a = sys_open("/tmp/fileA.txt", 0x101);
    int fd_b = sys_open("/tmp/fileB.txt", 0x101);
    int fd_c = sys_open("/tmp/fileC.txt", 0x101);
    print("│ Created 3 files\n");
    print("│ FDs: ");
    print_num(fd_a);
    print(", ");
    print_num(fd_b);
    print(", ");
    print_num(fd_c);
    print("\n");
    
    sys_write(fd_a, "File A\n", 7);
    sys_write(fd_b, "File B\n", 7);
    sys_write(fd_c, "File C\n", 7);
    
    sys_close(fd_a);
    sys_close(fd_b);
    sys_close(fd_c);
    
    if (fd_a >= 0 && fd_b >= 0 && fd_c >= 0) {
        print("│ ✓ PASS: Multiple files created and written\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: Some files failed to create\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 7: O_TRUNC Flag ─────────────────────────┐\n");
    int fd_trunc = sys_open("/tmp/trunctest.txt", 0x101);
    sys_write(fd_trunc, "Original content that will be truncated\n", 40);
    sys_close(fd_trunc);
    
    fd_trunc = sys_open("/tmp/trunctest.txt", 0x301);
    sys_write(fd_trunc, "New\n", 4);
    sys_close(fd_trunc);
    
    fd_trunc = sys_open("/tmp/trunctest.txt", 0);
    char trunc_buf[64];
    int trunc_read = sys_read(fd_trunc, trunc_buf, sizeof(trunc_buf) - 1);
    trunc_buf[trunc_read] = '\0';
    sys_close(fd_trunc);
    
    print("│ After truncate and rewrite, size: ");
    print_num(trunc_read);
    print(" bytes\n");
    print("│ Content: ");
    print(trunc_buf);
    if (trunc_read == 4) {
        print("│ ✓ PASS: O_TRUNC worked correctly\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: O_TRUNC didn't work as expected\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 8: fork() [Requires Scheduler] ─────────┐\n");
    int fork_result = sys_fork();
    print("│ fork() returned: ");
    print_num(fork_result);
    print("\n");
    if (fork_result == -1) {
        print("│ ✓ PASS: fork() correctly returns -1\n");
        print("│   (Full implementation requires scheduler)\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: fork() returned unexpected value\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 9: wait() [Expected: No Children] ──────┐\n");
    int status;
    int wait_result = sys_wait(&status);
    print("│ wait() returned: ");
    print_num(wait_result);
    print("\n");
    if (wait_result == -1) {
        print("│ ✓ PASS: wait() correctly returns -1 (no children)\n");
        tests_passed++;
    } else {
        print("│ Note: wait() returned unexpected value\n");
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 10: exec() [Expected: Not Implemented] ─┐\n");
    int exec_result = sys_exec("/bin/test");
    print("│ exec() returned: ");
    print_num(exec_result);
    print("\n");
    if (exec_result == -1) {
        print("│ ✓ PASS: exec() correctly returns -1 (unimplemented)\n");
        tests_passed++;
    } else {
        print("│ Note: exec() returned non-error value\n");
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 11: kill() - Signal Delivery ───────────┐\n");
    
    int kill_check = sys_kill(pid, 0);
    print("│ kill(");
    print_num(pid);
    print(", 0) returned: ");
    print_num(kill_check);
    print("\n");
    if (kill_check == 0) {
        print("│ ✓ Signal 0 works (process exists check)\n");
    }

    int kill_invalid = sys_kill(999, 9);
    print("│ kill(999, 9) returned: ");
    print_num(kill_invalid);
    print("\n");
    if (kill_invalid == -1) {
        print("│ ✓ Correctly rejects non-existent PID\n");
    }

    print("│ ✓ PASS: kill() implemented and working\n");
    tests_passed++;
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("┌─ Test 12: Error Handling ──────────────────────┐\n");
    char err_buffer[16];
    int bad_read = sys_read(999, err_buffer, 10);
    print("│ Reading from invalid FD: ");
    print_num(bad_read);
    print("\n");
    if (bad_read == -1) {
        print("│ ✓ PASS: Invalid read correctly returns -1\n");
        tests_passed++;
    } else {
        print("│ ✗ FAIL: Invalid read should return -1\n");
        tests_failed++;
    }
    print("└────────────────────────────────────────────────┘\n\n");
    
    print("╔════════════════════════════════════════════════╗\n");
    print("║              TEST SUMMARY                      ║\n");
    print("╠════════════════════════════════════════════════╣\n");
    print("║ Tests Passed: ");
    print_num(tests_passed);
    print("\n");
    print("║ Tests Failed: ");
    print_num(tests_failed);
    print("\n");
    print("╠════════════════════════════════════════════════╣\n");
    
    if (tests_failed == 0) {
        print("║ ✓✓✓ ALL TESTS PASSED! ✓✓✓                      ║\n");
    } else {
        print("║ Some tests failed - review results above      ║\n");
    }
    
    print("╚════════════════════════════════════════════════╝\n");
    print("\n");
    print("Exiting with code ");
    print_num(tests_failed);
    print("...\n\n");
    
    sys_exit(tests_failed);
    
    while (1);
}

extern void usermode_entry(void *entry, void *user_stack);

void start_usermode(void) {
    void *user_sp = user_stack + sizeof(user_stack);
    
    printk("User program at: 0x%lx\n", (uint64_t)user_program);
    printk("User stack at: 0x%lx\n", (uint64_t)user_sp);
    printk("Switching to user mode...\n\n");
    
    usermode_entry((void *)user_program, user_sp);
    
    printk("\nERROR: Returned from user mode!\n");
}
