.global rtc_wrapper, keyboard_wrapper



/* rtc_wrapper */
/* Inputs: None */
/* Return Value: void */
/*  Function: Saves registers, calls C function from rtc.c, pops registers and returns */

rtc_wrapper:
  pushal          /* save all registers */
  call rtc_handler    /* call to the C function */
  popal           /* pop all the registers */
  iret            /* control returned */


/* keyboard_wrapper */
/* Inputs: None */
/* Return Value: void */
/* Function: Saves registers, calls C function from keyboard.c, pops registers and returns */

keyboard_wrapper:
  pushal                 /* save all registers */
  call keyboard_handler     /* call to the C function */
  popal                   /* pop all the registers */
  iret                  /* control returned */
