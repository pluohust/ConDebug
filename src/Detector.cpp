#include "Detector.h"
#include <sstream>

using namespace std::tr1;

extern map<UINT32, ThreadVecTime> AllThread;
extern long Race_Warnings;
extern long Conflict_Warnings;
extern long ConNumFrames;
extern long NumAnalysis;
extern map<ADDRINT, MallocInf> MallocSize;

extern ofstream RaceOut;
extern ofstream ConflictOut;

set<struct StringTuple> RecondAllConflict; //conflict
long numofconflict=0;
set<struct StringTuple> RecondAllRace; //race
long numofrace=0;

void AddOneConFrame()
{
    ConNumFrames=ConNumFrames+1;
}

void AddOneAnalysis()
{
    NumAnalysis=NumAnalysis+1;
}

void AddOneRace()
{
    Race_Warnings=Race_Warnings+1;
}

void AddOneConflict()
{
    Conflict_Warnings=Conflict_Warnings+1;
}

bool CheckLock(const set<ADDRINT> &lockone, const set<ADDRINT> &locktwo) //check critical sections
{
    set<ADDRINT>::const_iterator setformain, setforanother;
    if(lockone.empty() || locktwo.empty())
        return true;
    for(setformain=lockone.begin(); setformain!=lockone.end(); setformain++)
    {
        setforanother=locktwo.find(*setformain);
        if(setforanother!=locktwo.end())
            return false;
    }
    return true;
}

bool IsInMallocSize(ADDRINT address, string &codeinf)
{
    map<ADDRINT, MallocInf>::iterator loopmalloc;
    for(loopmalloc=MallocSize.begin();loopmalloc!=MallocSize.end();loopmalloc++)
    {
        if(address>=loopmalloc->first && address<(loopmalloc->first+(loopmalloc->second).size))
        {
            codeinf=(loopmalloc->second).codeinf;
            return true;
        }
    }
    return false;
}

void InsertResult_Con(ADDRINT address, map<ADDRINT, ResultInf> &Result, string codeinf)
{
    map<ADDRINT, ResultInf>::iterator findaddress;
    findaddress=Result.find(address);
    if(findaddress==Result.end())
    {
        Result[address]=(struct ResultInf){1,codeinf};
    }
    else
    {
        (findaddress->second).number=(findaddress->second).number+1;
    }
}

void InsertResult_simple(set<string> &Result, const string &codeinf)
{
    Result.insert(codeinf);
}

