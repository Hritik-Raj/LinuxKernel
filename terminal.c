#include "terminal.h"

uint8_t buf[MAX_BUFFER_SIZEE]; // Global buf with max size 128
uint8_t enter_flag; // Global enter flag
uint8_t curr_index; // To track the index of buf

/* int32_t terminal_write(const unsigned char* buffer, int32_t nbytes);
 * Inputs: unsigned char* buffer, int32_t nbytes
 * Return Value: i-1 - amount of bytes written
 * Function: Writes to the keyboard the echoed message
 */
int32_t terminal_write(int32_t fd, const void* buffer, int32_t nbytes) {
    int i = 0;
    if (buffer == NULL) {
        return -1;
    }
    while (i < nbytes) { // While we are within desired bounds
    //   if(!buffer[i]){ // If a null character
    //     return i; // Retrun
    //   }
        putc_key(((unsigned char*)buffer)[i], i); // If not, print the char
        i++;
    }
    return i;
}
 
/* int32_t terminal_read(const unsigned char* buffer, int32_t nbytes);
 * Inputs: unsigned char * buffer, int32_t nbytes
 * Return Value: i- amount of bytes read
 * Function: Reads from the keyboard(terminal)
 */
int32_t terminal_read(int32_t fd, void * buffer, int32_t nbytes) {
    int i = 0;
    int j;

    if (buffer == NULL) {
        return -1;
    }

    if (nbytes > MAX_BUFFER_SIZEE) {
        nbytes = MAX_BUFFER_SIZEE;
    }
    while (enter_flag == 0); // Waits for enter to happen

    new_line(); // Makes a new line when enter hits

    while (i < nbytes) { // 0x1C is ENTER_P
        j = i;
        if (buf[i] == '\0') {
            break;
        }
        ((unsigned char*)buffer)[i] = buf[i]; // Transfers from the global buf
        buf[i] = NULL; // Then sets global buf to 0
        i++;

    }
    enter_flag = 0;
    curr_index = 0;

    return j;
}

/* int32_t terminal_open(const uint8_t * filename);
 * Inputs: const uint8_t * filename
 * Return Value: 0
 * Function: Opens the terminal
 */
int32_t terminal_open(const uint8_t * filename) {
    return 0;

}

/* int32_t terminal_close(int32_t fd);
 * Inputs: fd - file to close
 * Return Value: 0
 * Function: Closes the terminal
 */
int32_t terminal_close(int32_t fd) {
    return 0;

}
