### Simple OS Kernel (MicroKernel.asm)

##### Supports interrupts and clocks
 - Whenever an interrupt occurs, it copies all the state information (registers, sp, pc, PSW, base and limit_registers) to memory address starting from 256d (Decimal), that is hardwired to the machinery
- When operating system starts  interrupts immediately enabled and CPU will start counting. You can control your clock interrupts by the function setQuantum in 8080emu.cpp. (Default is 80)

##### Supports Multi-Programming
- Kernel can load multiple programs in to memory by selecting appropriate locations. 
- There is a **Process Table** that holds the necessary information about the processes in the memory
- For every RST 5 Interrupt, in other words scheduling signal, OS will handle the interrupt and perform **Round Robin Scheduling**.
-  if a program finishes execution it will acknowledge its termination by
calling PROCESS_EXIT

##### Supports Interprocess Communication, Synchronization
- Processes can call SIGNAL and WAIT to perform synchronization

##### Supports Paging and Virtual Memory
- 8 KBytes of total physical main memory
- Each process has a virtual address space of 16 KBytes
- Programs use virtual addresses, os translates virtual addresses to physical
- Page faults are handled
- When necessary, pages are written to and read from disk (pagetable.txt) (see memory.cpp)