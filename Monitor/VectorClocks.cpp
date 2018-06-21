#include "VectorClocks.h"
#include "Init_Close.h"
#include <sstream>
extern ofstream Imgfile;
extern long LockNum, SynNum;
extern PIN_LOCK lock;
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
	PIN_GetLock(&lock, threadid+1);
Output();
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_create(THREADID threadid)
{
	PIN_GetLock(&lock, threadid+1);
SynNum++;
	PIN_ReleaseLock(&lock);
}

VOID AfterPthread_join(THREADID threadid)
{
	PIN_GetLock(&lock, threadid+1);
SynNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_mutex_lock(ADDRINT currentlock, THREADID threadid, ADDRINT lockfunaddr)
{
	PIN_GetLock(&lock, threadid+1);
LockNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_mutex_trylock(ADDRINT currentlock, THREADID threadid, ADDRINT lockfunaddr)
{
	PIN_GetLock(&lock, threadid+1);
	LockNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_mutex_unlock(ADDRINT currentlock, THREADID threadid)
{
	PIN_GetLock(&lock, threadid+1);
LockNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_wait(ADDRINT cond, ADDRINT mutex, THREADID threadid)
{
	BeforePthread_mutex_unlock(mutex,threadid);
SynNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_timedwait(ADDRINT cond, ADDRINT mutex, THREADID threadid)
{
	BeforePthread_mutex_unlock(mutex,threadid);
SynNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_signal(ADDRINT cond, THREADID threadid)
{
	PIN_GetLock(&lock, threadid+1);
SynNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_cond_broadcast(ADDRINT cond, THREADID threadid)
{
	PIN_GetLock(&lock, threadid+1);
SynNum++;
	PIN_ReleaseLock(&lock);
}

VOID BeforePthread_barrier_wait(ADDRINT barrier, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
SynNum++;
    PIN_ReleaseLock(&lock);
}
