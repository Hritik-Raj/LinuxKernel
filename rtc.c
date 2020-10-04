#include "rtc.h"

// Macros used to determine whether virtualization or testing is on
#define VIRTUALIZED 0
#define TESTING_RATE 0

int rtc_interrupt_flag = 0;
int interrupt_counter = 0;
int virtual_freq = 0;

/* void rtc_init();
 * Inputs: None
 * Return Value: void
 *  Function: Enables IRQ at the the port 
 *  reserved for RTC, which is IRQ line 8, also sends appropriate values to RTC 
 *  Reference - https://wiki.osdev.org/RTC */
void rtc_init(){
  cli();
  outb(REG_B, RTC_PORT);		// select register B, and disable NMI
  char prev=inb(RTC_DATA);	// read the current value of register B
  outb(REG_B, RTC_PORT);		// set the index again (a read will reset the index to register D)
  outb(prev | 0x40, RTC_DATA);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
  enable_irq(RTC_IRQ);
  enable_irq(2);              // 2 because it corresponds to slave PIC
  sti();
}

/* void rtc_handler();
 * Inputs: None
 * Return Value: void
 * Function: Clears interrupt flags, 
 * Sends EOI to the PIC after character has been printed, and sets flags again.  
 * Reference - https://wiki.osdev.org/RTC */

void rtc_handler(){
  outb(REG_C, RTC_PORT);	// select register C
  inb(RTC_DATA);		// just throw away contents
  // test_interrupts();      // for testing the blinking
  send_eoi(RTC_IRQ);      // Send end of interrupt with correct irq number

  // testing what rate is being used if TESTING_RATE macro is 1
  if (TESTING_RATE) {
    interrupt_counter += 1;
    clear();
    printf("interrupt counter is: %d", interrupt_counter); 
  }
  
  // count the interrupts if virtualization is desired
  if (VIRTUALIZED) {

    // calculate the actual interrupt on which a virtualized interrupt should return to process
    int virtualized_interrupt = MAX_RATE / virtual_freq;
    interrupt_counter += 1;
    
    // if on desired interrupt, clear rtc_interrupt_flag and reset counter
    if (interrupt_counter == virtualized_interrupt) {
      rtc_interrupt_flag = 0;
      interrupt_counter = 0;
    }
  }

  // if not virtualized, just clear interrupt flag
  else {
    rtc_interrupt_flag = 0;
  }
}

/* void rtc_open();
 * Inputs: None
 * Return Value: 0
 *  Function: Resets frequency to 2Hz */
int32_t rtc_open() {
  
  // Reset freq to 2Hz -- low 4 bits of register set to 15
  int32_t freq = 2;
  int32_t* buf = &freq;

  rtc_write((const void*)buf, NBYTES);

  return 0;
}

/* void rtc_read();
 * Inputs: None
 * Return Value: 0
 *  Function: Returns 0 once interrupt has occured */
int32_t rtc_read() {

  // set flag, then wait until cleared by interrupt handler
  rtc_interrupt_flag = 1;
  while (rtc_interrupt_flag) { }

  // clear();
  // printf("read complete");
  return 0;
}

/* void rtc_write();
 * Inputs: buf -- containing interrupt frequency & nbytes -- containing number of bytes 
 * Return Value: 0 if successful, else -1
 *  Function: Sets interrupt rate according to value in buf if it is a power of two, and not more than 1024
 *  Reference - https://wiki.osdev.org/RTC */
int32_t rtc_write(const void* buf, int32_t nbytes) {
  
  // change the rtc frequency if virtualization is not desired
  if (!VIRTUALIZED) {
    // return unsuccessful (-1) if parameters not valid
    if (nbytes != NBYTES || buf == NULL) { 
      return -1; 
    }

    int32_t frequency = *(int32_t*)buf;
    
    if (frequency < MIN_RATE || frequency > MAX_RATE) {
      return -1;
    }

      // Check if freq is power of two. If not, return -1
    if ((frequency & (frequency - 1)) && (frequency > 0)) {
      return -1;
    }

    // calculate the number to be written to RegA by left shifting freq until it is equal to BASE_FREQ
    int32_t rate = 1;
    int32_t frequency_temp = frequency;
    while (frequency_temp != BASE_FREQ) {
      frequency_temp = frequency_temp << 1;
      rate += 1;
    }

    rate &= 0x0F;			// rate must be above 2 and not over 15
    disable_irq(RTC_IRQ);
    disable_irq(2);       
    outb(REG_A, RTC_PORT);		// set index to register A, disable NMI
    char prev = inb(RTC_DATA);	// get initial value of register A
    outb(REG_A, RTC_PORT);		// reset index to A
    outb((prev & 0xF0) | rate, RTC_DATA); //write only our rate to A. Note, rate is the bottom 4 bits. Preserves old A
    enable_irq(RTC_IRQ);
    enable_irq(2);              // 2 because it corresponds to slave PIC
  } 

  // set virtual freq (assuming buf is not a NULL ptr)
  else if (!(buf == NULL)){
    virtual_freq = *(int32_t*)buf;
  }

  return 0;

}

/* void rtc_close();
 * Inputs: None
 * Return Value: 0
 *  Function: Returns 0 */
int32_t rtc_close() {
  
  // TODO: (optional for cp2) virtualize rtc
  return 0;
}
