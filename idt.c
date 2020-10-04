#include "idt.h"

#define EXCEPTION_NUM 0x14
#define SYSCALL_IDX 0x80
#define INTERRUPT_START 32

/* Mnemonic Exceptions */
#define DE 0
#define DB 1
#define NMI 2
#define BP 3
#define OF 4
#define BR 5
#define UD 6
#define NM 7
#define DF 8
#define CPO 9
#define TS 10
#define NP 11
#define SS 12
#define GP 13
#define PF 14
#define MF 16
#define AC 17
#define MC 18
#define XF 19
#define KH 21
// #define RTCH 0x28

/* Local functions used for IDT setup */

/* void default_handler();
 * Inputs: none
 * Return Value: none
 * Function: Handler for undefined interrupts. */

static void set_exceptions();
static void set_interrupts();
static void set_rtc();
static void set_syscall();


/* void default_handler();
 * Inputs: none
 * Return Value: none
 * Function: Handler for undefined interrupts. */
void default_handler() {
    cli();
    printf("Interrupt not set.");
    halt_helper();
    sti();
}

/* Exception Handlers */
void DE_handler() {
    printf("Divide Error!");
    halt_helper();

}
void DB_handler() {
    printf("Reserved Intel Interrupt");
    halt_helper();
}
void NMI_handler() {
    printf("Non-maskable Interrupt!");
    halt_helper();
}
void BP_handler() {
    printf("Breakpoint!");
    halt_helper();
}
void OF_handler() {
    printf("Overflow!");
    halt_helper();
}
void BR_handler() {
    printf("BOUND Range Exceeded!");
    halt_helper();
}
void UD_handler() {
    printf("Invalid Opcode!");
    halt_helper();
}
void NM_handler() {
    printf("Device Not Available!");
    halt_helper();
}
void DF_handler() {
    printf("Double Fault!");
    halt_helper();
}
void CPO_handler() {
    printf("Coprocessor Segment Overrun :( ");
    halt_helper();
}
void TS_handler() {
    printf("Invalid TSS!");
    halt_helper();
}
void NP_handler() {
    printf("Segment Not Present!");

}
void SS_handler() {
    printf("Stack-Segment Fault!");
    halt_helper();
}
void GP_handler() {
    printf("General Protection!");
    halt_helper();
}
void PF_handler() {
    printf("Page Fault!");
    halt_helper();
}
void MF_handler() {
    printf("x87 FPU Floating-Point Error!");
    halt_helper();
}
void AC_handler() {
    printf("Alignment Check!");
    halt_helper();
}
void MC_handler() {
    printf("Machine Check!");
    halt_helper();
}
void XF_handler() {
    printf("SIMD Floating-Point Exception!");
    halt_helper();
}

// static void* exception_handler(int exc);

/* Set the correct values for the system call descriptor. */



/* static void set_exceptions();
 * Inputs: none
 * Return Value: none
 * Function: Puts exceptions into IDT */

/* static void set_exceptions();
 * Inputs: none
 * Return Value: none
 * Function: Puts exceptions into IDT */
static void set_exceptions() {
    int i;

    for (i = 0; i < EXCEPTION_NUM; i++) {
        idt[i].present       = 0x1;
        idt[i].dpl           = 0x0;
        idt[i].reserved0     = 0x0;
        idt[i].size          = 0x1;
        idt[i].reserved1     = 0x1;
        idt[i].reserved2     = 0x1;
        idt[i].reserved3     = 0x1; // Most exceptions use trap gates
        idt[i].reserved4     = 0x0;
        idt[i].seg_selector  = KERNEL_CS;

         if(i == 2){            // Reserved3 changes based on what exception is being called, according to documentation
             idt[i].reserved3 = 0x0;
         }
        //SET_IDT_ENTRY(idt[i], arr[i]);

    }

    //PUT ALL THESE IN ARRAY AND CALL LOOP TO DO IT
    // set handlers for exceptions
    SET_IDT_ENTRY(idt[DE], DE_handler);
    SET_IDT_ENTRY(idt[DB], DB_handler);
    SET_IDT_ENTRY(idt[NMI], NMI_handler);
    SET_IDT_ENTRY(idt[BP], BP_handler);
    SET_IDT_ENTRY(idt[OF], OF_handler);
    SET_IDT_ENTRY(idt[BR], BR_handler);
    SET_IDT_ENTRY(idt[UD], UD_handler);
    SET_IDT_ENTRY(idt[NM], NM_handler);
    SET_IDT_ENTRY(idt[DF], DF_handler);
    SET_IDT_ENTRY(idt[CPO], CPO_handler);
    SET_IDT_ENTRY(idt[TS], TS_handler);
    SET_IDT_ENTRY(idt[NP], NP_handler);
    SET_IDT_ENTRY(idt[SS], SS_handler);
    SET_IDT_ENTRY(idt[GP], GP_handler);
    SET_IDT_ENTRY(idt[PF], PF_handler);
    SET_IDT_ENTRY(idt[MF], MF_handler);
    SET_IDT_ENTRY(idt[AC], AC_handler);
    SET_IDT_ENTRY(idt[MC], MC_handler);
    SET_IDT_ENTRY(idt[XF], XF_handler);
}

