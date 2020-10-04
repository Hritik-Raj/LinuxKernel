#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "keyboard.h"
#include "lib.h"


#define MAX_BUFFER_SIZEE 128

extern int32_t terminal_write(int32_t fd,const void* buffer, int32_t nbytes);

extern int32_t terminal_read(int32_t fd, void * buffer, int32_t nbytes);

extern int32_t terminal_open(const uint8_t * filename);

extern int32_t terminal_close(int32_t fd);



#endif /* _LIB_H */
