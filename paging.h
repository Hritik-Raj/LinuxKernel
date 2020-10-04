#ifndef _PAGING_H
#define _PAGING_H

#include "paging_helper.h"
#include "lib.h"
#include "types.h"

#define ONE_KB 1024
#define FOUR_KB 4096
#define VIDEO_ADD 0xB8000
#define ATTRIB_VAL 7
#define PT_VAL 184
#define FOUR_MB 0x400000
#define FOUR_MB_ATTRIB 0x83
#define VID_VAL  0xB8

extern int PDT_addr;
extern int PT0_addr;
extern int PT1_addr;



typedef struct page_table {		// struct definition
	int entry[ONE_KB];
} page_table;

extern void init_paging();

#endif /*_PAGING_H */