/* static void set_interrupts();
 * Inputs: none
 * Return Value: none
 * Function: Puts interrupts into IDT */
static void set_interrupts() { // This needs to change
  int i;
  i = 0x21;                 // Keyboard Interrupts need to be put into IDT at 0x21 according to documentation
  idt[i].present       = 0x1; // This needs to be 1
  idt[i].dpl           = 0x0;
  idt[i].reserved0     = 0x0;
  idt[i].size          = 0x1;
  idt[i].reserved1     = 0x1;
  idt[i].reserved2     = 0x1;
  idt[i].reserved3     = 0x0; // If interrupt or trap gate
  idt[i].reserved4     = 0x0;
  idt[i].seg_selector  = KERNEL_CS;
  SET_IDT_ENTRY(idt[i], keyboard_wrapper);
}

/* static void set_syscall();
 * Inputs: none
 * Return Value: none
 * Function: Puts rtc interrupt entry into IDT at 0x28 */

static void set_rtc() { // This needs to change
  uint8_t i;
  i = 0x28;                     // RTC Interrupts need to be put into IDT at 0x21 according to documentation
  idt[i].present       = 0x1; // This needs to be 1
  idt[i].dpl           = 0x0;
  idt[i].reserved0     = 0x0;
  idt[i].size          = 0x1;
  idt[i].reserved1     = 0x1;
  idt[i].reserved2     = 0x1;
  idt[i].reserved3     = 0x0; // If interrupt or trap gate
  idt[i].reserved4     = 0x0;
  idt[i].seg_selector  = KERNEL_CS;
  SET_IDT_ENTRY(idt[i], rtc_wrapper);
}

/* static void set_syscall();
 * Inputs: none
 * Return Value: none
 * Function: Puts syscall entry into IDT at 0x80 */
static void set_syscall() {
        idt[SYSCALL_IDX].present       = 0x1;
        idt[SYSCALL_IDX].dpl           = 0x3;
        idt[SYSCALL_IDX].reserved0     = 0x0;
        idt[SYSCALL_IDX].size          = 0x1;
        idt[SYSCALL_IDX].reserved1     = 0x1;
        idt[SYSCALL_IDX].reserved2     = 0x1;
        idt[SYSCALL_IDX].reserved3     = 0x1;
        idt[SYSCALL_IDX].reserved4     = 0x0;
        idt[SYSCALL_IDX].seg_selector  = KERNEL_CS;

        SET_IDT_ENTRY(idt[SYSCALL_IDX], sys_wrap);
}

/* static void setup_idt();
 * Inputs: none
 * Return Value: none
 * Function: Puts entries into IDT accordingly */

void setup_idt() {

    lidt(idt_desc_ptr);
    // initialize the idt descriptors for exceptions
    set_exceptions();

    int i;
    for (i = EXCEPTION_NUM; i < NUM_VEC; i++) {

        if (i == 0x21) {            // If interrupt corresponds to keyboard (0x21)
            set_interrupts();
        }
        else if(i == 0x28){         // If interrupt corresponds to RTC (0x28)
          set_rtc();
        }
        else if (i == SYSCALL_IDX) {
            set_syscall();
        }

        // initialize a idt descriptor with present bit set to 0
        else {
                idt[i].present       = 0x0; // changed from hiraald's 0x1 - aditya directed this change
                idt[i].dpl           = 0x0;
                idt[i].reserved0     = 0x0;
                idt[i].size          = 0x1;
                idt[i].reserved1     = 0x1;
                idt[i].reserved2     = 0x1;
                idt[i].reserved3     = 0x1;
                idt[i].reserved4     = 0x0;
                idt[i].seg_selector  = KERNEL_CS;

                if (i >= INTERRUPT_START) {
                    idt[i].reserved3     = 0x0;
                }

                SET_IDT_ENTRY(idt[i], default_handler);

        }
    }
}

/* static void halt_helper();
 * Inputs: none
 * Return Value: none
 * Function: Puts newline onto screen and halts */
void halt_helper() {
    putc_key('\n', NULL);
    halt(0);
}
