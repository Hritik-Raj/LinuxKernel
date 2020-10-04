#include "keyboard.h"
#include "idt.h"

extern uint8_t buf[MAX_BUF];

uint32_t chars_num;
volatile uint32_t terminal_ready;
// flag 0 - shift
// flag 1 - caps
// flag 2 - control
// flag 3 = alt
static uint8_t flags = 0; // flag to specify which function key has been pressed

// volatile uint8_t enter_flag;
extern volatile uint8_t enter_flag;

// volatile uint8_t  curr_buf[MAX_BUF];


// static uint8_t curr_index = 0;
uint8_t curr_index = 0;

static volatile uint8_t printkey;

uint8_t standard_keyboard[MAX_BUF] =
{
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'q', 'w', 'e', 'r', /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
  0,      /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,    /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',      /* 49 */
  'm', ',', '.', '/',   0,        /* Right shift */
	'*',
  0,  /* Alt */
  ' ',  /* Space bar */
  0,  /* Caps lock */
  0,  /* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
  0,  /* < ... F10 */
  0,  /* 69 - Num lock*/
  0,  /* Scroll Lock */
  0,  /* Home key */
  0,  /* Up Arrow */
  0,  /* Page Up */
	'-',
  0,  /* Left Arrow */
	0,
  0,  /* Right Arrow */
	'+',
  0,  /* 79 - End key*/
  0,  /* Down Arrow */
  0,  /* Page Down */
  0,  /* Insert Key */
  0,  /* Delete Key */
	0,   0,   0,
  0,  /* F11 Key */
  0,  /* F12 Key */
  0,  /* All other keys are undefined */
};



