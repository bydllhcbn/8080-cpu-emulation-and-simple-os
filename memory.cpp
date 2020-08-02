#include <iostream>
#include <bitset>
#include "memory.h"
#define CURRENT_PROCESS_ENTRY_ADDRESS 270
#define PROCESS_TABLE_ENTRY_SIZE 144
#define INIT_PROCESS_ENTRY_ADDRESS 1024
#define PROCESS_COUNT_ADDRESS 272

Memory::Memory(uint64_t size) {
	mem = (uint8_t*) calloc(size, sizeof(uint8_t));
	baseRegister = 0;
	limitRegister = 0;

    pagetable.open("pagetable.txt",std::ofstream::out | std::ofstream::trunc);
    if (!pagetable.is_open()){
        std::cerr << "Could not open mailbox.txt \n";
        exit(1);
    }

    system.open("system.txt",std::ofstream::out | std::ofstream::trunc);

    if (!system.is_open()){
        std::cerr << "Could not open locallist.txt \n";
        exit(1);
    }

}


Memory::~Memory() {
    free(mem);
    system.close();
    pagetable.close();
}


int Memory::findMeSpace(int ramPage){
    int pageTableAddress = 1600 + (16);
    int8_t pcount = physicalAt((uint32_t) PROCESS_COUNT_ADDRESS);
    std::bitset<8> current;
    std::bitset<8> ramPageNumber (ramPage);


    for(int i=0;i<pcount;i++){
        for(int offset=0;offset<16;offset++){
            current = physicalAt(pageTableAddress);

            if(current.test(2) && (current>>3)==ramPageNumber){
                if(current.test(0)){
                    int pid = i+1;
                    pageFiles[pid].seekg(1024*offset, std::ios::beg);
                    pageFiles[pid].write((char*)(&mem[1024*ramPage]),1024);
                    current.reset(0);
                    current.reset(1);
                    current.reset(2);
                    physicalAt(pageTableAddress) = (uint8_t)current.to_ulong();

                }
                break;
            }
            pageTableAddress++;
        }

    }
    return 0;
}

uint8_t & Memory::at(uint32_t ind) {
    bool pageFault = false;
    if(baseRegister==0) return mem[ind]; //Kernel should directly access the memory
    //return mem[ind+baseRegister];
    int currentProcessAddress = (physicalAt(CURRENT_PROCESS_ENTRY_ADDRESS+1) << 8 ) |
                                (physicalAt(CURRENT_PROCESS_ENTRY_ADDRESS) & 0xff);
    int pid = physicalAt((uint32_t) currentProcessAddress);

    int pageTableAddress = 1600 + (16*pid);

    std::bitset<14> virtualAddress(ind);
    std::bitset<14>  virtualPageNumber = virtualAddress >> 10;
    std::bitset<10>  pageOffset(ind);
    std::bitset<8>  pageEntry   =  mem[pageTableAddress+virtualPageNumber.to_ulong()];
    //std::cout<< ind <<  " Virtual page Number -> " << virtualPageNumber.to_ulong()<<" vAddress -> "<<virtualAddress << " pageOffset -> "<<pageOffset<< '\n';
   // std::cerr<< "page Entry => "<<pageEntry<<"\n";
    pageEntry.set(0);
    pageEntry.set(1);
    if(!pageEntry.test(2)){//PageFault
        pageFault = true;
        ++fifoHead;
        if(fifoHead>7)fifoHead=3;
        findMeSpace(fifoHead);
        fprintf(stdout,"pageFoult %d\n",fifoHead);
        //uint8_t pageMemoryBuffer[1024];
        pageFiles[pid].seekg(1024*virtualPageNumber.to_ulong(), std::ios::beg);
        pageFiles[pid].read((char *)(&(mem[1024*fifoHead])), 1024);
        //std::cerr<<"readed into"<<1024*fifoHead<<"\n 1"<<mem[1024*fifoHead]<<" "<<mem[1024*fifoHead+1];

        pageEntry.set(2);

        std::bitset<3>  mnn(fifoHead);
        pageEntry.set(3,mnn.test(0));
        pageEntry.set(4,mnn.test(1));
        pageEntry.set(5,mnn.test(2));

        //printf("Now read %c %c %c %c %c\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
        //printf("Now read %c %c %c %c %c\n",mem[0 + baseRegister],mem[1 + baseRegister],mem[2 + baseRegister],mem[3 + baseRegister],mem[4 + baseRegister]);
    }

    mem[pageTableAddress+virtualPageNumber.to_ulong()] = (uint8_t)pageEntry.to_ulong();
    std::bitset<13>  physicalAddress(pageOffset.to_ulong());

    physicalAddress[10] = pageEntry[3];
    physicalAddress[11] = pageEntry[4];
    physicalAddress[12] = pageEntry[5];

   // std::cout<<  "Physical page Number-> " << newPageNumber.to_ulong()<<" pAddress -> "<<physicalAddress << '\n';

    //std::cout << "Process " << pid << "reaching " << virtualAddress.to_ulong() << "result " << physicalAddress.to_ulong()<<"\n";
    //exit(0);

    if(pageFault){
        system<<"PAGEFAULT: "<<pid<<", "<<virtualAddress.to_ulong()<<", "<<physicalAddress.to_ulong()<<", "<<pageEntry<<" \n";
        for(int i=0;i<16;i++){
            pagetable<<(char)mem[pageTableAddress+i];
        }
        pagetable<<"\n";
    }


    return mem[(uint16_t)physicalAddress.to_ulong()];
    //return mem[ind + baseRegister];
}



uint8_t & Memory::physicalAt(uint32_t ind) {
	return mem[ind];
}

char *Memory::readFromPage(int pid, int pageNumber) {

    return nullptr;
}
