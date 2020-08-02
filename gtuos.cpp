#include <iostream>
#include <fstream>
#include <time.h>
#include <bitset>
#include "8080emuCPP.h"
#include "gtuos.h"
#include "memory.h"

#define PRINT_B 1
#define PRINT_MEM 2
#define READ_B 3
#define READ_MEM 4
#define PRINT_STR 5
#define READ_STR 6
#define LOAD_EXEC 7
#define PROCESS_EXIT 8
#define SET_QUANTUM 9
#define RAND_INT 10
#define WAIT 11
#define SIGNAL 12

#define PROCESS_TABLE_ENTRY_SIZE 144
#define INIT_PROCESS_ENTRY_ADDRESS 1024
#define CURRENT_PROCESS_ENTRY_ADDRESS 270
#define PROCESS_COUNT_ADDRESS 272

#define STATE_EMPTY 0
#define STATE_RUNNING 1
#define STATE_READY 2
#define STATE_DEAD 3

/*
; 0-2000H   KERNEL SPACE (INIT AND PROCESS TABLE)
; PROCESS IMAGE SIZES ARE FIXED
; 2000H-3000H PROCESS 1
; 3000H-4000H PROCESS 2
; 4000H-5000H PROCESS 3
; ..000H-..000H PROCESS ..


;Process Table Entry Includes
;One Entry Size = 16+128 = 144 Bytes
;PID | STATE | PC.LO | PC.HI | BASE.LO | BASE.HI | SP.LO | SP.HI | A | CC | B | C | D | E | H | L | PNAME(128)

;This os will hold maximum 14 process at the same time so total process table size
;144*14 = 2304 bytes


 Page Table Entry Size is 1 Byte
 Whole page table of a progress is 16 byte
 Page table start address
 For init 1600
 For process 1 1616
 For process 2 1632
*/

GTUOS::GTUOS(){
    mailbox.open("mailbox.txt",std::ofstream::out | std::ofstream::trunc);
    if (!mailbox.is_open()){
        std::cerr << "Could not open mailbox.txt \n";
        exit(1);
    }
    locallist.open("locallist.txt",std::ofstream::out | std::ofstream::trunc);

    if (!locallist.is_open()){
        std::cerr << "Could not open locallist.txt \n";
        exit(1);
    }
}

GTUOS::~GTUOS(){
    locallist.close();
    mailbox.close();
}


bool GTUOS::isOnInterrupt( CPU8080 & cpu){
    return cpu.state->pc == 0x28;
}

uint64_t GTUOS::handleCall( CPU8080 & cpu,std::ifstream& fin,std::ofstream& fout){
    uint16_t address = (cpu.state->b << 8 ) | (cpu.state->c & 0xff);//Convert BC register pairs into one pair address

    switch (cpu.state->a) {
        case PRINT_B:
            fout << (int)cpu.state->b;
           // std::cout << "PRINTBCALLEd"<<std::endl;
            break;
        case PRINT_MEM:
            fout << (int)cpu.memory->at(address);
            break;
        case READ_B:
            int a;
            fin>>a;
            cpu.state->b = (uint8_t)a;
            break;
        case READ_MEM:
            fin>>cpu.memory->at(address);
            break;
        case PRINT_STR:
            printStr(cpu,fin,fout,address);
            break;
        case READ_STR:
            readStr(cpu,fin,fout,address);
            break;

        case LOAD_EXEC:
            loadProgram(cpu,fin,fout,address);
            break;

        case PROCESS_EXIT:
            processExit(cpu);
            break;
        case SET_QUANTUM:
            cpu.setQuantum(cpu.state->b);
            break;
        case RAND_INT:
            randInt(cpu);
            break;
        case WAIT:
            wait(cpu);
            break;
        case SIGNAL:
            signal(cpu);
            break;

        default:break;
    }

	return 0;
}

void GTUOS::randInt( CPU8080 & cpu){
    int random = rand() % 255 + 1;
    cpu.state->b = random;
}

void GTUOS::printLists(CPU8080 & cpu) {
    int currentProcessAddress = (cpu.memory->physicalAt(CURRENT_PROCESS_ENTRY_ADDRESS+1) << 8 ) |
                                (cpu.memory->physicalAt(CURRENT_PROCESS_ENTRY_ADDRESS) & 0xff);

    ((Memory*)cpu.memory)->system<<"CSEVENT: "<< (int)cpu.memory->physicalAt((uint32_t)currentProcessAddress);
    ((Memory*)cpu.memory)->system<<",";
    int i=0;
    while (cpu.memory->physicalAt((uint16_t)(currentProcessAddress+16+i))!=0){
        ((Memory*)cpu.memory)->system<<(char)cpu.memory->physicalAt((uint16_t)(currentProcessAddress+16+i));
        i++;
    }
    ((Memory*)cpu.memory)->system<<",";


    ((Memory*)cpu.memory)->system<<(int)cpu.memory->physicalAt((uint32_t)currentProcessAddress+PROCESS_TABLE_ENTRY_SIZE);



    ((Memory*)cpu.memory)->system<<"\n";
}

void GTUOS::wait( CPU8080 & cpu){
    int idMB = cpu.state->b;
    int addressSem = cpu.state->c;
    int semAddress = 5000 + (idMB-1)*53 + addressSem;

    int sem = cpu.memory->physicalAt((uint32_t)(semAddress)) ;
    //printf("sem is %d at address %d \n",sem,semAddress);
    if(sem == 0){
        //if semephore is zero go back to previus instruction that will cause this os call to be happen again
        //as long as it is zero the calling process will stuck here
        cpu.state->pc--;
        cpu.state->sp++;
        cpu.state->sp++;
    }else{
        cpu.memory->physicalAt((uint32_t)(semAddress))--;
    }
    cpu.setQuantum(200);//this will cause and interrupt for scheduler
}

