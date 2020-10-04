#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "fs.h"
#include "keyboard.h"
#include "terminal.h"
#include "paging.h"

#define PASS 1
#define FAIL 0

#define FREQ_ABOVE_MAX 2048
#define FREQ_BELOW_MIN 1
#define NON_POW2_FREQ 100

void init_paging();

int PDT_addr;
int PT0_addr;
int PT1_addr;



/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;

	for (i = 0; i < 15; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	for (i = 16; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	/* Check if syscall entry is null */
	if ((idt[0x80].dpl != 0x3)){
		assertion_failure();
		result = FAIL;
	}
	return result;
}





/* int div_by_zero();
 * Exception handler, fails assertion if it encounters a divide by 0 error
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition  */

int div_by_zero() {
	TEST_HEADER;

	int result = FAIL;

	int a = 3;
	int b = 0;
	if (a / b) {
		result = FAIL;
	}

	return result;
}

/* int dereference_null();
 * Exception handler, fails assertion if it encounters a null pointer being dereferenced
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition  */
int dereference_null() {
	int result = PASS;

	idt_desc_t* a =  NULL;
	if (a->dpl) {
		result = FAIL;
	}

	return result;
}

/* Checkpoint 2 tests */

/* int terminal_test();
 * Terminal tester, will read and write for terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Terminal driver
 */
//  int terminal_test(){
//  	int32_t term_read;
//  	int32_t term_write;
//  	unsigned char buf[128]; // 128 is max size of buff
//  	memset(buf,0,128); // Sets buffer to all NULL chars
//  	printf("What is your name? "); // Asks for input
//  	term_read = terminal_read(buf, 128); // Calls term_read and return amt bytes read
//  	printf("Howdy, "); // Response printed
//  	term_write = terminal_write(buf, 128); // Calls term_write and returb amt bytes written
//  	printf("Read %d characters from terminal, and wrote %d characters to terminal.\n",term_read, term_write); // Status
//  	return PASS;
//  }

int test_rtc_read() {
	// if (rtc_read() == 0) {
	// 	return PASS;
	// }

	int32_t freq = 2;
	int32_t* buf = &freq;
	int32_t nbytes = 4;
	rtc_write(buf, nbytes);

	int i;
	for (i = 0; i < 10; i++) {
		rtc_read();
	}

	return PASS;
}

int test_rtc_open() {
	if (rtc_open() == 0) {
		return PASS;
	}

	return FAIL;
}

int test_rtc_close() {
	if (rtc_close() == 0) {
		return PASS;
	}

	return FAIL;
}

// int test_fs() {
// 	struct fs_data data = {0};
// 	uint8_t buf[40] = {0};
// 	fs_open((unsigned char *) "verylargetextwithverylongname.tx", &data);
// //printf("\n%u\n", data.current_inode);
// // 	printf("%u\n", data.type);
// 	fs_read(&data, buf, 10);
// 	printf("%s\n", buf);
// 	fs_read(&data, buf, 10);
// 	printf("%s\n", buf);
// 	fs_read(&data, buf, 10);
// 	printf("%s\n", buf);
// 	fs_read(&data, buf, 10);
// 	printf("%s\n", buf);
// 	fs_open((unsigned char *) ".", &data);
// 	while (1) {
// 		if (fs_read(&data, buf, 32) == 0) break;
// 		printf("%s\n", buf);
// 	}
// 	fs_open((unsigned char *) "frame0.txt", &data);
// 	int i = 0;
// 	while (1) {
// 		int s = fs_read(&data, buf, 1);
// 		if (s != 1) break;
// 		printf("%c", *buf);
// 		i++;
// 	}
// 	printf("%d\n", i);
// 	return PASS;
// }



int test_rtc_write_valid() {
	int result = PASS;

	// test from 2Hz to 1024Hz
	int i = MIN_RATE;
	while (i <= MAX_RATE) {
		int32_t freq = i;
		int32_t* buf = &freq;
		int32_t nbytes = 4;

		if (rtc_write(buf, nbytes) == -1) {
			result = FAIL;
		}

		// testing freqs that are power of 2s
		i = i << 1;
	}
	return result;
}

int test_freq_4Hz() {
	int32_t freq = 4;
	int32_t* buf = &freq;
	int32_t nbytes = 4;

	int result = rtc_write(buf, nbytes);
	if (result == -1) {
		return FAIL;
	}

	return PASS;

}

int test_rtc_write_invalid() {
	int32_t freq = FREQ_ABOVE_MAX;
	int32_t* buf = &freq;
	int32_t nbytes = 4;

	// test with a frequency that is a power-of-two but above valid range
	if (rtc_write(buf, nbytes) != -1) {
		printf("RTC Write should have returned -1 for frequency outside valid range.");
		return FAIL;
	}

	// test with a frequency that is a power-of-two but below valid range
	freq = FREQ_BELOW_MIN;
	buf = &freq;

	if (rtc_write(buf, nbytes) != -1) {
		printf("RTC Write should have returned -1 for frequency outside valid range.");
		return FAIL;
	}

	freq = NON_POW2_FREQ;
	buf = &freq;

	// test with a frequency that is not a power-of-two but within valid range
	if (rtc_write(buf, nbytes) != -1) {
		printf("RTC Write should have returned -1 for frequency that is not power of two.");
		return FAIL;
	}

	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("Filesystem", test_fs());
	// TEST_OUTPUT("Terminal test", terminal_test());
	// TEST_OUTPUT("idt_test", idt_test());
	//printf("Test Paging");	 // Peter added this
	//TEST_OUTPUT("div_by_zero", div_by_zero());
	//printf("Test Paging");
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("dereference_null", dereference_null());
	// printf("Test rtc driver\n");
	// TEST_OUTPUT("rtc_test_open", test_rtc_open());
	// TEST_OUTPUT("rtc_test_write_invalid", test_rtc_write_invalid());
	// TEST_OUTPUT("rtc_test_write_valid", test_rtc_write_valid());
	// TEST_OUTPUT("test_freq_4Hz", test_freq_4Hz());
	// TEST_OUTPUT("rtc_test_read", test_rtc_read());
	// TEST_OUTPUT("rtc_test_close", test_rtc_close());
	// launch your tests here
}
