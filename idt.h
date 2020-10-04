#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "keyboard.h"
#include "rtc.h"
#include "wrapper.h"
#include "sys_wrapper.h"
#include "sys.h"

// #include "../syscalls/syscall.h"

#define INTERRUPT_START 32

/*
 * Fill the IDT with default decriptors, with the exception of the system call descriptors, exceptions, and some interrupts.
 * Returns: void
 * Side-effects: fills the idt table
 */
extern void setup_idt();
void halt_helper();
// extern void exception_handler(int exc);


#endif /* _IDT_H */
