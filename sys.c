
#include "sys.h"

int32_t pid_flag[NUM_PROC]; // Used to store all of our processes
int32_t cur_pid; // Used to track the current pid
int PDT_addr; // The page tablr address to offset the pcb entries
uint8_t argbuf[128]; // 128 is the max size for the local arg (command)


// All of the following are used as jumptable pointers to the possible file types
int32_t(*stdin_fops[FOPS_NUM])() = {bad_call, terminal_read, 0, bad_call};
int32_t(*stdout_fops[FOPS_NUM])() = {bad_call, 0, terminal_write, bad_call};
int32_t(*rtc_fops[FOPS_NUM])() = {rtc_open, rtc_read, rtc_write, rtc_close};
int32_t(*file_fops[FOPS_NUM])() = {fs_open, fs_read, fs_write, fs_close};
int32_t(*dir_fops[FOPS_NUM])() = {dir_open, dir_read, dir_write, dir_close};


/* int32_t bad_call();
 * Inputs: None
 * Return Value: -1
 *  Function: Used as a garbage function for the stdin and stdout
 */
int32_t bad_call(){
  return -1;
}


/* int32_t halt(uint8_t status);
 * Inputs: uint8_t status
 * Return Value: 0
 *  Function: Halts the status that is being passed in
 */
int32_t halt (uint8_t status){
  int i;

  uint8_t* shell = (uint8_t*)"shell"; // Used to default call the shell if no other processes are used

  pcb_t *pcb_ptr = (pcb_t*)(EIGHT_MB - EIGHT_KB*(cur_pid + 1)); // Finds a new pcb to use with the cur_pid offset
  pcb_t *pcb_parent = (pcb_t*)(EIGHT_MB - EIGHT_KB* (pcb_ptr->parent_pid + 1)); // Keeps track of the parent pcb

  pcb_parent->retval = (uint32_t)status; // Sets its retval to keep track of return from user program

  for (i = 0; i < NUM_PROC; i++) { // Goes thru all fdesc indices and closes if in use
    if ((pcb_ptr->fdesc[i]).flags == 1) {   // check if we need to manually modify flags for others as well
      close(i); // Call to close
    }
  }

  pid_flag[cur_pid] = 0; // Then reset the global pid array

  if(cur_pid == pcb_ptr->parent_pid){ // If theres no user programs, only shell
    execute(shell); // Keep executing shell
  }

  // Inline assembly for exit needed here????
  cur_pid = pcb_ptr->parent_pid; // Reset the current pid
  pcb_parent->cur_child = 0; // Set the child to be 0 now (Remove later)

  page_table * PDT = (page_table*)PDT_addr; // Restore parent paging
  PDT->entry[PDT_VID_IDX] = (EIGHT_MB + (FOUR_MB* cur_pid)) |PDT_VID_ATTRIB; // Sets the corect bits for the pdt entry

   // Flush TLBs
   // Inputs-none, Outputs-none, Clobbers-eax and cc, Effect-Reset the CR3 reg
  asm volatile(
            "movl %%CR3, %%eax;"
            "movl %%eax, %%CR3;"
            :
            :
            : "eax", "cc"
            );

  tss.ss0 = KERNEL_DS; // Save the KERNEL_DS value to the tss.ss0
  tss.esp0 = pcb_parent->par_esp0; // Store the parents esp0 val

  // memset(pcb_ptr->args, '\0', 128);
  // memset(argbuf, '\0', 128);

  // These inline assembly functions take care of context switching and jumps to the execute_return function
  asm volatile (  "movl %0, %%ebp;"
                  :
                  :"g"(pcb_ptr->par_ebp)
                );

  asm volatile (  "movl %0, %%esp;"
                  :
                  :"g"(pcb_ptr->par_esp)
                );
  asm volatile (  "jmp execute_return;"
                 );


  return 0;
}

/* int32_t execute(const uint8_t* command);
 * Inputs: command - file name and args to parse
 * Return Value: -1 if fail, 256 if dies by exception, 0-255 if stops in halt
 *  Function: Parses thru the command and attempts to execute the corresponding file or command
 */
