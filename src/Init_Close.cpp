#include "Init_Close.h"

extern map<UINT32, ThreadVecTime> AllThread;

extern set<ADDRINT> ShareVarAddress;

extern bool monitorendflag;

extern ofstream RaceOut;
extern ofstream ConflictOut;
extern ofstream AllEventOut;
extern ofstream FrameOut;

extern ofstream AllMetrics;

extern int ThreadNum;
extern long Race_Warnings;
extern long Conflict_Warnings;
extern long ConNumFrames;
extern size_t MaxSum;
extern long NumAnalysis;
extern long LockNum;
extern long SynNum;

void InitFileOutput()
{
    char filename[50];

    sprintf(filename,"%dRaceOut.out", getpid());
    RaceOut.open(filename);
    RaceOut.setf(ios::showbase);
    
    sprintf(filename,"%dConflictOut.out", getpid());
    ConflictOut.open(filename);
    ConflictOut.setf(ios::showbase);
    
    sprintf(filename,"%dAllEventOut.out", getpid());
    AllEventOut.open(filename);
    AllEventOut.setf(ios::showbase);
    
    sprintf(filename,"%dFrame.out", getpid());
    FrameOut.open(filename);
    FrameOut.setf(ios::showbase);
    
    sprintf(filename,"%dAllMetrics.out", getpid());
    AllMetrics.open(filename);
    AllMetrics.setf(ios::showbase);
}

void CloseFileOutput()
{
    RaceOut.close();
    ConflictOut.close();
    AllEventOut.close();
    FrameOut.close();
    AllMetrics.close();
}

int Usage()
{
    cerr << "This is the invocation pintool" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

void RecordEvents(int Type, UINT32 threadID, ADDRINT ObjectAddress)
{
    switch(Type)
    {
        case 1:
            ThreadNum++;
            break;
        case 4:
            LockNum++;
            break;
        case 3:
        case 13:
        case 6:
        case 7:
        case 8:
        case 9:
            SynNum++;
            break;
    }

    AllEventOut<<dec<<Type<<" "<<threadID<<" "<<hex<<ObjectAddress<<dec<<endl;
}

void RecordFrames()
{
    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    for(ITforAllThread=AllThread.begin(); ITforAllThread!=AllThread.end(); ITforAllThread++)
    {
        FrameOut<<"---------->>>>> "<<dec<<ITforAllThread->first<<" <<<<<----------"<<endl;
        list<struct RWVecTime>::iterator ITforVecTimeList;
        int Fnum=0;
        for(ITforVecTimeList=((ITforAllThread->second).VecTimeList).begin(); ITforVecTimeList!=((ITforAllThread->second).VecTimeList).end(); ITforVecTimeList++)
        {
            Fnum++;
            FrameOut<<"Frame: "<<Fnum<<"    Syn: "<<ITforVecTimeList->SynName<<endl;
            FrameOut<<"VT ";
            map<UINT32, long>::iterator ITforVecTime;
            for(ITforVecTime=(ITforVecTimeList->VecTime).begin(); ITforVecTime!=(ITforVecTimeList->VecTime).end(); ITforVecTime++)
            {
                FrameOut<<dec<<ITforVecTime->first<<":"<<ITforVecTime->second<<" ";
            }
            FrameOut<<endl;
            FrameOut<<"Locks: ";
            set<ADDRINT>::iterator ITforlocks;
            for(ITforlocks=(ITforVecTimeList->locks).begin(); ITforlocks!=(ITforVecTimeList->locks).end(); ITforlocks++)
            {
                FrameOut<<hex<<*ITforlocks<<" ";
            }
            FrameOut<<endl;
            FrameOut<<"Access: ";
            unordered_map<ADDRINT, SharedMemoryAccessInf>::iterator ITforSharedLocation;
            for(ITforSharedLocation=(ITforVecTimeList->SharedLocation).begin(); ITforSharedLocation!=(ITforVecTimeList->SharedLocation).end(); ITforSharedLocation++)
            {
                FrameOut<<hex<<ITforSharedLocation->first<<" ";
            }
            FrameOut<<endl<<endl;
        }
    }
}

void GetMaxMemory() //count memory used
{
    size_t mysum=0;
    map<UINT32, ThreadVecTime>::iterator AllIT;
    for(AllIT=AllThread.begin();AllIT!=AllThread.end();AllIT++)
    {
        mysum+=sizeof(UINT32);
        mysum+=sizeof((AllIT->second).ListAddress);

        list<struct RWVecTime>::iterator listIT;
        for(listIT=((AllIT->second).VecTimeList).begin();listIT!=((AllIT->second).VecTimeList).end();listIT++)
        {
            mysum+=sizeof(listIT->SynName)+sizeof(bool)+sizeof(bool);
            mysum+=(listIT->VecTime).size() * (sizeof(UINT32) + sizeof(long));
            mysum+=(listIT->locks).size()*sizeof(ADDRINT);
            unordered_map<ADDRINT, SharedMemoryAccessInf>::iterator ITforSharedLocation;
            for(ITforSharedLocation=(listIT->SharedLocation).begin(); ITforSharedLocation!=(listIT->SharedLocation).end();ITforSharedLocation++)
            {
                mysum+=sizeof(ADDRINT)+sizeof(bool)+sizeof(bool)+((ITforSharedLocation->second).InsR).size()*sizeof(ADDRINT)+((ITforSharedLocation->second).InsW).size()*sizeof(ADDRINT);
            }
        }
    }
    if(mysum > MaxSum)
        MaxSum=mysum;
}

void OutputAllMetrics()
{
    AllMetrics<<"NumberOfThreads: "<<dec<<ThreadNum<<endl;
    AllMetrics<<"Locations: "<<ShareVarAddress.size()<<endl;
    AllMetrics<<"Locks: "<<LockNum<<endl;
    AllMetrics<<"Syns: "<<SynNum<<endl;
    AllMetrics<<"Races: "<<Race_Warnings<<endl;
    AllMetrics<<"Conflicts: "<<Conflict_Warnings<<endl;
    AllMetrics<<"Analysis: "<<NumAnalysis<<endl;
    AllMetrics<<"ConFrames: "<<ConNumFrames<<endl;
    AllMetrics<<"Space: "<<MaxSum<<endl;
}

VOID Fini(INT32 code, VOID *v)
{
    RecordFrames();
    GetMaxMemory();
    monitorendflag=true;
    OutputAllMetrics();
}
