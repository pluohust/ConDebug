#include "VectorClocks.h"
#include "Init_Close.h"
#include "Detector.h"
#include <sstream>

/*please forgive my laziness.*/
extern map<UINT32, ThreadVecTime> AllThread;
extern map<THREADID, ThreadParent> ThreadMapParent;
extern map<OS_THREAD_ID, THREADID> ThreadIDInf;
extern PIN_LOCK lock;
extern PIN_LOCK shareaddrlock;
extern map<THREADID, RecordCond> VecWait;
extern map<THREADID, ADDRINT> AfterLockInf;
map<THREADID, string> LockFunAddress;
extern bool monitorendflag;
extern bool realendflag;
extern map<ADDRINT, map<UINT32, long> > SignalVecTime;
//extern map<ADDRINT, map<UINT32, long> > LockVecTime;

extern map<UINT32, queue<map<UINT32, long> > > CreateThreadVecTime;
extern map<UINT32, map<UINT32, long> > FiniThreadVecTime;

extern map<THREADID, map<ADDRINT, LockVecTime> > AllLockVecTime;

extern vector<ShareAddreeStruct> MallocVec;

map<THREADID, ADDRINT> TryLockInf; // new add 

//barrier
map<ADDRINT, map<THREADID, long> > BarrierVC; // VC of each barrier
map<THREADID, ADDRINT> ThreadBarrier; //record the relation between thread id and barrier

extern ofstream AllEventOut;

static void UpdateVC(map<THREADID, long> &destVC, const map<THREADID, long> sourceVC)
{
    map<THREADID, long>::iterator iteratordestvc;
    map<THREADID, long>::const_iterator iteratorsourcevc;
    for(iteratorsourcevc=sourceVC.begin(); iteratorsourcevc!=sourceVC.end(); iteratorsourcevc++)
    {
        iteratordestvc=destVC.find(iteratorsourcevc->first);
        if(iteratordestvc==destVC.end())
        {
            destVC[iteratorsourcevc->first]=iteratorsourcevc->second;
        }
        else
        {
            destVC[iteratorsourcevc->first]=iteratorsourcevc->second > iteratordestvc->second ? iteratorsourcevc->second : iteratordestvc->second;
        }
    }
}

void InitThreadVecTime(UINT32 threadID)  //first frame when thread start
{
    map<UINT32, ThreadVecTime>::iterator ITforallthread;
    RWVecTime tmpRWVecTime;
    (tmpRWVecTime.VecTime)[threadID]=1;
    tmpRWVecTime.SynName="FirstFrame";

    ThreadVecTime tmpThreadVecTime;
    (tmpThreadVecTime.VecTimeList).push_back(tmpRWVecTime);

    AllThread.insert(pair<UINT32, ThreadVecTime>(threadID, tmpThreadVecTime));
    ITforallthread=AllThread.find(threadID);
    if(AllThread.end()==ITforallthread)
    {
        return;
    }
    
    (ITforallthread->second).ListAddress=((ITforallthread->second).VecTimeList).begin();

}

