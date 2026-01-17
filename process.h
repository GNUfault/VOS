#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 64
#define STACK_SIZE 8192
#define PROC_NAME_LEN 32

typedef enum {
    PROC_UNUSED = 0,
    PROC_RUNNING,
    PROC_READY,
    PROC_BLOCKED,
    PROC_ZOMBIE
} proc_state_t;

typedef struct {
    uint64_t regs[32];
    uint64_t pc;
    uint64_t sp;
} context_t;

typedef struct process {
    int pid;
    int ppid;
    proc_state_t state;
    char name[PROC_NAME_LEN];
    
    context_t context;
    uint8_t *stack;
    
    int exit_code;

    int fds[16];

    uint64_t start_time;
    uint64_t cpu_time;
} process_t;

extern process_t proc_table[MAX_PROCESSES];
extern int current_pid;

void process_init(void);
int process_create(const char *name, void (*entry)(void));
void process_exit(int code);
int process_fork(void);
int process_exec(const char *path);
int process_kill(int pid, int sig);
int process_wait(int *status);
void process_yield(void);
process_t *process_current(void);
process_t *process_get(int pid);

#endif
