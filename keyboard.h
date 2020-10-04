#ifndef _KEYBOARD_H
#define _KEYBOARD_H


#include "lib.h"
#include "i8259.h"

#define KEYBOARD_IRQ 1					// Keyboard corresponds to IRQ line 1
#define KEYBOARD_PORT 0x60				// Port to get keyboard press data from
#define KEYS_CP1      0x35				// (Temporary fix ) We only want to print first 35 characters, because keyboard releases are not accounted for
#define MAX_BUF 128

void keyboard_init ();                  // prototypes for functions
void keyboard_handler();
void add_curr_buf(uint8_t scan);
void helper_print(uint8_t scan);
void key_helper(uint8_t scancode);

#define L_SHIFT_P   0x2A                // hex values for key presses
#define L_SHIFT_U   0xAA
#define R_SHIFT_P   0x36
#define R_SHIFT_U   0xB6
#define CAPS_P      0x3A
#define L_ALT_P     0x38
#define L_ALT_U     0xB8
#define L_CTR_P     0x1D
#define L_CTR_U     0x9D
#define TAB_P       0x0F
#define TAB_U       0x8F
#define BACKSPACE_P 0x0E
#define BACKSPACE_U 0x8E
#define ENTER_P     0x1C
#define ENTER_U     0x9C
#define L_P         0x26
#define TAB_P       0x0F
#define SHIFT_FLAG  1
#define CAPS_FLAG   2
#define CTRL_FLAG   4
#define ALT_FLAG    8
#define CAPS_SHIFT_FLAG 3
#define TAB_KEY  0x39
#define L_KEY    0x26

#endif /* _KEYBOARD_H */
