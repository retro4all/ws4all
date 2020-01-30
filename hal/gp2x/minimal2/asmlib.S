@ gp2x demo article code by Dzz
@ this code is placed in the public domain.  do anything you want with it.

	.align 4
	.globl OpenFile
	.globl CloseFile
	.globl MUnmap
	.globl ChangeDir
	.globl ExecuteFile
	.globl MMap
	.globl CopyScreen
	.globl ClearScreen
	.globl myuname

OpenFile:
	swi #0x900005
	mov pc, lr

CloseFile:
	swi #0x900006
	mov pc, lr

MUnmap:
	swi #0x90005B
	mov pc, lr

ChangeDir:
	swi #0x90000C
	mov pc, lr

ExecuteFile:
	swi #0x90000B
	mov pc, lr

MMap:
	stmdb sp!, {r0, r1, r2, r3}
	mov r0, sp
	swi #0x90005A
	add sp, sp, #16
	mov pc, lr
	
CopyScreen:
  stmfd sp!, {r4-r10}  @ remember registers 4-10
  mov r2, #7        @ we will run the loop 4800 times to copy the screen
.CopyScreenLoop:
  ldmia r1!, {r3-r10}  @ pull in 32 bytes from the source
  stmia r0!, {r3-r10}  @ write the 32 bytes to the destination
  subs r2, r2, #1      @ decrement the loop counter
  bne .CopyScreenLoop  @ if we're not done, do it again
  ldmfd sp!, {r4-r10}  @ restore the registers
	mov pc, lr           @ return

ClearScreen:
  stmfd sp!, {r4-r10}  @ remember registers 4-10
  mov r2, #4800        @ we will run the loop 4800 times to copy the screen
	mov r3, #0           @ load up the registers with zeros
	mov r4, #0
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	mov r9, #0
	mov r10, #0
.ClearScreenLoop:
  stmia r0!, {r3-r10}  @ write the 32 bytes of zeros to the destination
  subs r2, r2, #1      @ decrement the loop counter
  bne .ClearScreenLoop  @ if we're not done, do it again
  ldmfd sp!, {r4-r10}  @ restore the registers
	mov pc, lr           @ return

myuname:
	swi #0x90007A
	mov pc, lr
