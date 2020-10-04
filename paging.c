#include "paging.h"

static page_table PDT __attribute__((aligned(FOUR_KB)));			// Alignment for 4kb, refernce osdev
static page_table PT0 __attribute__((aligned(FOUR_KB)));
static page_table PT1 __attribute__((aligned(FOUR_KB)));



int PDT_addr = (int)&PDT;			// To be used in other functions 
int PT0_addr = (int)&PT0;
int PT1_addr = (int)&PT1;

/* void init_paging();
 * Inputs: NONE
 * Return Value: NONE
 *  Function: Initializes the Page directory with the page 
 * 				tables and sets the correct bits to their appropriate values
 * 				enables 4mb and 4kb pages
 */

void init_paging() {

	int i;
    uint32_t page_d;
	for (i = 0; i< ONE_KB; i++) {
		PDT.entry[i] = 0x2;					// "OR" it with 2 to Set to not present, and enable r/w bit
		PT0.entry[i] = (i * FOUR_KB) | 0x3;				//"OR" it with 3 to Set page entries to present, and enable r/w
		PT1.entry[i] = 0;								// initialize video page table
		if (i == PT_VAL) {									// index of entry in page table 1
			PT0.entry[i] = (PT0.entry[i] | 0x1);		// "OR" it with 1 to  set page to present 
		}
	}
	PT1.entry[0] = VIDEO_ADD | ATTRIB_VAL;				// VIDEO is 0xB8000, Attrib val is 7, to set the present, r/w and supervisor bits
	PDT.entry[0] = ((unsigned int)PT0.entry) | 0x3;		// Put first page in the PDT, and "OR" it with 3 to set page entries to present, and enable r/w
	PDT.entry[1] = FOUR_MB | FOUR_MB_ATTRIB;				// 4 MB for the starting address, "OR"d with 0x83 to set the proper bits 
	PT0.entry[VID_VAL] |= 3; 								// At entry corresponding to video address, "OR" it with 3 to set the present, r/w and supervisor bits
    page_d = (uint32_t)(PDT.entry);
    paging1_helper(page_d);					// Call to assembly function
}
