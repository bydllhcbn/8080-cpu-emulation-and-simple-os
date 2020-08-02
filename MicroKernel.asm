        ; 8080 assembler code
        .hexfile MicroKernel.hex
        .binfile MicroKernel.com
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
	
	; Ubeydullah Çoban - 161044008
	; 0-2000H   KERNEL SPACE (INIT AND PROCESS TABLE)
	; PROCESS IMAGE SIZES ARE FIXED
	; 2000H-3000H PROCESS 1
	; 3000H-4000H PROCESS 2
	; 4000H-5000H PROCESS 3
	; ..000H-..000H PROCESS ..


	;Process Table Entry Includes
	;One Entry Size = 16+128 = 144 Bytes
    ;PID | STATE | PC.LO | PC.HI | BASE.LO | BASE.HI | SP.LO | SP.HI | A | CC | B | C | D | E | H | L | PNAME(128)

    ;This kernel will hold maximum 14 process at the same time so total process table size
    ;144*14 = 2304 bytes

    ;270

	; ---------------------------------------------------------------
innerruptSpace:	DS 27
	interrupt:
        DI
    	; interrupt --------------------------------------
        LHLD 270 ;GET CURRENT PROCESS TABLE ENTRY ADDRESS

        INX H ; GET STATE ADDRESS TO HL
        MOV A,M

        CPI stateDead ; IF THE CURRENT PROCESS IS DEAD
        JZ skipToNextProcess ; NO NEED TO SAVE SKIP TO NEXT PROCESS

        MVI M,stateReady ; SET STATUS TO READY

        ;----------START-> SAVE THE CURRENT STATE TO PROCESS TABLE
        INX H ; GET PC.LO ADDRESS TO HL
        LDA 265 ;LOAD PC.LO FROM HARDWARE
        MOV M,A
        INX H ; GET PC.HI ADDRESS TO HL
        LDA 266 ;LOAD PC.HI FROM HARDWARE
        MOV M,A
        INX H ; GET BASE.LO ADDRESS TO HL
        LDA 267 ;LOAD BASE.LO FROM HARDWARE
        MOV M,A
        INX H ; GET BASE.HI ADDRESS TO HL
        LDA 268 ;LOAD BASE.HI FROM HARDWARE
        MOV M,A
        INX H ; GET SP.LO ADDRESS TO HL
        LDA 263 ;LOAD SP.LO FROM HARDWARE
        MOV M,A
        INX H ; GET SP.HI ADDRESS TO HL
        LDA 264 ;LOAD SP.HI FROM HARDWARE
        MOV M,A
        INX H ; GET A ADDRESS TO HL
        LDA 256 ;LOAD A FROM HARDWARE
        MOV M,A
        INX H ; GET CC ADDRESS TO HL
        LDA 269 ;LOAD CC FROM HARDWARE
        MOV M,A
        INX H ; GET B ADDRESS TO HL
        LDA 257 ;LOAD B FROM HARDWARE
        MOV M,A
        INX H ; GET C ADDRESS TO HL
        LDA 258 ;LOAD C FROM HARDWARE
        MOV M,A
        INX H ; GET D ADDRESS TO HL
        LDA 259 ;LOAD D FROM HARDWARE
        MOV M,A
        INX H ; GET E ADDRESS TO HL
        LDA 260 ;LOAD E FROM HARDWARE
        MOV M,A
        INX H ; GET H ADDRESS TO HL
        LDA 261 ;LOAD H FROM HARDWARE
        MOV M,A
        INX H ; GET L ADDRESS TO HL
        LDA 262 ;LOAD L FROM HARDWARE
        MOV M,A
        ;----------END-> SAVE THE CURRENT STATE TO PROCESS TABLE


        LHLD 270 ;GET CURRENT PROCESS ADDRESS
        INX H ; GET STATUS
skipToNextProcess:
        LXI B,144 ; GOTO NEXT PROCESS
        DAD B ; GOTO NEXT PROCESS
        MOV A,M ; GOTO NEXT PROCESS
        CPI stateDead ; IF NEXT PROCESS IS DEAD
        JZ skipToNextProcess ; SKIP TO NEXT PROCESS

        CPI stateReady ; IF NEXT PROCESS IS READY
        JZ gotoNextProcess ; START LOADING IT

        ; ELSE GOTO INIT PROCESS (ROUND ROBIN)
gotoInit:
        LXI H,1024
        SHLD 270
        JMP gotoProcess

gotoNextProcess:
        DCX H
        SHLD 270


        ; NOW LOADING NEXT PROCESS STATE