void GTUOS::signal( CPU8080 & cpu){
    int idMB = cpu.state->b;
    int addressSem = cpu.state->c;
    int semAddress = 5000 + (idMB-1)*53 + addressSem;
   // printf("signal is come at addr %d \n",addressSem);
    cpu.memory->physicalAt((uint32_t)(semAddress))++ ;

    cpu.setQuantum(200);//this will cause and interrupt for scheduler
}



void GTUOS::processExit(const CPU8080 & cpu){
    int currentProcessAddress = (cpu.memory->physicalAt(CURRENT_PROCESS_ENTRY_ADDRESS+1) << 8 ) |
            (cpu.memory->physicalAt(CURRENT_PROCESS_ENTRY_ADDRESS) & 0xff);

    cpu.memory->physicalAt((uint32_t)PROCESS_COUNT_ADDRESS)--;

    printf("A process killed with pid=%d\n",cpu.memory->physicalAt((uint32_t)currentProcessAddress));

    char pageImageName[21]; // enough to hold all numbers up to 64-bits
    sprintf(pageImageName, "%d_page.img", cpu.memory->physicalAt((uint32_t)currentProcessAddress));
    //remove(pageImageName);

    cpu.memory->physicalAt((uint32_t)currentProcessAddress+1) = STATE_DEAD; //Change process state to dead
}

void GTUOS::printStr(const CPU8080 &cpu, std::ifstream &fin, std::ofstream &fout,uint16_t address) {


    while (cpu.memory->at(address)!=0){
        fout << (char) cpu.memory->at(address);
        address++;
    }

    fout.flush();
}


void GTUOS::loadProgram( CPU8080 & cpu,std::ifstream& fin,std::ofstream& fout,uint16_t fileNameAddress){
    uint32_t startAddress = static_cast<uint32_t>((cpu.state->h << 8 ) | (cpu.state->l & 0xff));

    char* filename = (char *) calloc(128, sizeof(char));
    int i = 0;

    while (cpu.memory->at(fileNameAddress)!=0 && i<128){
        filename[i] = (char) cpu.memory->at(fileNameAddress);
        fileNameAddress++;
        ++i;
    }



    //cpu.ReadFileIntoMemoryAt(filename,startAddress);

    int newProcessAddr = INIT_PROCESS_ENTRY_ADDRESS+PROCESS_TABLE_ENTRY_SIZE;

    uint8_t pid = 1;
    //Loop through process table until an empty space found
    while (cpu.memory->physicalAt((uint32_t)(newProcessAddr+1))!=STATE_EMPTY){
        newProcessAddr+=PROCESS_TABLE_ENTRY_SIZE;
        pid++;
    }

    char pageImageName[21];
    sprintf(pageImageName, "%d_page.img", pid);

    //CREATE AN 16KB PAGE IMAGE FOR THIS PROCESS
    //ON PAGE FAULT THIS FILE WILL BE USED
    std::fstream ifile(filename, std::ios::in | std::ios::binary | std::ios::ate);

    std::fstream ofile(pageImageName, std::ios::out | std::ios::binary);

    //std::cerr << pageImageName <<"   is open "<<ofile.is_open() ;
    //((Memory*)cpu.memory)->pageFiles[pid] = std::fstream.open(pageImageName, std::ios::in | std::ios::out | std::ios::binary);
    ((Memory*)cpu.memory)->pageFiles[pid].open(pageImageName, std::ios::in | std::ios::out | std::ios::binary);

    size_t size   = ifile.tellg();
    char*  buffer = new char[size];

    ifile.seekg(0, std::ios::beg);
    ifile.read(buffer, size);
    ofile.write(buffer, size);

    ofile.seekp((16<<10) - 1);
    ofile.write("", 1);

    ifile.close();
    ofile.close();
    free(buffer);

    int pageTableAddress = 1600 + (16*pid);

    //SETUP PAGE TABLE
    for(int i=0;i<16;i++){
        cpu.memory->physicalAt(pageTableAddress+i) = 0;
    }


    printf("New process started at BASE=%02xH with pid=%d,name=%s\n",startAddress,pid,filename);
    cpu.memory->physicalAt((uint32_t)PROCESS_COUNT_ADDRESS)++;

    //Put pid of the process
    cpu.memory->physicalAt((uint32_t)(newProcessAddr)) = pid;

    //Put state to ready
    cpu.memory->physicalAt((uint32_t)(newProcessAddr+1)) = STATE_READY;

    //put base register
    cpu.memory->physicalAt((uint32_t)(newProcessAddr+4)) = (uint8_t)(startAddress & 0xff);
    cpu.memory->physicalAt((uint32_t)(newProcessAddr+5)) = (uint8_t)(startAddress >> 8);

    //STACK POINTER MUST BE SET AT PROGRAM CODE

    //put the process name to process table
    i=0;
    while (filename[i]!=0){
        cpu.memory->physicalAt((uint16_t)(newProcessAddr+16+i)) = (uint8_t)(filename[i]);
        i++;
    }

    free(filename);
}

void GTUOS::readStr(const CPU8080 &cpu, std::ifstream &fin, std::ofstream &fout,uint16_t address) {
    int byte;
    while ((byte=fin.get()) != -1){
	if(byte==10)break;
        cpu.memory->at(address) = (uint8_t)byte;
        address++;
    }
}
