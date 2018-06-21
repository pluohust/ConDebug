#include "main_basictype.h"

#ifndef HUST_VECTORCLOCKS_CGCL
#define HUST_VECTORCLOCKS_CGCL

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v);
VOID BeforePthread_create(THREADID threadid);
VOID AfterPthread_join(THREADID threadid);
VOID BeforePthread_mutex_lock(ADDRINT currentlock, THREADID threadid, ADDRINT lockfunaddr);
VOID BeforePthread_mutex_trylock(ADDRINT currentlock, THREADID threadid, ADDRINT lockfunaddr);
VOID BeforePthread_mutex_unlock(ADDRINT currentlock, THREADID threadid);
VOID BeforePthread_cond_wait(ADDRINT cond, ADDRINT mutex, THREADID threadid);
VOID BeforePthread_cond_timedwait(ADDRINT cond, ADDRINT mutex, THREADID threadid);
VOID BeforePthread_cond_signal(ADDRINT cond, THREADID threadid);
VOID BeforePthread_cond_broadcast(ADDRINT cond, THREADID threadid);
VOID BeforePthread_barrier_wait(ADDRINT barrier, THREADID threadid);
#endif
