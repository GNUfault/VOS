#include "process.h"
#include "printk.h"
#include <stddef.h>

process_t proc_table[MAX_PROCESSES];
int current_pid = 0;
static int next_pid = 1;

void process_init(void) {
    printk("Initializing process table...\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        proc_table[i].state = PROC_UNUSED;
        proc_table[i].pid = 0;
        proc_table[i].stack = NULL;
    }
    
    proc_table[0].pid = 1;
    proc_table[0].ppid = 0;
    proc_table[0].state = PROC_RUNNING;
    for (int i = 0; i < PROC_NAME_LEN && "init"[i]; i++) {
        proc_table[0].name[i] = "init"[i];
    }
    current_pid = 1;
    
    printk("Init process created (PID 1)\n");
}

process_t *process_current(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].pid == current_pid) {
            return &proc_table[i];
        }
    }
    return NULL;
}

process_t *process_get(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].pid == pid && proc_table[i].state != PROC_UNUSED) {
            return &proc_table[i];
        }
    }
    return NULL;
}

int process_create(const char *name, void (*entry)(void)) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].state == PROC_UNUSED) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) return -1;
    
    process_t *proc = &proc_table[slot];
    proc->pid = next_pid++;
    proc->ppid = current_pid;
    proc->state = PROC_READY;
    
    int i;
    for (i = 0; i < PROC_NAME_LEN - 1 && name[i]; i++) {
        proc->name[i] = name[i];
    }
    proc->name[i] = '\0';
    
    static uint8_t stacks[MAX_PROCESSES][STACK_SIZE];
    proc->stack = stacks[slot];

    proc->context.pc = (uint64_t)entry;
    proc->context.sp = (uint64_t)(proc->stack + STACK_SIZE);

    for (int j = 0; j < 16; j++) {
        proc->fds[j] = -1;
    }
    
    printk("Created process '%s' (PID %d)\n", proc->name, proc->pid);
    return proc->pid;
}

void process_exit(int code) {
    process_t *proc = process_current();
    if (!proc) return;
    
    printk("Process %d ('%s') exiting with code %d\n", proc->pid, proc->name, code);
    
    proc->state = PROC_ZOMBIE;
    proc->exit_code = code;
    
    process_t *parent = process_get(proc->ppid);
    if (parent && parent->state == PROC_BLOCKED) {
        parent->state = PROC_READY;
    }

    process_yield();
}

int process_wait(int *status) {
    process_t *proc = process_current();
    if (!proc) return -1;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].ppid == proc->pid && proc_table[i].state == PROC_ZOMBIE) {
            int child_pid = proc_table[i].pid;
            if (status) {
                *status = proc_table[i].exit_code;
            }

            proc_table[i].state = PROC_UNUSED;
            proc_table[i].pid = 0;
            
            return child_pid;
        }
    }

    proc->state = PROC_BLOCKED;
    process_yield();
    
    return -1;
}

void process_yield(void) {
    process_t *current = process_current();
    if (current && current->state == PROC_RUNNING) {
        current->state = PROC_READY;
    }

    int start = current_pid;
    for (int i = 1; i <= MAX_PROCESSES; i++) {
        int idx = (start + i) % MAX_PROCESSES;
        if (proc_table[idx].state == PROC_READY) {
            current_pid = proc_table[idx].pid;
            proc_table[idx].state = PROC_RUNNING;
            return;
        }
    }
    
    if (current && current->state == PROC_READY) {
        current->state = PROC_RUNNING;
    }
}

int process_fork(void) {
    printk("[fork] Not fully implemented (requires scheduler)\n");
    return -1;
}

int process_exec(const char *path) {
    process_t *proc = process_current();
    if (!proc) return -1;
    
    printk("[exec] Process %d executing '%s'\n", proc->pid, path);
    printk("[exec] ERROR: exec not fully implemented\n");
    printk("[exec] Would load: %s\n", path);
    
    return -1;
}

int process_kill(int pid, int sig) {
    printk("[kill] Attempting to send signal %d to PID %d\n", sig, pid);

    process_t *target = process_get(pid);
    if (!target) {
        printk("[kill] ERROR: Process %d not found\n", pid);
        return -1;
    }
    
    printk("[kill] Found target process '%s' (PID %d)\n", target->name, target->pid);

    switch (sig) {
        case 9:
            printk("[kill] SIGKILL: Terminating process %d\n", pid);
            target->state = PROC_ZOMBIE;
            target->exit_code = 128 + sig;

            process_t *parent = process_get(target->ppid);
            if (parent && parent->state == PROC_BLOCKED) {
                parent->state = PROC_READY;
            }
            break;
            
        case 15:
            printk("[kill] SIGTERM: Requesting termination of process %d\n", pid);
            target->state = PROC_ZOMBIE;
            target->exit_code = 128 + sig;
            
            process_t *parent2 = process_get(target->ppid);
            if (parent2 && parent2->state == PROC_BLOCKED) {
                parent2->state = PROC_READY;
            }
            break;
            
        case 0:
            printk("[kill] Signal 0: Process %d exists\n", pid);
            return 0;
            
        default:
            printk("[kill] Signal %d not implemented\n", sig);
            return -1;
    }
    
    return 0;
}