void CreateNewFrame(THREADID threadid, ThreadVecTime &TargetThread, string SynName)
{
    RWVecTime NewFrame;
    NewFrame.VecTime=(TargetThread.ListAddress)->VecTime;
    NewFrame.locks=(TargetThread.ListAddress)->locks;
    NewFrame.SynName=SynName;
    map<UINT32, long>::iterator mapvectime; //to find the thread's vec time
    mapvectime=(NewFrame.VecTime).find(threadid);
    if(mapvectime==(NewFrame.VecTime).end())
    {
        exit(-1);
    }
    mapvectime->second+=1;
    (TargetThread.VecTimeList).push_back(NewFrame);
    
    TargetThread.ListAddress=(TargetThread.VecTimeList).end();
    (TargetThread.ListAddress)--;
    if(TargetThread.ListAddress==(TargetThread.VecTimeList).end())
    {
        exit(-1);
    }
    list<struct RWVecTime>::iterator TestTheLastIterator;
    TestTheLastIterator=TargetThread.ListAddress;
    TestTheLastIterator++;
    if(TestTheLastIterator!=(TargetThread.VecTimeList).end())
    {
        exit(-1);
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    RecordType GetOne;
    OS_THREAD_ID tmp;
    map<OS_THREAD_ID, THREADID>::iterator maptmp;
    struct ThreadVecTime OneThreadVecInf;
    set<ADDRINT>::iterator ITSharedVar;
    PIN_GetLock(&lock, threadid+1);
    if(INVALID_OS_THREAD_ID!=(tmp=PIN_GetParentTid()))
    {
        maptmp=ThreadIDInf.find(tmp);
        if(maptmp!=ThreadIDInf.end())
        {
            GetOne=(RecordType){1, threadid, (ADDRINT)maptmp->second};
            ThreadMapParent[threadid]=(ThreadParent) {true, maptmp->second}; //Just for thread join
        }
        else
        {
            GetOne=(RecordType){1, threadid, 0};
        }
    }
    else
    {
        GetOne=(RecordType){1, threadid, 0};
    }
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);
    
    ThreadIDInf.insert(make_pair(PIN_GetTid(), threadid));
    
    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    if(AllThread.empty()) //first thread
    {
        InitThreadVecTime(GetOne.threadID);
    }
    else
    {
        list<struct RWVecTime>::iterator ITtmplist;
        for(ITforAllThread=AllThread.begin();ITforAllThread!=AllThread.end();ITforAllThread++)
        {
            for(ITtmplist=((ITforAllThread->second).VecTimeList).begin();ITtmplist!=((ITforAllThread->second).VecTimeList).end();ITtmplist++)
                (ITtmplist->VecTime).insert(pair<UINT32, long>(GetOne.threadID,0));
        }
        InitThreadVecTime(GetOne.threadID);
        ITforAllThread=AllThread.find(GetOne.threadID);
        if(AllThread.end()==ITforAllThread)
        {
            PIN_ReleaseLock(&lock);
            return;
        }
        map<UINT32, queue<map<UINT32, long> > >::iterator mapfindfather;
        mapfindfather=CreateThreadVecTime.find((UINT32)GetOne.object);
        if(mapfindfather!=CreateThreadVecTime.end())
        {
            ((ITforAllThread->second).ListAddress)->VecTime=(mapfindfather->second).front(); //从父线程中继承vec time
            (mapfindfather->second).pop();
            (((ITforAllThread->second).ListAddress)->VecTime).insert(pair<UINT32, long>(GetOne.threadID,1));
        }
        else
        {
            ITforAllThread=AllThread.find(GetOne.threadID);
            if(AllThread.end()==ITforAllThread)
            {
                PIN_ReleaseLock(&lock);
                return;
            }
            (((ITforAllThread->second).ListAddress)->VecTime).insert(pair<UINT32, long>(GetOne.threadID,1));
        }
    }
    
    PIN_ReleaseLock(&lock);
}

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    PIN_GetLock(&lock, threadid+1);

    RecordType GetOne;
    GetOne=(RecordType) {2, threadid, 0};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);
    
    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }
    
    VectorDetect(GetOne.threadID,ITforAllThread->second);

    CreateNewFrame(threadid,ITforAllThread->second,"ThreadFini");

    FiniThreadVecTime[GetOne.threadID]=((ITforAllThread->second).ListAddress)->VecTime;

    PIN_ReleaseLock(&lock);
}

VOID BeforePthread_create(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    RecordType GetOne;
    GetOne=(RecordType) {13, threadid, 0};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }
    
    VectorDetect(GetOne.threadID,ITforAllThread->second);

    CreateNewFrame(threadid,ITforAllThread->second,"ThreadCreate");

    map<UINT32, queue<map<UINT32, long> > >::iterator mapforqueue; //to record the father thread's inf
    mapforqueue=CreateThreadVecTime.find(GetOne.threadID);
    if(mapforqueue==CreateThreadVecTime.end()) //the first time to create child thread
    {
        queue<map<UINT32, long> > queuetmp;
        queuetmp.push(((ITforAllThread->second).ListAddress)->VecTime);
        CreateThreadVecTime.insert(pair<UINT32, queue<map<UINT32, long> > >(GetOne.threadID, queuetmp));
    }
    else
    {
        (mapforqueue->second).push(((ITforAllThread->second).ListAddress)->VecTime);
    }

    PIN_ReleaseLock(&lock);
}

