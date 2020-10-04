.globl paging1_helper

/* int32_t paging1_helper;
 * Inputs: PDT_Address in %eax
 * Return Value: - 0
 * Function: sets up the pages 
 *  */

paging1_helper:
	# reference - osdev

    pushl %ebp
    movl %esp, %ebp
	# load page table

	movl 8(%ebp), %eax		# Move the arg passed into EAX
	movl %eax, %cr3			# Put value of arg into CR3

	# enable 4MB pages
	movl %cr4, %eax				
	orl $0x10, %eax			# enable correct bit
	movl %eax, %cr4

	# enable protected mode and paging
	movl %cr0, %eax
	orl $0x80000001, %eax		# enable correct bits
	movl %eax, %cr0

	# return 0
	xorl %eax, %eax
	leave
	ret