int32_t execute (const uint8_t* command){
  uint32_t i, new_process, dentry_check, read_data_check = 0;
  uint8_t fname[FNAME_SIZE]; // 32 is the max size of the filename
  uint8_t buf_magicnum[EXE_SIZE];          // first 4 bytes of exe file
  uint8_t arr_magicnum[EXE_SIZE] = {0x7f, 0x45, 0x4c, 0x46}; // First 4 bytes of an exe file, used to identify itself
  uint8_t is_executable = 0;


  uint8_t leading_space, command_idx, parse_idx;

  leading_space = 0;

  cli();

  memset(argbuf, '\0', 128);

  if(command == NULL) { // Check if command is valid
    return -1;
  }
  while (command[leading_space] == ' '){ // Leading spaces check
    leading_space++;
  }

  command_idx = leading_space; // Then set the actual start of command

  while (command[command_idx] != '\n' && command[command_idx] != ' ' && command[command_idx] != '\0'){ // Go until end
    command_idx++;
  }

  for (parse_idx = leading_space; parse_idx < command_idx; parse_idx++){ // And copy all of command to fname
    fname[parse_idx - leading_space] = (int8_t)command[parse_idx];
  }
  fname[command_idx] = '\0';

  // ADD BUF PARSING FOR CP4
  if (command[command_idx] == ' ') {
    command_idx++;
    leading_space = command_idx;

    while (command[command_idx] != '\n' && command[command_idx] != '\0'){ // Go until end
      command_idx++;
    }

    for (parse_idx = leading_space; parse_idx < command_idx; parse_idx++){ // And copy all of command to fname
      argbuf[parse_idx - leading_space] = (int8_t)command[parse_idx];
      // printf("argbuf in execute: %s", argbuf[parse_idx-leading_space]);
    }
  }

  struct dentry_t new_dentry;

  dentry_check = read_dentry_by_name((uint8_t *) fname, &new_dentry); // Calls fs read_dentry_by_name to see if its a file
  if(dentry_check == -1){
    sti();
    return -1;
  }

  read_data_check = read_data(new_dentry.inode, 0,(uint8_t *) buf_magicnum, EXE_SIZE); // Calls fs read_data to see if its an executable
  if(read_data_check == -1){
    sti();
    return -1;
  }

  is_executable = strncmp((const int8_t *) buf_magicnum, (const int8_t *) arr_magicnum, 4); // Compares with the magic number array we made

  if (is_executable != 0) { // Return if string is different
    sti();
    return -1;
  }

  i = 0;
  while(i < 6){ // Checks all pids
    if(pid_flag[i] == 0){ // True if an available PID
      break;
    }
    i++;
  }

  if(i >= 6){ // Check if the available PID is valid
      printf("All processes in use\n");
      sti(); // Allow something else to happen to
      return 0;
  }

  new_process = i;
  pid_flag[i] = 1; // Now this pid is in use

  //Store the arg data to the cur pcb's argbuf using strcpy
  // DO WE NEED THIS???

  page_table* PDT = (page_table*)PDT_addr; // Sets up the pdt entry val to store
  PDT->entry[PDT_VID_IDX] = (EIGHT_MB + (i * FOUR_MB)) | PDT_VID_ATTRIB;    // set correct bit values by ORing with 0x97

  // Flush TLBs
  // Inputs-none, Outputs-none, Clobbers-eax and cc, Effect-Reset the CR3 reg
  asm volatile(
            "movl %%CR3, %%eax;"
            "movl %%eax, %%CR3;"
            :
            :
            : "eax", "cc"
                );



  uint8_t entry_val[EXE_SIZE];
  read_data(new_dentry.inode, 24, entry_val, EXE_SIZE); // Calls fs read_data and copies to entry val (24 because thats the index where we start reading)

  uint32_t l;
	uint32_t c;

	i = 0;
	while((c = read_data(new_dentry.inode, i*FOUR_BYTE, buf_magicnum, FOUR_BYTE)) > 0){ // Copies the entire user level program
		for(l = 0; l < c; l++)  {
			*((uint8_t*)(LOAD + (i*FOUR_BYTE)) + l) = buf_magicnum[l]; // Sets the correct address starts at load addr
		}
		i++;
	}


  pcb_t * new_pcb = (pcb_t *) (EIGHT_MB - EIGHT_KB * (new_process + 1)); // Set new pcb at new_process index we found
  int j;
  int k;

  for (j = 0; j < NUM_PROC; j++) { // Then sets all the new pcbs attributes
      for (k = 0; k < FOPS_NUM; k++) {
          new_pcb->fdesc[j].jumptable[k] = NULL;
      }
      new_pcb->fdesc[j].inode = -1;
      new_pcb->fdesc[j].offset = -1;
      new_pcb->fdesc[j].flags = 0;
  }

  // Finish setting up the rest of the pcb attributes
  new_pcb->parent_pid = cur_pid;
  new_pcb->curr_pid = new_process;
  new_pcb->retval = 0;
  new_pcb->par_ebp = 0;
  new_pcb->par_esp = 0;


  // The following two loops calls the stdin and stdout functions from the jumptable
  for (j = 0; j < FOPS_NUM; j++) {
    new_pcb->fdesc[0].jumptable[j] = stdin_fops[j];   // setting stdin (fd = 0)
  }
  new_pcb->fdesc[0].flags = 1;

  for (j = 0; j < FOPS_NUM; j++) {
    new_pcb->fdesc[1].jumptable[j] = stdout_fops[j];  //  setting stdout (fd = 1)
  }
   new_pcb->fdesc[1].flags = 1;

  cur_pid = new_process; // Update the cur pid global var

  uint32_t par_esp_val;
  uint32_t par_ebp_val;

  // Saves the parents ebp and esp
  asm volatile (    "movl %%ebp, %0;"
                    "movl %%esp, %1;"
                    : "=g"(par_ebp_val),
                      "=g"(par_esp_val)
                );
  new_pcb->par_ebp = par_ebp_val;
  new_pcb->par_esp = par_esp_val;

  new_pcb->par_esp0 = tss.esp0;
  tss.ss0 = KERNEL_DS;
  tss.esp0 = (EIGHT_MB - (new_process) * EIGHT_KB) - 4; // check this line

  strcpy((int8_t*)new_pcb->args, (int8_t*)argbuf); // Copies to that pcbs args buf

  // Setting up IRET values for below inline assembly
  uint32_t target = *((uint32_t*)entry_val);
  uint32_t CS = USER_CS;
  uint32_t SP = SP_VAL;
  uint32_t SS = USER_DS;

  sti();


  // Context switching, passes in values defined above
  asm volatile
                    (
                    "movw %0, %%ax;"
                    "movw %%ax, %%ds;"
                    "pushl %0;"
                    "pushl %1;"
                    "pushfl;"
                    "pushl %2;"
                    "pushl %3;"
                    "iret;"
                    "execute_return:;"
                    "movl $0, %%eax;"
                    "leave;"
                    "ret;"

                    :
                    : "g" (SS),
                      "g" (SP),
                      "g" (CS),
                      "g" (target)
                    : "eax"

                );

  pcb_t* PCB = (pcb_t *)(EIGHT_MB - (cur_pid + 1) * EIGHT_KB); // Sets pcb for the return value
	int32_t return_val = PCB->retval;

	return return_val;

}

