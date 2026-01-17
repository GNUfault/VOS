#ifndef FS_H
#define FS_H

#include <stdint.h>

#define MAX_FILES 32
#define MAX_FILENAME 64
#define MAX_FILESIZE 4096
#define MAX_FDS 64

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  0x100
#define O_TRUNC  0x200

typedef struct {
    char name[MAX_FILENAME];
    uint8_t data[MAX_FILESIZE];
    uint32_t size;
    int in_use;
} file_t;

typedef struct {
    int file_idx;
    int flags;
    uint32_t offset;
    int in_use;
} fd_t;

extern file_t file_table[MAX_FILES];
extern fd_t fd_table[MAX_FDS];

void fs_init(void);
int fs_open(const char *path, int flags);
int fs_close(int fd);
int fs_read(int fd, void *buf, uint32_t count);
int fs_write(int fd, const void *buf, uint32_t count);

#endif
