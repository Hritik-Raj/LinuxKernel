/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFB; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */
#define MASTER_DATA (MASTER_8259_PORT + 1) // Defined the ports to be used
#define SLAVE_DATA (SLAVE_8259_PORT + 1)


/* Initialize the 8259 PIC */

/* void i8259_init(void);
 * Inputs: void
 * Return Value: void
 *  Function: Initializes the PIC with the control words, 
 * tells the PIC how the master and slave ports are wired to each other */
void i8259_init(void) {

  // Clears
  outb(CLEAR, MASTER_DATA);
  outb(CLEAR, SLAVE_DATA);

  outb(ICW1, MASTER_8259_PORT); // Setting ICW1
  outb(ICW1, SLAVE_8259_PORT);

  outb(ICW2_MASTER, MASTER_DATA); // Setting ICW2
  outb(ICW2_SLAVE, SLAVE_DATA);

  outb(ICW3_MASTER, MASTER_DATA); // Setting ICW3
  outb(ICW3_SLAVE, SLAVE_DATA);

  outb(ICW4, MASTER_DATA); // Setting ICW4
  outb(ICW4, SLAVE_DATA);

  outb(master_mask, MASTER_DATA); // Setting masks to be used below
  outb(slave_mask, SLAVE_DATA);
}

/* Enable (unmask) the specified IRQ */

/* void enable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num - Interrupt to enable
 * Return Value: void
 *  Function: Enables the selected interrupt line, 
 *  based on whether it is a master or slave */
void enable_irq(uint32_t irq_num) {
  uint8_t en_mask = 0x01; // Set the mask to be LSB (Mask all bits except the LSB)

  // If irq num is in master range
    if(irq_num <= 7){                  // 0 - 7 corresponds to master irq values
      en_mask = ~(en_mask << irq_num); // Left shift and not the bits because active low
      master_mask &= en_mask;           // AND 0xFB with the shifted mask to get the value we need
      outb(master_mask, MASTER_DATA); // Writing the mask
  }

  // If irq num is in slave range
  else if(irq_num > 7 && irq_num <= 15){    // 8 - 15 corresponds to slave irq numbers
      en_mask = ~(en_mask << (irq_num-8)); // Subtract 8 to get into "range" of master
      slave_mask &= en_mask;                // AND 0xFF with the shifted mask to get the value we need
      outb(slave_mask, SLAVE_DATA); // Writing the mask
  }
}

/* Disable (mask) the specified IRQ */

/* void disable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num - Interrupt to disable
 * Return Value: void
 *  Function: Disables the selected interrupt line, 
 *  based on whether it is a master or slave 
 *  We also take care of the fact that it is active low */
void disable_irq(uint32_t irq_num) {
  uint8_t dis_mask = 0x01; // Set the mask to be LSB (Mask all bits except the LSB)

  // If irq num is in master range
  if(irq_num >= 0 && irq_num <= 7){       // 0 - 7 corresponds to master irq values
      dis_mask = (dis_mask << irq_num); // Left shift by irq num
      master_mask |= dis_mask; // OR 0xFB with the shifted value of dis_mask
      outb(master_mask, MASTER_DATA); // Writing the mask
  }

  // If irq num is in slave range
  else if(irq_num > 7 && irq_num <= 15){      // 8 - 15 corresponds to slave irq numbers
      dis_mask = ~(dis_mask << (irq_num-8)); // Subtract 8 to get into "range" of master
      slave_mask |= dis_mask;                 // OR 0xFF with the shifted value of dis_mask
      outb(slave_mask, SLAVE_DATA); // Writing the mask
  }

}

/* Send end-of-interrupt signal for the specified IRQ */

/* void send_eoi(uint32_t irq_num);
 * Inputs: uint32_t irq_num - Interrupt line we want to send EOI to
 * Return Value: void
 * Function: Sends an End of Interrupt signal based on whether 
 * the PIC is a master or slave. If it is a master, it is enough to just
 * send the interrupt to the master PIC, if it is a slave, EOI must be sent to
 * both master and slave PICs*/
void send_eoi(uint32_t irq_num) {

  // If irq num is in master range
  if(irq_num >= 0 && irq_num <= 7){          // 0 - 7 corresponds to master irq values
      outb(EOI | irq_num, MASTER_8259_PORT); // Writing the mask
  }

  // If irq num is in slave range
  else if(irq_num > 7 && irq_num <= 15){      // 8 - 15 corresponds to slave irq numbers
      outb(EOI | (irq_num - 8), SLAVE_8259_PORT); // Writing the mask (Subtract 8 to get in "range" of the master)
      outb(EOI | 0x02, MASTER_8259_PORT); // Since slave is connected to master ( 0x02 because that is the connection between slave and master)
  }
}