/* int32_t read();
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: 0 on succes, -1 on fail
 *  Function: Calls the corresponding read function based on the file type */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
  uint32_t try_read;
  pcb_t * cur_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(cur_pid + 1));  // Gets the pcb pointer

  if(fd < 0 || fd > MAX_FD || cur_pcb->fdesc[fd].flags == 0 || buf == NULL || fd == 1){ // Bounds check
    return -1;
  }

  try_read = (uint32_t) (*(cur_pcb->fdesc[fd].jumptable)[1])(fd, buf, nbytes); // Calls the proper read funcion
  if (try_read == 0) {
    cur_pcb->fdesc[fd].offset = 0;
  }
  return try_read; // Returns the status from the call
}


/* int32_t write();
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Return Value: void
 *  Function: Calls the corresponding write function based on the file type
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
  uint32_t try_write;
  pcb_t * cur_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(cur_pid + 1)); // Gets the pcb pointer

  if (cur_pcb->fdesc[fd].flags == 0) {
    return -1;
  }

  if(fd <= 0 || fd > MAX_FD || buf == NULL){ // Bounds check
    return -1;
  }

  try_write = (uint32_t)(*((cur_pcb->fdesc[fd].jumptable)[2]))(fd, buf, nbytes); // Calls the proper write function
  return try_write; // Returns the status from the call
}

/* int32_t open();
 * Inputs: const uint8_t* filename
 * Return Value: -1 on fail or the fdesc val
 *  Function: Opens the corresponding open funcion based on file type
 *  reserved for Keyboard, which is IRQ line 1 */
int32_t open (const uint8_t* filename){
  struct dentry_t* fill_dentry;
  int x, i;
  int check_dentry = read_dentry_by_name(filename, fill_dentry);
  read_dentry_by_name(filename, fill_dentry); // Gets a dentry that we will use for opening

  // int check_dentry = read_dentry_by_index(0, fill_dentry);
  // read_dentry_by_index(0, fill_dentry);

  pcb_t * cur_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(cur_pid + 1)); // Sets a new pcb to the cur_pid offset address

  if(check_dentry == -1){ // If file doesnt exist
    return -1;
  }

  // If it gets here, it is either rtc, directory, or reg file
  x = 2;
  while(x < 8){
    if(cur_pcb->fdesc[x].flags == 0){ // If this PCB is not used already (i added the .flags part)
      switch(fill_dentry->type){ // Find the correct file type using a swtch-case

        case TYPE_RTC: // If RTC(0)
          for (i = 0; i < 4; i++) {
            cur_pcb->fdesc[x].jumptable[i] =  rtc_fops[i]; // Call the correct functions from jumptable
          }
          break;

        case TYPE_DIR: // If DIR(1)
          for (i = 0; i < 4; i++) {
            cur_pcb->fdesc[x].jumptable[i] =  dir_fops[i]; // Call the correct functions from jumptable
          }
          break;

        case TYPE_FILE: // If FILE(2)
          for (i = 0; i < 4; i++) {
            cur_pcb->fdesc[x].jumptable[i] =  file_fops[i]; // Call the correct functions from jumptable
          }
          break;

        default: // Return fail else
          return -1;
      }

      // Set the corresponding pcb vals
      cur_pcb->fdesc[x].inode = fill_dentry->inode;
      cur_pcb->fdesc[x].offset=0;
      cur_pcb->fdesc[x].flags=1;

      // check this
      // if ((*cur_pcb->fdesc[x].jumptable[0])(filename) < 0) { // If this returns a bad val (from other funcs)
      //   return -1;
      // }

      strcpy((int8_t *)cur_pcb->fname[x], (const int8_t *)filename); // Copy the filename then to the pcbs file name



      return x;
    }
    x++;
  }
  return -1;
}