VOID AfterPthread_join(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    RecordType GetOne;
    map<THREADID, ThreadParent>::iterator itertmp;
    for(itertmp=ThreadMapParent.begin();itertmp!=ThreadMapParent.end();itertmp++)
    {
        if((itertmp->second).fatherthreadid==threadid)
            break;
    }
    if(itertmp!=ThreadMapParent.end())
    {
        GetOne=(RecordType) {3, threadid, (ADDRINT)(itertmp->first)};
        
        ThreadMapParent.erase(itertmp);
    }
    else
    {
        GetOne=(RecordType) {3, threadid, 0};
    }
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }

    VectorDetect(GetOne.threadID,ITforAllThread->second);

    CreateNewFrame(threadid,ITforAllThread->second,"ThreadJoin");

    map<UINT32, long>::iterator mapvectimemain; //to find the thread's vec time
    map<UINT32, long>::iterator mapvectimechild; //to find child's vec time
    map<UINT32, ThreadVecTime>::iterator mapfini; //to find fini
    mapfini=AllThread.find((THREADID)(GetOne.object));
    if(mapfini==AllThread.end())
    {
        exit(-1);
    }
    for(mapvectimechild=(((mapfini->second).ListAddress)->VecTime).begin();mapvectimechild!=(((mapfini->second).ListAddress)->VecTime).end();mapvectimechild++)
    {
        mapvectimemain=(((ITforAllThread->second).ListAddress)->VecTime).find(mapvectimechild->first);
        if(mapvectimemain==(((ITforAllThread->second).ListAddress)->VecTime).end())
        {
            (((ITforAllThread->second).ListAddress)->VecTime)[mapvectimechild->first]=mapvectimechild->second+1;
        }
        else
        {
            mapvectimemain->second=(mapvectimechild->second+1) > mapvectimemain->second ? (mapvectimechild->second+1) : mapvectimemain->second;
        }
    }
    FiniThreadVecTime.erase((THREADID)(GetOne.object));

    PIN_ReleaseLock(&lock);
}

void UpdateVecTime(map<UINT32, long> &OldVC, map<UINT32, long> &NewVC)
{
    map<UINT32, long>::iterator ITforOldVC;
    map<UINT32, long>::iterator ITforNewVC;
    
    for(ITforOldVC=OldVC.begin(); ITforOldVC!=OldVC.end(); ITforOldVC++)
    {
        ITforNewVC=NewVC.find(ITforOldVC->first);
        if(ITforNewVC==NewVC.end())
        {
            NewVC[ITforOldVC->first]=ITforOldVC->second;
        }
        else
        {
            ITforNewVC->second=ITforNewVC->second > ITforOldVC->second ? ITforNewVC->second:ITforOldVC->second;
        }
    }
}

void UpdateLockVecTime(ADDRINT currentlock, THREADID threadid, map<UINT32, long> &VecTime)
{
    map<THREADID, map<ADDRINT, LockVecTime> >::iterator ITforAllLockVecTime;
    for(ITforAllLockVecTime=AllLockVecTime.begin(); ITforAllLockVecTime!=AllLockVecTime.end(); ITforAllLockVecTime++)
    {
        if(ITforAllLockVecTime->first!=threadid)
        {
            map<ADDRINT, LockVecTime>::iterator ITforOneLock;
            ITforOneLock=(ITforAllLockVecTime->second).find(currentlock);
            if(ITforOneLock!=(ITforAllLockVecTime->second).end())
            {
                if(CompareVC(ITforAllLockVecTime->first, (ITforOneLock->second).OldAcquire, VecTime))
                {
                    UpdateVecTime((ITforOneLock->second).NewRelease, VecTime);
                }
            }
        }
    }
}

