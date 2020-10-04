
.global sys_wrap

# This function serves as a wrapper for our sys calls
# Purpose: to call the corresponding system call (halt, exec, etc)
sys_wrap:
  pushl %ebp # Save the base pointer
  pushl %esi # Save the index pointer
  pushl %edi # Save the index pointer

  pushfl # Push all flags

  decl %eax     # subtract 1 to account for indexing
  cmpl $0, %eax  
  jl sys_error #  lower, call the error
  cmpl $9, %eax
  jg sys_error    # higher, call the error 
  
  pushl %edx # Push the arguments in order right-left
  pushl %ecx
  pushl %ebx

  call *sys_jumptable(, %eax, 4) #  here, call the correct syscall function

  popl %ebx # Pop all from stack
  popl %ecx
  popl %edx

# This function will be called to finish the wrapper file
# Purpose: Pop all stack vars and return to C function
sys_finish:
  popfl

  popl %edi
  popl %esi
  popl %ebp

  iret

# This function will be called if we are attempting to execute a syscall function that is out of bounds
# Purpose: Returns -1
sys_error:
  movl $-1, %eax
  jmp sys_finish



# This is our jumptable for all sys functions
sys_jumptable:
  .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