bool CompareVC(THREADID threadid, map<UINT32, long> &OldVC, map<UINT32, long> &NewVC)
{
    map<UINT32, long>::iterator OldItVC;
    map<UINT32, long>::iterator NewItVC;
    
    OldItVC=OldVC.find(threadid);
    NewItVC=NewVC.find(threadid);
    if((OldItVC!=OldVC.end())&&(NewItVC!=NewVC.end())&&(OldItVC->second<NewItVC->second))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void GetSource(const set<ADDRINT> &InsAddress, string &Source)
{
    set<ADDRINT>::const_iterator loopins;
    PIN_LockClient();
    for(loopins=InsAddress.begin(); loopins!=InsAddress.end(); loopins++)
    {
        string filename;
        INT32 line;
        PIN_GetSourceLocation(*loopins, NULL, &line, &filename);
        if(line>1)
        {
            ostringstream tmposs;
            tmposs<<filename<<" : "<<line<<endl;
            Source=Source+tmposs.str();
        }
    }
    PIN_UnlockClient();
}

void DetectRace(const unordered_map<ADDRINT, SharedMemoryAccessInf> &MemoryOne, const unordered_map<ADDRINT, SharedMemoryAccessInf> &MemoryTwo, UINT32 threadIDOne, UINT32 threadIDTwo)
{
    unordered_map<ADDRINT, SharedMemoryAccessInf>::const_iterator MapforOne;
    unordered_map<ADDRINT, SharedMemoryAccessInf>::const_iterator MapforTwo;
    for(MapforOne=MemoryOne.begin();MapforOne!=MemoryOne.end();MapforOne++)
    {
        MapforTwo=MemoryTwo.find(MapforOne->first);
        if(MapforTwo!=MemoryTwo.end())
        {
            string sourcecodeOneR, sourcecodeOneW;
            string sourcecodeTwoR, sourcecodeTwoW;
            if((MapforOne->second).Rstatus && (MapforTwo->second).Wstatus)
            {
                AddOneRace();
                GetSource((MapforOne->second).InsR, sourcecodeOneR);
                GetSource((MapforTwo->second).InsW, sourcecodeTwoW);
                if((sourcecodeOneR.length()>1) && (sourcecodeTwoW.length()>1))
                {
                    struct StringTuple tmptuple;
                    tmptuple.one=sourcecodeOneR < sourcecodeTwoW ? sourcecodeOneR : sourcecodeTwoW;
                    tmptuple.two=sourcecodeTwoW > sourcecodeOneR ? sourcecodeTwoW : sourcecodeOneR;
                    set<struct StringTuple>::iterator findrace;
                    findrace=RecondAllRace.find(tmptuple);
                    if(findrace==RecondAllRace.end())
                    {
                            RecondAllRace.insert(tmptuple);
                            numofrace=numofrace+1;
                            RaceOut<<"The "<<numofrace<<"th Race"<<endl;
                            RaceOut<<tmptuple.one<<endl;
                            RaceOut<<tmptuple.two<<endl<<endl;
                    }
                }
            }
            if((MapforOne->second).Wstatus && (MapforTwo->second).Rstatus)
            {
                AddOneRace();
                GetSource((MapforOne->second).InsW, sourcecodeOneW);
                GetSource((MapforTwo->second).InsR, sourcecodeTwoR);
                if((sourcecodeOneW.length()>1) && (sourcecodeTwoR.length()>1))
                {
                    struct StringTuple tmptuple;
                    tmptuple.one=sourcecodeOneW < sourcecodeTwoR ? sourcecodeOneW : sourcecodeTwoR;
                    tmptuple.two=sourcecodeTwoR > sourcecodeOneW ? sourcecodeTwoR : sourcecodeOneW;
                    set<struct StringTuple>::iterator findrace;
                    findrace=RecondAllRace.find(tmptuple);
                    if(findrace==RecondAllRace.end())
                    {
                            RecondAllRace.insert(tmptuple);
                            numofrace=numofrace+1;
                            RaceOut<<"The "<<numofrace<<"th Race"<<endl;
                            RaceOut<<tmptuple.one<<endl;
                            RaceOut<<tmptuple.two<<endl<<endl;
                    }
                }
            }
            if((MapforOne->second).Wstatus && (MapforTwo->second).Wstatus)
            {
                AddOneRace();
                GetSource((MapforOne->second).InsW, sourcecodeOneW);
                GetSource((MapforTwo->second).InsW, sourcecodeTwoW);
                if((sourcecodeOneW.length()>1) && (sourcecodeTwoW.length()>1))
                {
                    struct StringTuple tmptuple;
                    tmptuple.one=sourcecodeOneW < sourcecodeTwoW ? sourcecodeOneW : sourcecodeTwoW;
                    tmptuple.two=sourcecodeTwoW > sourcecodeOneW ? sourcecodeTwoW : sourcecodeOneW;
                    set<struct StringTuple>::iterator findrace;
                    findrace=RecondAllRace.find(tmptuple);
                    if(findrace==RecondAllRace.end())
                    {
                            RecondAllRace.insert(tmptuple);
                            numofrace=numofrace+1;
                            RaceOut<<"The "<<numofrace<<"th Race"<<endl;
                            RaceOut<<tmptuple.one<<endl;
                            RaceOut<<tmptuple.two<<endl<<endl;
                    }
                }
            }
        }
    }
}

void DetectConflict(const unordered_map<ADDRINT, SharedMemoryAccessInf> &MemoryOne, const unordered_map<ADDRINT, SharedMemoryAccessInf> &MemoryTwo, UINT32 threadIDOne, UINT32 threadIDTwo)
{
    unordered_map<ADDRINT, SharedMemoryAccessInf>::const_iterator MapforOne;
    unordered_map<ADDRINT, SharedMemoryAccessInf>::const_iterator MapforTwo;
    for(MapforOne=MemoryOne.begin();MapforOne!=MemoryOne.end();MapforOne++)
    {
        MapforTwo=MemoryTwo.find(MapforOne->first);
        if(MapforTwo!=MemoryTwo.end())
        {
            string sourcecodeOneR, sourcecodeOneW;
            string sourcecodeTwoR, sourcecodeTwoW;
            if((MapforOne->second).Rstatus && (MapforTwo->second).Wstatus)
            {
                GetSource((MapforOne->second).InsR, sourcecodeOneR);
                GetSource((MapforTwo->second).InsW, sourcecodeTwoW);
                if((sourcecodeOneR.length()>1) && (sourcecodeTwoW.length()>1))
                {
                    AddOneConflict();
                    
                    struct StringTuple tmptuple;
                    tmptuple.one=sourcecodeOneR < sourcecodeTwoW ? sourcecodeOneR : sourcecodeTwoW;
                    tmptuple.two=sourcecodeTwoW > sourcecodeOneR ? sourcecodeTwoW : sourcecodeOneR;
                    set<struct StringTuple>::iterator findconflict;
                    findconflict=RecondAllConflict.find(tmptuple);
                    if(findconflict==RecondAllConflict.end())
                    {
                        RecondAllConflict.insert(tmptuple);
                        numofconflict=numofconflict+1;
                        ConflictOut<<"The "<<numofconflict<<"th Conflict"<<endl;
                        ConflictOut<<tmptuple.one<<endl;
                        ConflictOut<<tmptuple.two<<endl<<endl;
                    }
                }
            }
            if((MapforOne->second).Wstatus && (MapforTwo->second).Rstatus)
            {
                GetSource((MapforOne->second).InsW, sourcecodeOneW);
                GetSource((MapforTwo->second).InsR, sourcecodeTwoR);
                if((sourcecodeOneW.length()>1) && (sourcecodeTwoR.length()>1))
                {
                    AddOneConflict();
                    struct StringTuple tmptuple;
                    tmptuple.one=sourcecodeOneW < sourcecodeTwoR ? sourcecodeOneW : sourcecodeTwoR;
                    tmptuple.two=sourcecodeTwoR > sourcecodeOneW ? sourcecodeTwoR : sourcecodeOneR;
                    set<struct StringTuple>::iterator findconflict;
                    findconflict=RecondAllConflict.find(tmptuple);
                    if(findconflict==RecondAllConflict.end())
                    {
                        RecondAllConflict.insert(tmptuple);
                        numofconflict=numofconflict+1;
                        ConflictOut<<"The "<<numofconflict<<"th Conflict"<<endl;
                        ConflictOut<<tmptuple.one<<endl;
                        ConflictOut<<tmptuple.two<<endl<<endl;
                    }
                }
            }
            if((MapforOne->second).Wstatus && (MapforTwo->second).Wstatus)
            {
                
                GetSource((MapforOne->second).InsW, sourcecodeOneW);
                GetSource((MapforTwo->second).InsW, sourcecodeTwoW);
                if((sourcecodeOneW.length()>1) && (sourcecodeTwoW.length()>1))
                {
                    AddOneConflict();
                    struct StringTuple tmptuple;
                    tmptuple.one=sourcecodeOneW < sourcecodeTwoW ? sourcecodeOneW : sourcecodeTwoW;
                    tmptuple.two=sourcecodeTwoW > sourcecodeOneW ? sourcecodeTwoW : sourcecodeOneW;
                    set<struct StringTuple>::iterator findconflict;
                    findconflict=RecondAllConflict.find(tmptuple);
                    if(findconflict==RecondAllConflict.end())
                    {
                        RecondAllConflict.insert(tmptuple);
                        numofconflict=numofconflict+1;
                        ConflictOut<<"The "<<numofconflict<<"th Conflict"<<endl;
                        ConflictOut<<tmptuple.one<<endl;
                        ConflictOut<<tmptuple.two<<endl<<endl;
                    }
                }
            }
        }
    }
}

/* each detection analysis for one thread */
void VectorDetect(THREADID threadID, ThreadVecTime &TargetThread)
{
    map<UINT32, ThreadVecTime>::iterator LoopItDetect;
    list<struct RWVecTime>::iterator MainItListDetect;
    list<struct RWVecTime>::iterator AnotherItListBeDetected;
    
    if((TargetThread.VecTimeList).empty())
    {
        return;
    }

    MainItListDetect=TargetThread.ListAddress;

    for(LoopItDetect=AllThread.begin();LoopItDetect!=AllThread.end();LoopItDetect++)
    {
        if((LoopItDetect->first!=threadID)&&((LoopItDetect->second).VecTimeList).size()>1)
        {
            AnotherItListBeDetected=(LoopItDetect->second).ListAddress;
            AnotherItListBeDetected--;
            for(;AnotherItListBeDetected!=((LoopItDetect->second).VecTimeList).end();AnotherItListBeDetected--)
            {
                AddOneAnalysis();
                if(CompareVC(LoopItDetect->first,AnotherItListBeDetected->VecTime,MainItListDetect->VecTime))
                {
                    break;
                }
                AddOneConFrame();
                if((MainItListDetect->Writeflag && AnotherItListBeDetected->Accessflag) || (MainItListDetect->Accessflag && AnotherItListBeDetected->Writeflag))
                {
                    set<ADDRINT> tmpInter;
                    set_intersection((MainItListDetect->locks).begin(), (MainItListDetect->locks).end(), (AnotherItListBeDetected->locks).begin(), (AnotherItListBeDetected->locks).end(), inserter(tmpInter, tmpInter.begin()));
                    if(tmpInter.empty())
                        DetectRace(MainItListDetect->SharedLocation, AnotherItListBeDetected->SharedLocation, threadID, LoopItDetect->first);
                    else
                        DetectConflict(MainItListDetect->SharedLocation, AnotherItListBeDetected->SharedLocation, threadID, LoopItDetect->first);
                }
            }
        }
    }
}
