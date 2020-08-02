        ; 8080 assembler code
        .hexfile sender.hex
        .binfile sender.com
        ; try "hex" for downloading in hex format
        .download bin  
        .objcopy gobjcopy
        .postbuild echo "OK!"
        ;.nodump

	; OS call list
PRINT_B		equ 1
PRINT_MEM	equ 2
READ_B		equ 3
READ_MEM	equ 4
PRINT_STR	equ 5
READ_STR	equ 6
LOAD_EXEC	equ 7
PROCESS_EXIT	equ 8
SET_QUANTUM	equ 9
RAND_INT	equ 10
WAIT	equ 11
SIGNAL	equ 12
	org 000H
	jmp begin

	; Start of our Operating System
GTU_OS:	PUSH D
	push D
	push H
	push psw
	nop	; This is where we run our OS in C++, see the CPU8080::isSystemCall()
		; function for the detail.
	pop psw
	pop h
	pop d
	pop D
	ret
	; ---------------------------------------------------------------
	; YOU SHOULD NOT CHANGE ANYTHING ABOVE THIS LINE

	;This program adds numbers from 0 to 10. The result is stored at variable
	; sum. The results is also printed on the screen.

HANDLE	equ 1
MBMUTEX	equ 0
MBSEMFULL	equ 1
MBSEMEMPTY	equ 2
BUFFERCOUNTADDRESS	equ 3000

sum	ds 2 ; will keep the sum

begin:
	LXI SP,999 	; always initialize the stack pointer


    MVI D,20
    loop1:
    MOV A,D
    CPI 0
    JZ loop1Exit
        DCR D
        MVI E,20
        loop2:
        MOV A,E
        CPI 0
        JZ loop1
        ;START

        ;MVI A, WAIT
        ;MVI B, HANDLE
        ;MVI C, MBSEMEMPTY
        ;call GTU_OS

        ;MVI A, WAIT
        ;MVI B, HANDLE
        ;MVI C, MBMUTEX
        ;call GTU_OS
        ;CRITICAL SECTION


        LHLD BUFFERCOUNTADDRESS
        LXI B,2003 ;BUFFER FIRST ITEM ADDRESS
        DAD B

        MVI A, RAND_INT
        call GTU_OS
        MOV M,B

        LHLD BUFFERCOUNTADDRESS
        INX H
        SHLD BUFFERCOUNTADDRESS
        ;CRITICAL SECTION
        ;MVI A, SIGNAL
        ;MVI B, HANDLE
        ;MVI C, MBMUTEX
        ;call GTU_OS

        ;MVI A, SIGNAL
        ;MVI B, HANDLE
        ;MVI C, MBSEMFULL
        ;call GTU_OS



        ;END
        DCR E
        JMP loop2
loop1Exit:

	MVI A, PROCESS_EXIT
	call GTU_OS

	;MVI A, WAIT
	;MVI B, HANDLE
	;MVI C, 1
    ;call GTU_OS