void InsertVecTimeToLock(ADDRINT currentlock, THREADID threadid, map<UINT32, long> &VecTime, int type)
{
    map<THREADID, map<ADDRINT, LockVecTime> >::iterator ITforAllLockVecTime;
    ITforAllLockVecTime=AllLockVecTime.find(threadid);
    if(ITforAllLockVecTime==AllLockVecTime.end())
    {
        LockVecTime tmpLockVecTime;
        switch(type)
        {
            case 4:
                tmpLockVecTime.OldAcquire=VecTime;
                break;
            case 5:
                tmpLockVecTime.NewRelease=VecTime;
                break;
        }
        map<ADDRINT, LockVecTime> tmpOneLock;
        tmpOneLock[currentlock]=tmpLockVecTime;
        AllLockVecTime[threadid]=tmpOneLock;
    }
    else
    {
        map<ADDRINT, LockVecTime>::iterator ITforOneLock;
        ITforOneLock=(ITforAllLockVecTime->second).find(currentlock);
        if(ITforOneLock==(ITforAllLockVecTime->second).end())
        {
            LockVecTime tmpLockVecTime;
            switch(type)
            {
                case 4:
                    tmpLockVecTime.OldAcquire=VecTime;
                    break;
                case 5:
                    tmpLockVecTime.NewRelease=VecTime;
                    break;
            }
            (ITforAllLockVecTime->second)[currentlock]=tmpLockVecTime;
        }
        else
        {
            switch(type)
            {
                case 4:
                    (ITforOneLock->second).OldAcquire=VecTime;
                    break;
                case 5:
                    (ITforOneLock->second).NewRelease=VecTime;
                    break;
            }
        }
    }
}

void JustBeforeLock(ADDRINT currentlock, THREADID threadid)
{
    RecordType GetOne;
    GetOne=(RecordType) {4, threadid, currentlock};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);
    
    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }

    VectorDetect(GetOne.threadID,ITforAllThread->second);
    InsertVecTimeToLock(currentlock, threadid, ((ITforAllThread->second).ListAddress)->VecTime, 4); //add vector time to lock-acquire
    CreateNewFrame(threadid,ITforAllThread->second,"Lock");
    ((ITforAllThread->second).ListAddress)->locks.insert(currentlock); // add lock to lock set
    UpdateLockVecTime(currentlock, threadid, ((ITforAllThread->second).ListAddress)->VecTime); //update the vector time based on lock operations
//    map<ADDRINT, map<UINT32, long> >::iterator findthelock;
//    findthelock=LockVecTime.find(currentlock);
//    if(findthelock!=LockVecTime.end())
//    {
//        map<UINT32, long>::iterator mainvc, anothervc;
//        for(anothervc=(findthelock->second).begin();anothervc!=(findthelock->second).end();anothervc++)
//        {
//            mainvc=(((ITforAllThread->second).ListAddress)->VecTime).find(anothervc->first);
//            if(mainvc==(((ITforAllThread->second).ListAddress)->VecTime).end())
//            {
//                (((ITforAllThread->second).ListAddress)->VecTime)[anothervc->first]=anothervc->second;
//            }
//            else
//            {
//                mainvc->second=anothervc->second > mainvc->second ? anothervc->second : mainvc->second;
//            }
//        }
//    }
}

static void GetFunSource(const ADDRINT &FunAddress, string &Source)
{
    PIN_LockClient();
    string filename;
    INT32 line;
    PIN_GetSourceLocation(FunAddress, NULL, &line, &filename);
    if(line>1)
    {
        ostringstream tmposs;
        tmposs<<filename<<" : "<<line-1;
        Source=Source+tmposs.str();
    }
    PIN_UnlockClient();
}

VOID BeforePthread_mutex_lock(ADDRINT currentlock, THREADID threadid, ADDRINT lockfunaddr)
{
    PIN_GetLock(&lock, threadid+1);

    AfterLockInf[threadid]=currentlock;

    string lockcodeaddress;
    GetFunSource(lockfunaddr, lockcodeaddress);
    LockFunAddress[threadid]=lockcodeaddress;

    PIN_ReleaseLock(&lock);
}

