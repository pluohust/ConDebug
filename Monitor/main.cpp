#include "main_basictype.h"
#include "VectorClocks.h"
#include "Init_Close.h"
#include "monitor_function.h"
using namespace std;
ofstream Imgfile; //output the img access information
long LockNum=0;
long SynNum=0;
PIN_LOCK lock;
int main(int argc, char * argv[])
{
	if (PIN_Init(argc, argv))
		return Usage();
	PIN_InitLock(&lock);
	PIN_InitSymbols();
	InitFileOutput();
	IMG_AddInstrumentFunction(ImageLoad, 0);
	PIN_AddFiniFunction(Fini, 0);
	PIN_AddThreadFiniFunction(ThreadFini, 0);
	PIN_StartProgram();
	CloseFileOutput();
	return 0;
}
