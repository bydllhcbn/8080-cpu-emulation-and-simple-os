#ifndef H_MEMORY
#define H_MEMORY

#include <cstdlib>
#include "memoryBase.h"
#include <vector>
#include <iostream>
#include <fstream>
// This is just a simple memory with no virtual addresses.
// You will write your own memory with base and limit registers.

class Memory: public MemoryBase {
public:
	Memory(uint64_t size);
	~Memory();
	virtual uint8_t & at(uint32_t ind);
	virtual uint8_t & physicalAt(uint32_t ind);
	uint16_t getBaseRegister() const { return baseRegister;}
	uint16_t getLimitRegister() const { return limitRegister;}
	void setBaseRegister(uint16_t base) { this->baseRegister = base;}
	void setLimitRegister(uint16_t limit) {this->limitRegister = limit;}
    int fifoHead = 3;
	char * readFromPage(int pid,int pageNumber);

    std::fstream pageFiles[16];
    std::ofstream system;
    std::ofstream pagetable;

private:
	uint8_t * mem;
	uint16_t baseRegister;
	uint16_t limitRegister;

    int findMeSpace(int ramPage);
};

#endif


