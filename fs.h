#ifndef _FS_H
#define _FS_H

#include "lib.h"
#include "sys.h"
#include "multiboot.h"


#define GET_INODE(bb, inum) ((inum) >= (bb)->nr_inodes ? NULL : &((bb)->inodes[inum]))
#define GET_DATABLOCK(bb, dnum) ((dnum) >= (bb)->nr_datablocks ? NULL : &((bb)->inodes[(bb)->nr_inodes + dnum]))
#define NR_DIR_ENTRIES 63
#define NR_DATABLOCKS 1023
#define TYPE_RTC 0
#define TYPE_DIR 1
#define TYPE_FILE 2
#define FOUR_KB 4096
#define KERNEL_BOT 0x800000
#define EIGHT_KB 8192
#define MAX_NAME_LEN 32
#define DENTRY_SIZE 64

extern int32_t cur_pid; // change location of this to either filesystem or lib.h bc its used in filesystem too


struct dentry_t {
	uint8_t name[32];
	uint32_t type;
	uint32_t inode;
	uint8_t reserved[24];
};


typedef struct inode {
	uint32_t length;
	uint32_t datablocks[NR_DATABLOCKS];
} inode_t;

typedef struct bootblock {
	uint32_t nr_dentries;
	uint32_t nr_inodes;
	uint32_t nr_datablocks;
	uint8_t reserved[52];
	struct dentry_t dir_entries[NR_DIR_ENTRIES];
}  bootblock_t;

struct fs_data {
	uint32_t curr_inode;
	uint32_t curr_offset;
	uint32_t type;
};


int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);
// int32_t fs_read(struct fs_data *data, void* buf, int32_t nbytes);
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t fs_close(int32_t fd);
int32_t fs_open(const uint8_t *filename);
// int32_t fs_open(const uint8_t *filename, struct fs_data *data);

int32_t dir_open(const uint8_t *filename);
int32_t dir_close(int32_t fd);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
// int32_t dir_read(struct fs_data *data, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t read_dentry_by_name(const uint8_t *filename, struct dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, struct dentry_t *dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

void fs_init(module_t* mod);

#endif /* _FS_H */