gotoProcess:
        LHLD 270
        INX H
        MVI M,stateRunning ; PUT PROCESS STATE TO RUNNING

        ;-------START-> GET A AND FLAG REGISTERS
        LHLD 270
        LXI B,8
        DAD B
        MOV A,M
        INX H
        MOV B,M ; B = FLAGS
        LXI H,0
        DCX SP
        DAD SP  ; HV=SP-1
        MOV M,A ; MOVE A TO SP-1
        DCX SP  ;SP = SP-2
        DCX H   ;SP = SP-2 = HP
        MOV M,B ;MOVE B TO SP-2
        POP PSW ;RECOVER ACCUMULATOR AND FLAGS WITH THE HELP OF POP
        ;-------END-> GET A AND FLAG REGISTERS


        ;-------START-> LOAD STACK POINTER OF NEXT PROCCESS
        LHLD 270
        PUSH B
        LXI B,6
        DAD B
        MOV B,M
        INX H
        MOV C,M
        MOV H,C
        MOV L,B
        POP B
        SPHL
        ;-------END-> LOAD STACK POINTER OF NEXT PROCCESS

        ;-------START-> GET B C H L REGISTERS
        LHLD 270
        LXI B,10
        DAD B

        MOV B,M
        INX H
        MOV C,M
        INX H
        MOV D,M
        INX H
        MOV E,M
        PUSH B
        INX H
        MOV B,M
        INX H
        MOV C,M
        MOV H,B
        MOV L,C
        POP B
        ;-------END-> GET B C H L REGISTERS


        ;START-> SAVE DC HL REGISTERS TO STACK (PCHL WILL GET THEM ACCORDING TO HW)
        PUSH D
        PUSH H
        ;END-> SAVE DC HL REGISTERS TO STACK (PCHL WILL GET THEM ACCORDING TO HW)


        ;-------START-> GET PROGRAM COUNTER AND BASE ADDRESS
        LHLD 270 ;GET CURRENT PROCESS ADDRESS
        INX H
        INX H
        INX H
        INX H
        MOV E,M
        INX H
        MOV D,M
        DCX H
        DCX H
        DCX H

        push B
        MOV B,M ; low
        INX H
        MOV C,M ; high

        MOV L,B
        MOV H,C
        pop B
        ;-------END-> GET PROGRAM COUNTER AND BASE ADDRESS



        EI ; ENABLE INTERRUPTS
        PCHL ; FINISH INTERRUPT AND RETURN


kernelSpace:	DS 2000 ; Will hold process table and other stuff
processTableEntrySize	equ 144
stateRunning	equ 1
stateReady	equ 2
stateDead	equ 3
currentProcessEntryAddress	equ 270


processName1:	dw 'sender.com',00H ; null terminated string
processName2:	dw 'receiver.com',00H ; null terminated string


begin:

    DI ; WE SHOULD NOT GET INTERRUPTED DURING LOADING THE KERNEL AND INIT
    ;------START-> STARTING THE INIT PROCESS
    MVI A,0
    STA 272

    LXI H,1024
    SHLD currentProcessEntryAddress
    MVI A,0 ; SET INIT PROCESS ID 0

    STA 1024
    MVI A,stateRunning ; IT IS RUNNING NOW
    STA 1025

    ; SET INIT PROCESS NAME
    MVI A,'i'
    STA 1040
    MVI A,'n'
    STA 1041
    MVI A,'i'
    STA 1042
    MVI A,'t'
    STA 1043
    MVI A,0
    STA 1044
    ; SET INIT PROCESS NAME

    LXI SP,1FFFH ; INITIALIZE STACK POINTER

    LXI H,1FFFH ; SAVE STACK POINTER TO PROCESS TABLE
    MOV A,H
    STA 1031
    MOV A,L
    STA 1030
    ;------END-> STARTING THE INIT PROCESS

    MVI A,1
    STA 5000
    MVI A,0
    STA 5001
    MVI A,50
    STA 5002
    MVI A,0
    STA 6000

    EI ; ENABLE INTERRUPTS


    ;START THE PROCESSES
    ;-------
    LXI B, processName1
    LXI H,3000
    MVI A, LOAD_EXEC
    CALL GTU_OS

    LXI B, processName2
    LXI H,4000
    MVI A, LOAD_EXEC
    CALL GTU_OS




    LXI H,272 ; GET PROCESS COUNT ADDRESS

initLoop:
	MOV A,M ; GET ACTIVE PROCESS COUNT
	CPI 0 ; AS LONG AS IT IS NOT ZERO LOOP FOREVER
	JNZ initLoop

	HLT ; IF IT IS ZERO HALT THE MACHINE