uint8_t shift_keyboard[MAX_BUF] =
{
	0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
	0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
	'*',
	0,	/* Alt */
  ' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};


uint8_t caps_keyboard[MAX_BUF] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',	/* Enter key */
	0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', ',', '.', '/',   0,				/* Right shift */
	'*',
	0,	/* Alt */
  ' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

uint8_t alt_keyboard[MAX_BUF] =
{
  0,  27,  120, 121, 122, 123, 124, 125, 126, 127, /* 9 */
  128, 129, 130, 131, 14, /* Backspace */
  165,     /* Tab */
  16, 17, 18, 19, /* 19 */
  20, 21, 22, 23, 24, 25, 26, 27, 28, /* Enter key */
  0,      /* 29   - Control */
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, /* 39 */
  40, 41,   0,    /* Left shift */
  43, 44, 45, 46, 47, 48, 49,      /* 49 */
  50, 51, 52, 53,   0,        /* Right shift */
	'*',
  0,  /* Alt */
  ' ',  /* Space bar */
  0,  /* Caps lock */
  104,  /* 59 - F1 key ... > */
	105,   106,   107,   108,   109,   110,   111,   112,
  113,  /* < ... F10 */
  0,  /* 69 - Num lock*/
  0,  /* Scroll Lock */
  0,  /* Home key */
  0,  /* Up Arrow */
  0,  /* Page Up */
	'-',
  0,  /* Left Arrow */
	0,
  0,  /* Right Arrow */
	'+',
  0,  /* 79 - End key*/
  0,  /* Down Arrow */
  0,  /* Page Down */
  0,  /* Insert Key */
  0,  /* Delete Key */
	0,   0,   0,
  139,  /* F11 Key */
  140,  /* F12 Key */
  0,  /* All other keys are undefined */
};


uint8_t caps_shift_keyboard[MAX_BUF] =
{
	0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',	/* Enter key */
	0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', '<', '>', '?',   0,				/* Right shift */
	'*',
	0,	/* Alt */
  ' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

uint8_t control_keyboard[MAX_BUF] =
{
	0,  27, '!', '@', '#', '$', '%', 30, '&', '*',	/* 9 */
  '(', ')', 31, '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  17, 23, 5, 18,	/* 19 */
  20, 25, 21, 9, 15, 16, 27, 29, '\n',	/* Enter key */
	0,			/* 29   - Control */
  1, 19, 4, 6, 7, 8, 10, 11, 12, ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 28, 26, 24, 3, 22, 2, 14,			/* 49 */
  13, '<', '>', '?',   0,				/* Right shift */
	'*',
	0,	/* Alt */
  ' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};


/* void keyboard_init();
 * Inputs: None
 * Return Value: void
 *  Function: Enables IRQ at the the port
 *  reserved for Keyboard, which is IRQ line 1 */
void keyboard_init() {
  enter_flag = 0;
  curr_index = 0;
	enable_irq(KEYBOARD_IRQ);			// Enables irq
}

/* void helper_print();
* Inputs: scan - scancode of the key pressed
* Return Value: void
*  Function: adds the corresponding letters/ values from key presses into the buffer */
void helper_print(uint8_t scan) {
  uint8_t mask = 0x80; // variable to map scancode to correct array
  if((mask & scan) & (scan | mask)){
    return;
  }
  else{
    if (flags == SHIFT_FLAG) {
  		printkey = shift_keyboard[scan]; // use shift array if flag corresponds to shift being pressed
      add_curr_buf(printkey);
  	}

  	else if (flags == CAPS_FLAG) {
  		printkey = caps_keyboard[scan]; // use capslock array if flag corresponds to capslock being pressed
      add_curr_buf(printkey);
  	}

  	else if (flags == CTRL_FLAG) {
  		printkey = control_keyboard[scan]; // use control array if flag corresponds to control being pressed
      add_curr_buf(printkey);
  	}

  	else if (flags == ALT_FLAG) {
  		printkey = alt_keyboard[scan]; // use alt array if flag corresponds to alt being pressed
      add_curr_buf(printkey);
  	}

  	else if (flags == CAPS_SHIFT_FLAG) {
  		printkey = caps_shift_keyboard[scan]; // use caps_shift array if flag corresponds to caps_shift being pressed
      add_curr_buf(printkey);
  	}

  	if (flags == 0) { // if flag = 0, no special key has been pressed
  		printkey = standard_keyboard[scan]; // use normal array if flag corresponds to no special function keys being pressed
      add_curr_buf(printkey);
  	}

  	putc_key(printkey, curr_index); // use putc_key to print character to screen
  }


}

//* void key_helper();
//  * Inputs: scancode - scancode of the current key pressed
//  * Return Value: void
//  *  Function: Enables IRQ at the the port
//  *  reserved for Keyboard, which is IRQ line 1 */
void key_helper(uint8_t scancode) {
	int j;
  if(curr_index >= 128 && scancode != BACKSPACE_P){ // to avoid putting garbage values into buffer
    return;
  }
	switch (scancode) { // switch statements based on scancode
		case CAPS_P:
			flags ^= 0x02; // XOR flags with hex 2  to handle future caps lock presses
      		break;
		case L_SHIFT_P:
			flags |= 0x01; // bitmask the first bit of flags to handle shift key being pressed
     		break;
		case R_SHIFT_P:
			flags |= 0x01; // bitmask the first bit of flags to handle shift key being pressed
      		break;
		case L_SHIFT_U:
			flags &= ~0x01; // negate flags and and it with 0x01 to handle shift being unpressed
      		break;
		case R_SHIFT_U:
			flags &= ~0x01; // negate flags and and it with 0x01 to handle shift being unpressed
      		break;
		case L_CTR_P:
			flags |= 0x04; // OR flags with 0x04 to handle control key being pressed
      		break;
		case L_CTR_U:
			flags &= ~0x04; // negate flags and and it with 0x04 to handle control being unpressed
      		break;
		case L_ALT_P:
			flags |= 0x08; // bitmask the third (0 indexed) bit of flags to handle alt key being pressed
      		break;
		case L_ALT_U:
			flags &= ~0x08; // negate flags and and it with 0x08 to handle alt being unpressed
      		break;
    case TAB_P: // define this
      helper_print(TAB_KEY); // Print four spaces for tab
      break;
    case ENTER_P:
      enter_flag = 1; // change the volatile enter flag so that terminal knows when command is over
      buf[curr_index] = '\n'; // This sets the new line escape code
      enter();
      curr_index = 0; // reset number of items in buffer
      break;
      case BACKSPACE_P:
      if(curr_index > 0){ // decrease number of items in buffer
        curr_index--;
        buf[curr_index] = '\0';
        handle_backspace(curr_index); // helper function call
      }
      break;
		default:
      // HANDLE ALL REGULAR CASES HERE - SHIFT VS CAPS
      if(scancode >= 0x02 && scancode <= 0x39){ // >= 0x2 and <= 0x39 handles all the normal keys (does not include function keys)
        helper_print(scancode);
      }

      if (flags & CTRL_FLAG) {
		  if (scancode == L_KEY) {
			  for (j = 0; j < MAX_BUF; j++) {
				  buf[j] = 0; // if ctrl + alt is pressed, reset the screen, set everything to 0
			  }
			  curr_index = 0; // reset the number of elements in buffer
			  handle_ctrl_l();
		    }
	   }
		  break;
	}
}

// /* void add_curr_buf();
//  * Inputs: scan - letter from the key press
//  * Return Value: void
//  *  Function: Checks if buffer is not full, then puts the letter into the buffer */
void add_curr_buf(uint8_t scan){
  if(curr_index < MAX_BUF){
    buf[curr_index] = scan; // Puts in the elements
    curr_index++;
  }
}
// /* void keyboard_handler();
//  * Inputs: None
//  * Return Value: void
//  *  Function: Disables and enables irq, takes data in from keyboard port, and calls helper function */
void keyboard_handler() {
  disable_irq(KEYBOARD_IRQ); // disables the IRQ flag
  send_eoi(KEYBOARD_IRQ); // send eoi

	uint8_t keycode;
	keycode = inb(KEYBOARD_PORT); // Gets data from keyboard
	key_helper(keycode); // Calls to the helper
  enable_irq(KEYBOARD_IRQ); // Then enables irq
}
