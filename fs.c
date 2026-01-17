#include "fs.h"
#include "printk.h"

file_t file_table[MAX_FILES];
fd_t fd_table[MAX_FDS];

static int strcmp_simple(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

static void strcpy_simple(char *dst, const char *src, int max) {
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

void fs_init(void) {
    printk("Initializing filesystem...\n");
    
    for (int i = 0; i < MAX_FILES; i++) {
        file_table[i].in_use = 0;
        file_table[i].size = 0;
    }
    
    for (int i = 0; i < MAX_FDS; i++) {
        fd_table[i].in_use = 0;
    }
    
    printk("Filesystem initialized\n");
}

int fs_open(const char *path, int flags) {
    int file_idx = -1;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].in_use && strcmp_simple(file_table[i].name, path) == 0) {
            file_idx = i;
            break;
        }
    }
    
    if (file_idx == -1 && (flags & O_CREAT)) {
        for (int i = 0; i < MAX_FILES; i++) {
            if (!file_table[i].in_use) {
                file_idx = i;
                file_table[i].in_use = 1;
                strcpy_simple(file_table[i].name, path, MAX_FILENAME);
                file_table[i].size = 0;
                break;
            }
        }
    }
    
    if (file_idx == -1) {
        return -1;
    }

    if (flags & O_TRUNC) {
        file_table[file_idx].size = 0;
    }

    for (int i = 0; i < MAX_FDS; i++) {
        if (!fd_table[i].in_use) {
            fd_table[i].in_use = 1;
            fd_table[i].file_idx = file_idx;
            fd_table[i].flags = flags;
            fd_table[i].offset = 0;
            return i;
        }
    }
    
    return -1;
}

int fs_close(int fd) {
    if (fd < 0 || fd >= MAX_FDS || !fd_table[fd].in_use) {
        return -1;
    }
    
    fd_table[fd].in_use = 0;
    return 0;
}

int fs_read(int fd, void *buf, uint32_t count) {
    if (fd < 0 || fd >= MAX_FDS || !fd_table[fd].in_use) {
        return -1;
    }
    
    fd_t *fdesc = &fd_table[fd];
    file_t *file = &file_table[fdesc->file_idx];
    
    if ((fdesc->flags & 3) == O_WRONLY) {
        return -1;
    }
    
    uint32_t available = file->size - fdesc->offset;
    if (count > available) {
        count = available;
    }
    
    uint8_t *dest = (uint8_t *)buf;
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = file->data[fdesc->offset + i];
    }
    
    fdesc->offset += count;
    return count;
}

int fs_write(int fd, const void *buf, uint32_t count) {
    if (fd < 0 || fd >= MAX_FDS || !fd_table[fd].in_use) {
        return -1;
    }
    
    fd_t *fdesc = &fd_table[fd];
    file_t *file = &file_table[fdesc->file_idx];
    
    if ((fdesc->flags & 3) == O_RDONLY) {
        return -1;
    }
    
    if (fdesc->offset + count > MAX_FILESIZE) {
        count = MAX_FILESIZE - fdesc->offset;
    }
    
    const uint8_t *src = (const uint8_t *)buf;
    for (uint32_t i = 0; i < count; i++) {
        file->data[fdesc->offset + i] = src[i];
    }
    
    fdesc->offset += count;
    
    if (fdesc->offset > file->size) {
        file->size = fdesc->offset;
    }
    
    return count;
}
