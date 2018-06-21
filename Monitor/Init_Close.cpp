#include "Init_Close.h"
extern ofstream Imgfile;
extern long LockNum, SynNum;
void InitFileOutput()
{
	char filename[50];
	sprintf(filename, "Img%d.out", getpid());
	Imgfile.open(filename);
	Imgfile.setf(ios::showbase);
}

void CloseFileOutput()
{
	Imgfile.close();
}

void Output()
{
	Imgfile<<"Lock: "<<LockNum<<endl;
	Imgfile<<"Syn: "<<SynNum<<endl;
}

int Usage()
{
	cerr << "This is the invocation pintool" << endl;
	cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
	return -1;
}

VOID Fini(INT32 code, VOID *v)
{
	Output();
}
