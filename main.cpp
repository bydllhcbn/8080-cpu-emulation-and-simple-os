#include <iostream>
#include <fstream>
#include <time.h>
#include <bitset>
#include "8080emuCPP.h"
#include "gtuos.h"
#include "memory.h"

int main (int argc, char**argv){


    if (argc != 3){
        std::cerr << "Usage: prog exeFile debugOption\n";
        exit(1);
    }

    std::ofstream fout ("output.txt");
    if (!fout.is_open()){
        std::cerr << "Could not open output.txt \n";
        exit(1);
    }

    std::ifstream fin ("input.txt");//Open input.txt if not opened create and try again
    if (!fin.is_open()){
	std::ofstream outfile ("input.txt");
	outfile.close();
	std::ifstream fin ("input.txt");
	if (!fin.is_open()){
	 std::cerr << "Could not open input.txt \n";
         exit(1);
	}
       
    }


    int DEBUG = atoi(argv[2]);

	Memory mem(0x10000);
	CPU8080 theCPU(&mem);
	GTUOS	theOS;

	theCPU.ReadFileIntoMemoryAt(argv[1], 0x0000);
	do{
		theCPU.Emulate8080p(DEBUG);

        if(theOS.isOnInterrupt(theCPU)){
            theOS.printLists(theCPU);
        }

		if(theCPU.isSystemCall() && theCPU.interrupt==0){
			theOS.handleCall(theCPU,fin,fout);
        }
    }	while (!theCPU.isHalted());

    fout.close();
    fin.close();

	return 0;
}