VOID AfterPthread_mutex_lock(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    map<THREADID, ADDRINT>::iterator findlock;
    findlock=AfterLockInf.find(threadid);
    if(findlock==AfterLockInf.end())
    {
        exit(-1);
    }

    JustBeforeLock(findlock->second, threadid);

    map<THREADID, string>::iterator findlockaddr;
    findlockaddr=LockFunAddress.find(threadid);
    if(findlockaddr==LockFunAddress.end())
    {
        exit(-1);
    }
    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(threadid);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }
    ((ITforAllThread->second).ListAddress)->LockCode=findlockaddr->second;

    PIN_ReleaseLock(&lock);
}

VOID BeforePthread_mutex_trylock(ADDRINT currentlock, THREADID threadid, ADDRINT lockfunaddr)
{
    PIN_GetLock(&lock, threadid+1);

    TryLockInf[threadid]=currentlock;

    string lockcodeaddress;
    GetFunSource(lockfunaddr, lockcodeaddress);
    LockFunAddress[threadid]=lockcodeaddress;

    PIN_ReleaseLock(&lock);
}

VOID AfterPthread_mutex_trylock(int flag, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    if(0==flag)
    {
        map<THREADID, ADDRINT>::iterator findlock;
        findlock=TryLockInf.find(threadid);
        if(findlock==TryLockInf.end())
        {
            exit(-1);
        }

        JustBeforeLock(findlock->second, threadid);

        map<THREADID, string>::iterator findlockaddr;
        findlockaddr=LockFunAddress.find(threadid);
        if(findlockaddr==LockFunAddress.end())
        {
            exit(-1);
        }
        map<UINT32, ThreadVecTime>::iterator ITforAllThread;
        ITforAllThread=AllThread.find(threadid);
        if(AllThread.end()==ITforAllThread)
        {
            PIN_ReleaseLock(&lock);
            return;
        }
        ((ITforAllThread->second).ListAddress)->LockCode=findlockaddr->second;
    }
    PIN_ReleaseLock(&lock);
}

/*please focus on the nesting cases*/
VOID BeforePthread_mutex_unlock(ADDRINT currentlock, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    RecordType GetOne;
    GetOne=(RecordType) {5, threadid, currentlock};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);
    
    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }

    VectorDetect(GetOne.threadID,ITforAllThread->second);
    
    CreateNewFrame(threadid,ITforAllThread->second,"MutexUnlock");
    ((ITforAllThread->second).ListAddress)->locks.erase(currentlock); // remove lock from lock set
    InsertVecTimeToLock(currentlock, threadid, ((ITforAllThread->second).ListAddress)->VecTime, 5); //add vector time to lock-acquire
//    LockVecTime[currentlock]=((ITforAllThread->second).ListAddress)->VecTime;

    PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_wait(ADDRINT cond, ADDRINT mutex, THREADID threadid)
{
    BeforePthread_mutex_unlock(mutex,threadid);
    PIN_GetLock(&lock, threadid+1);
    VecWait[threadid]=((RecordCond) {6, threadid, cond, mutex});
    PIN_ReleaseLock(&lock);
}

VOID AfterPthread_cond_wait(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    RecordCond GetOne;
    map<THREADID, RecordCond>::iterator itwait;
    itwait=VecWait.find(threadid);
    if(itwait==VecWait.end())
    {
        exit(-1);
    }
    GetOne=(RecordCond) {(itwait->second).style, (itwait->second).threadID, (itwait->second).cond, (itwait->second).mutex};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.cond);
    
    VecWait.erase(itwait);

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        exit(-1);;
    }
    
    VectorDetect(GetOne.threadID,ITforAllThread->second);

    CreateNewFrame(threadid,ITforAllThread->second,"CondWait");

    map<UINT32, long>::iterator mapvectimemain;
    map<UINT32, long>::iterator mapvectimechild; //to find thread's vec time
    map<ADDRINT, map<UINT32, long> >::iterator mapsignal; //to find signal
    mapsignal=SignalVecTime.find(GetOne.cond);
    if(mapsignal==SignalVecTime.end())
    {
        PIN_ReleaseLock(&lock);
        return;
    }
    for(mapvectimechild=(mapsignal->second).begin();mapvectimechild!=(mapsignal->second).end();mapvectimechild++)
    {
        mapvectimemain=(((ITforAllThread->second).ListAddress)->VecTime).find(mapvectimechild->first);
        if(mapvectimemain==(((ITforAllThread->second).ListAddress)->VecTime).end())
        {
            (((ITforAllThread->second).ListAddress)->VecTime)[mapvectimechild->first]=mapvectimechild->second;
        }
        else
        {
            mapvectimemain->second=mapvectimechild->second > mapvectimemain->second ? mapvectimechild->second : mapvectimemain->second;
        }
    }

    JustBeforeLock(GetOne.mutex, threadid);

    PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_timedwait(ADDRINT cond, ADDRINT mutex, THREADID threadid)
{
    BeforePthread_mutex_unlock(mutex,threadid);
    PIN_GetLock(&lock, threadid+1);
    VecWait[threadid]=((RecordCond) {7, threadid, cond, mutex});
    PIN_ReleaseLock(&lock);
}

