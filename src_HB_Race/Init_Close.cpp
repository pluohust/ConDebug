#include "Init_Close.h"

extern map<UINT32, ThreadVecTime> AllThread;
extern size_t MaxSum;
extern bool monitorendflag;

extern ofstream RaceOut;

extern ofstream AllMetrics; //output all metrics
extern int ThreadNum; //thread count
extern long LockNum; //lock count
extern long SynNum; //syn count
extern set<ADDRINT> ShareVarAddress;

void InitFileOutput()
{
    char filename[50];

    sprintf(filename,"RaceOut.out");
    RaceOut.open(filename);
    RaceOut.setf(ios::showbase);
	
	sprintf(filename,"%dAllMetrics.out", getpid());
    AllMetrics.open(filename);
    AllMetrics.setf(ios::showbase);
}

void CloseFileOutput()
{
    RaceOut.close();
	AllMetrics.close();
}

void OutPutMetrics()
{
	AllMetrics<<"ThreadNum: "<<ThreadNum<<endl;
	AllMetrics<<"LockNum: "<<LockNum<<endl;
	AllMetrics<<"SynNum: "<<SynNum<<endl;
	AllMetrics<<"Loc: "<<ShareVarAddress.size()<<endl;
}

int Usage()
{
    cerr << "This is the invocation pintool" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

VOID Fini(INT32 code, VOID *v)
{
	OutPutMetrics();
    monitorendflag=true;
}
