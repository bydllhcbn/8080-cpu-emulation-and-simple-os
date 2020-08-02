#ifndef H_GTUOS
#define H_GTUOS

#include "8080emuCPP.h"


class GTUOS{
public:
    uint64_t handleCall( CPU8080 & cpu,std::ifstream& fin,std::ofstream& fout);
    GTUOS();
    ~GTUOS();
	bool isOnInterrupt(CPU8080 & cpu);
	void printLists(CPU8080 & cpu);

private:
    std::ofstream mailbox;
    std::ofstream locallist;

    void randInt( CPU8080 & cpu);
    void wait( CPU8080 & cpu);
    void signal( CPU8080 & cpu);

    void processExit(const CPU8080 & cpu);
    void printStr(const CPU8080 & cpu,std::ifstream& fin,std::ofstream& fout,uint16_t address);
    void readStr(const CPU8080 & cpu,std::ifstream& fin,std::ofstream& fout,uint16_t address);
    void loadProgram( CPU8080 & cpu,std::ifstream& fin,std::ofstream& fout,uint16_t fileNameAddress);

};

#endif