/* int32_t close ()
 * Inputs: int32_t fd
 * Return Value: 0 on success, -1 on fail
 *  Function: Calls the corresponding close function and resets pcb vals */
int32_t close (int32_t fd){
  pcb_t * cur_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(cur_pid + 1)); // Gets a new pcb pointer
  uint32_t try_close;
  int i;

  if(fd < MIN_FD || fd > MAX_FD || cur_pcb->fdesc[fd].flags == 0){ // Check if valid params and pcb isnt in use
    return -1; // Fail
  }

  cur_pcb->fdesc[fd].flags=0; // Set this pcb flag to 0 for not used anymore

  try_close = (uint32_t)(*(cur_pcb->fdesc[fd].jumptable)[3])(fd); // Call the correct close, and set a var to determine if worked

  if(try_close != 0){ // If closed improperly
    return -1; // Return failed
  }

  for ( i = 0; i < 4; i++) { // If worked, set the jumptable val to null
    cur_pcb->fdesc[fd].jumptable[i] = NULL;
  }

  // And set other attributes to default
  cur_pcb->fdesc[fd].inode = -1;
  cur_pcb->fdesc[fd].offset = -1;
  cur_pcb->fdesc[fd].flags = 0;

  return 0; // Success
}

/* int32_t getargs();
 * Inputs: uint8_t* buf, int32_t nbytes
 * Return Value: 0 or -1
 *  Function: Wrote, not too sure if right yet, not needed until CP4
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
  int8_t find_strlen, i, total_len, num_space;

  // memset(buf, '\0', nbytes);

  if(buf == NULL || nbytes == 0){ // Should an additional OR see the length+end terminal
    return -1;
  }

  pcb_t * cur_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(cur_pid + 1)); // Gets the pcb pointer
  //memset(cur_pcb->args, '\0', nbytes);


  // strncpy((int8_t *) cur_pcb->args, (int8_t *) argbuf, (uint32_t) nbytes);
  // find_strlen = strlen((int8_t *) cur_pcb->args);
  //
  // if(find_strlen == 0){
  //   return -1;
  // }

  if(strlen((int8_t *) cur_pcb->args) > nbytes){
    return -1;
  }
  //
  total_len = strlen(cur_pcb->args);
  num_space = 0;
  while(num_space < total_len){
    if(cur_pcb->args[num_space] != ' '){
      break;
    }
    num_space++;
  }
  memmove(cur_pcb->args, cur_pcb->args+num_space, total_len-num_space+1);

  if(cur_pcb->args[0] == '\0'){
    return -1;
  } else{
    strncpy((int8_t *) buf, (int8_t *) cur_pcb->args, (uint32_t) nbytes);
  }

  // memset(argbuf, '\0', 128);



  // for(i = 0; i < strlen(argbuf); i++){
  //   argbuf[i] = NULL;
  // }

  // printf("argbuf: %s\n", buf);

  //See if neded
  // cur_pcb->args[0] = '\0'; // To signify that the pcb is now empty and can be used again

  return 0;
}

/* int32_t vidmap();
 * Inputs: uint8_t** sreen_start - double pointer that has vid mem address of user programs
 * Return Value: 0 or -1
 *  Function: Haven't written yet, need to in CP4
 */
int32_t vidmap (uint8_t** screen_start){

  if((uint32_t) screen_start < ONETWENTYEIGHT_MB || (uint32_t) screen_start > ONETHIRTYTWO_MB){
		return -1;
	}

	uint8_t * destination_mem = (uint8_t*) 0x06400000;//Pre setting virtual address
	*screen_start = destination_mem;

	return 0;
}

/* int32_t set_handler ()
 * Inputs: int32_t signum, void* handler_address
 * Return Value: 0 or -1
 *  Function: Haven't written yet, need to in CP4
*/
int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}

/* int32_t sigreturn (void)
 * Inputs: None
 * Return Value: 0 or -1
 *  Function: Haven't written yet, need to in CP4
*/
int32_t sigreturn (void){
    return -1;
}
