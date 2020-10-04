#include "fs.h"

static uint32_t start;

/* void fs_init(module_t* mod);
 * Inputs: mod
 * Return Value: none
 * Function: Sets the start value for the bootlblock */
void fs_init(module_t* mod) {
	start = mod->mod_start;
}

/* int32_t read_dentry_by_name(const uint8_t *filename, struct dentry_t *dentry);
 * Inputs: const uint8_t *filename, struct dentry_t *dentry
 * Return Value: - 1 for failure, otherwise 0
 * Function: Given a filename, it looks through our filesystem for a corresponding file, and then updates dentry
 * with data from the file.
 *  */

int32_t read_dentry_by_name(const uint8_t *filename, struct dentry_t *dentry) {

	uint32_t length = strlen((int8_t*) filename);	// get length of filename
	if (length > MAX_NAME_LEN) {			// bound check
		return -1;
	}
	int dentry_ct = *((int*) start);		// first 4 bytes corrspond to num dentries
	struct bootblock *curr_ = (struct bootblock *)(start);		// declare bootblock at start address

	int i;
	int len;
	for (i = 0; i < dentry_ct; i++) {
		len = strlen((int8_t*)(curr_->dir_entries[i]).name);		// check len of all filenames
			if (len == length && strncmp((int8_t*)(curr_->dir_entries[i]).name, (int8_t*)filename, length) == 0) {		// if length of names match and names match
				strcpy((int8_t*)dentry->name, (int8_t*)(curr_->dir_entries[i]).name);		// copy name into dentry attribute "name"
				dentry->type = (curr_->dir_entries[i]).type;		// copy over type into dentry attribute "type"
				dentry->inode = (curr_->dir_entries[i]).inode;		// copy over inode into dentry attribute "inode"
				return 0;
		}
	}
	return -1;
}

/* int32_t read_dentry_by_index(uint32_t index, struct dentry_t *dentry);
 * Inputs: uint32_t index, struct dentry_t *dentry
 * Return Value: - 1 for failure, otherwise 0
 * Function: Given an index, it looks through our filesystem for a file corresponding to that index, and then updates dentry
 * with data from the file.
 *  */


int32_t read_dentry_by_index(uint32_t index, struct dentry_t *dentry) {
	
	int dentry_ct = *((int*) start);			// first 4 bytes corrspond to num dentries
	struct dentry_t *curr = (struct dentry_t*)(start + ((index + 1)* DENTRY_SIZE));	// set current dentry based on index passed in, accounting for how far it is from start address
	if (index > dentry_ct) {		// bound check
		return -1;
	}
	strcpy((int8_t*)dentry->name, (int8_t*)curr->name);		// copy name into dentry attribute "name"
	dentry->type = curr->type;		// copy over type into dentry attribute "type"
	dentry->inode = curr->inode;		// copy over inode into dentry attribute "inode"
	return 0;

}


/* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);
 * Inputs: uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length
 * Return Value: - 1 for failure, otherwise num bytes copied 
 * Function: Given an inode and offset, it looks through our filesystem for datablocks corresponding 
 * to that inode and offset value, and then updates our buffer with contents from datablocks
 *  */


int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length) {

	inode_t * in_ptr = (inode_t *)(start + ((1 + inode) * FOUR_KB));		// get the correct attributes from "bootblock" (not explicitly used, but rather through offseting by 4 Bytes)
	uint32_t inode_num = *((int *)(start + 4));		// inode num is 4 bytes from start
	uint32_t dblock_num = *((int*)(start + 8));			// num of dblocks is 8 bytes from start
	uint32_t copy_count = 0;		// Initialize variables
	uint32_t data_block_idx = 0;
	uint32_t byte;
	uint8_t* dat_block_address;

	if (inode >= inode_num) {		// Bound check
		return -1;
	}

	if (offset >= in_ptr->length) {		// value check
		return 0;
	}

	data_block_idx = offset/FOUR_KB;		// get offset number and index for data block
	byte = offset % FOUR_KB;

	if (in_ptr->datablocks[data_block_idx] > dblock_num) {		// check if valid datablock
		return -1;
	}

	dat_block_address = (uint8_t *)(start + ((inode_num + 1 + in_ptr->datablocks[data_block_idx]) * FOUR_KB));		// Correct address for datablock corresponding to index and data block idx


	while (copy_count < length) {
		buf[copy_count] = dat_block_address[byte];		// copy over elements
		copy_count++;
		byte++;

		if (byte >= FOUR_KB) {		// moving onto next datablock
			byte = 0;
			data_block_idx++;		// set correct vals
			if (in_ptr->datablocks[data_block_idx] > dblock_num) {	// check for invalid datablock again
				return -1;
			}
			dat_block_address = (uint8_t *)(start + ((inode_num + 1 + in_ptr->datablocks[data_block_idx]) * FOUR_KB));		// Correct address for datablock corresponding to index and data block idx

		}

		if (copy_count + offset == in_ptr->length) {		// all have been copied
			break;
		}

	}

	return copy_count;
}