VOID AfterPthread_cond_timedwait(THREADID threadid)
{
    AfterPthread_cond_wait(threadid);
}

VOID BeforePthread_cond_signal(ADDRINT cond, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    RecordType GetOne;
    GetOne=(RecordType) {8, threadid, cond};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }
    
    VectorDetect(GetOne.threadID,ITforAllThread->second);

    CreateNewFrame(threadid,ITforAllThread->second,"CondSignal");

    SignalVecTime.insert(pair<ADDRINT, map<UINT32, long> >(GetOne.object, ((ITforAllThread->second).ListAddress)->VecTime));

    PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_broadcast(ADDRINT cond, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);

    RecordType GetOne;
    GetOne=(RecordType) {9, threadid, cond};
    
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }
    
    VectorDetect(GetOne.threadID,ITforAllThread->second);

    CreateNewFrame(threadid,ITforAllThread->second,"CondBroadcast");

    SignalVecTime[GetOne.object]=((ITforAllThread->second).ListAddress)->VecTime;

    PIN_ReleaseLock(&lock);
}

/***************************************
1. store barrier to ThreadBarrier
2. find threadid in Root
3. update VC for each barrier
***************************************/
VOID BeforePthread_barrier_wait(ADDRINT barrier, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    ThreadBarrier[threadid]=barrier;

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(threadid);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }

    map<UINT32, long> NewVC=((ITforAllThread->second).ListAddress)->VecTime;
    map<UINT32, long>::iterator mapvectime; //to find the thread's vec time
    mapvectime=NewVC.find(threadid);
    if(mapvectime==NewVC.end())
    {
        exit(-1);
    }
    mapvectime->second+=1;

    map<ADDRINT, map<THREADID, long> >::iterator findbarriervc;
    findbarriervc=BarrierVC.find(barrier);
    if(findbarriervc==BarrierVC.end())
    {
        BarrierVC[barrier]=NewVC;
    }
    else
    {
        UpdateVC(findbarriervc->second, NewVC);
    }
    PIN_ReleaseLock(&lock);
}

/****************************************
1. find barrier fron BarrierVC and output to memory file
2. find threadid from Root
3. create a new frame
4. update VC
****************************************/
VOID AfterPthread_barrier_wait(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    map<THREADID, ADDRINT>::iterator findbarrier;
    findbarrier=ThreadBarrier.find(threadid);
    if(findbarrier==ThreadBarrier.end())
    {
        exit(-1);
    }
    RecordType GetOne;
    GetOne=(RecordType) {14, threadid, findbarrier->second};
    RecordEvents(GetOne.style,GetOne.threadID,GetOne.object);

    map<UINT32, ThreadVecTime>::iterator ITforAllThread;
    ITforAllThread=AllThread.find(GetOne.threadID);
    if(AllThread.end()==ITforAllThread)
    {
        PIN_ReleaseLock(&lock);
        return;
    }

    CreateNewFrame(threadid,ITforAllThread->second,"BarrierWait");

    map<ADDRINT, map<THREADID, long> >::iterator findbarriervc;
    findbarriervc=BarrierVC.find(findbarrier->second);
    if(findbarriervc==BarrierVC.end())
    {
        exit(-1);
    }
    UpdateVC(((ITforAllThread->second).ListAddress)->VecTime, findbarriervc->second);

    PIN_ReleaseLock(&lock);
}
