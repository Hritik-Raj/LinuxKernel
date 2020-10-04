#ifndef SYS_H
#define SYS_H

#include "fs.h"
#include "lib.h"
#include "x86_desc.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"

#define FOPS_SIZE 4 // Size of our fops table
#define MAX_FDESC_NUM 8 // Total number of file descriptors

#define EXE_SIZE 4
#define EIGHT_KB_SIZE 0xFFFFE000
#define TYPE_RTC 0 // Case for RTC, DIR, and FILE
#define TYPE_DIR 1
#define TYPE_FILE 2
#define NUM_PROC 8
#define FOPS_NUM 4
#define EIGHT_MB 0x800000
#define PDT_VID_IDX 32
#define PDT_VID_ATTRIB 0x97
#define FNAME_SIZE 32
#define SP_VAL 138412028
#define MAX_FD 7
#define MIN_FD 2
#define FOUR_BYTE 4
#define LOAD 0x08048000
#define ONETWENTYEIGHT_MB 0x8000000
#define ONETHIRTYTWO_MB 0x8400000

typedef struct fd_t{ // This is our file descriptor struct, used for sys calls
  int32_t (*jumptable[4])(); // Jumptable for the fops pointers
  int32_t offset; // Used to track paging
  int32_t inode;
  int32_t flags;
} fd_t;

typedef struct pcb_t{ // This is our pcb struct, used for sys calls
  fd_t fdesc[MAX_FDESC_NUM]; // List of file descriptors for a pcb, size 8
  uint8_t fname[MAX_FDESC_NUM][32]; // 2d array to hold filenames
  uint32_t parent_pid; // Parent process id number
  uint32_t curr_pid; // Current process id number
  uint8_t args[100]; // Our arg buffer
  uint32_t par_kbp; // Parent base pointer for kernel process
  uint32_t par_ksp; // Parent stack pointer for kernel process
  uint32_t par_esp; // Parent esp
  uint32_t par_ebp; // parent ebp
  uint32_t par_esp0; // Parent's esp0 for TLB
  uint8_t par_pro_num; // Parent's process number, used to track return PCB
  uint8_t cur_pro_num; // Current process number, used to track used PCBs
  int cur_child; // True/false to determine if cur process has a child
  int retval;     // probably needed?
} pcb_t;

// Sys.c function prototypes
int32_t bad_call ();
pcb_t * setup_PCB(int32_t new_pid);
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

#endif /* SYS_H */