/* int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: - retval - aggregate value of offsets
 * Function: Given an fd, it looks through our filesystem for datablocks corresponding 
 * to that inode and offset value, and then updates our buffer with contents from datablocks using helper function
 *  */


int32_t fs_read(int32_t fd, void* buf, int32_t nbytes) {
	int retval;			// initialize correct values
	uint8_t* buffer = (uint8_t*) buf;
	pcb_t* pcb_ptr = (pcb_t*)(KERNEL_BOT - (cur_pid + 1) * EIGHT_KB);	// pcb pointer corresponding to current process id
	uint32_t inode_idx = pcb_ptr->fdesc[fd].inode;		// to pass into our helper function
	uint32_t offset = pcb_ptr->fdesc[fd].offset;
	retval = read_data(inode_idx, offset, buffer, nbytes);	// call to read data, which will copy into buffer from specified index
	pcb_ptr->fdesc[fd].offset += retval;		// add offset
	return retval;

}

/* int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: - retval - aggregate value of offsets or -1 on fail
 * Function: Given an fd, it looks through our directory for files corresponding 
 * to that fd value, and then updates our buffer with contents from datablocks using helper function
 *  */


int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
	pcb_t* pcb_ptr = (pcb_t*)(0x800000 - (cur_pid + 1) * EIGHT_KB);		// pcb pointer corresponding to current process id
	uint32_t offset = pcb_ptr->fdesc[fd].offset;		// to pass into our helper function
	int dentry_ct = *((int*) start);		// first 4 bytes corrspond to num dentries

	uint8_t* char_;

	struct dentry_t curr;

	if (offset >= dentry_ct) {		// value check
		return 0;
	}

	if (read_dentry_by_index(offset, &curr) == -1) {		// failed to read dentry at specified index
		return -1;		// fail
	}
	int i = 0;
	char_ = (uint8_t*)&curr;		// typecast
	while (i < MAX_NAME_LEN && char_[i] != '\0') {		// check for length and null character
		((uint8_t*)buf)[i] = char_[i];		// copy over to buffer
		i++;

	}

	pcb_ptr->fdesc[fd].offset++;	// increment offset
	return i;		// pass
}

/* int32_t fs_open(const uint8_t *filename);
 * Inputs: const uint8_t *filename
 * Return Value: - 0
 * Function: Opens file
 *  */


int32_t fs_open(const uint8_t *filename) {
	return 0;
}

/* int32_t fs_write(const uint8_t *filename);
 * Inputs: const uint8_t *filename
 * Return Value: - 0
 * Function: Opens file
 *  */


int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}

/* int32_t fs_close(const uint8_t *filename);
 * Inputs: const uint8_t *filename
 * Return Value: - 0
 * Function: Opens file
 *  */
int32_t fs_close(int32_t fd) {
	return 0;
}
/* int32_t dir_write(const uint8_t *filename);
 * Inputs: const uint8_t *filename
 * Return Value: - 0
 * Function: Opens file
 *  */


int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes) {
	return -1;
}
/* int32_t dir_close(const uint8_t *filename);
 * Inputs: const uint8_t *filename
 * Return Value: - 0
 * Function: Opens file
 *  */


int32_t dir_close(int32_t fd) {
	return 0;
}
/* int32_t dir_open(const uint8_t *filename);
 * Inputs: const uint8_t *filename
 * Return Value: - 0
 * Function: Opens file
 *  */


int32_t dir_open(const uint8_t *filename) {
	return 0;
}


