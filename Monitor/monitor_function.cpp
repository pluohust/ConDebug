#include "monitor_function.h"
#include "VectorClocks.h"

extern ofstream Imgfile;

VOID ImageLoad(IMG img, VOID *v)
{
	RTN rtnpthread_create = RTN_FindByName(img, "pthread_create");
	if ( RTN_Valid( rtnpthread_create ))
	{
		RTN_Open(rtnpthread_create);
		RTN_InsertCall(rtnpthread_create, IPOINT_BEFORE, AFUNPTR(BeforePthread_create), IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_create);
	}
	RTN rtnpthread_join = RTN_FindByName(img, "pthread_join");
	if ( RTN_Valid( rtnpthread_join ))
	{
		RTN_Open(rtnpthread_join);
		RTN_InsertCall(rtnpthread_join, IPOINT_AFTER, (AFUNPTR)AfterPthread_join, IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_join);
	}
	
	RTN rtnpthread_mutex_lock = RTN_FindByName(img, "pthread_mutex_lock");
	if ( RTN_Valid( rtnpthread_mutex_lock ))
	{
		RTN_Open(rtnpthread_mutex_lock);
		RTN_InsertCall(rtnpthread_mutex_lock, IPOINT_BEFORE, AFUNPTR(BeforePthread_mutex_lock), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_THREAD_ID,IARG_RETURN_IP, IARG_END);
		RTN_Close(rtnpthread_mutex_lock);
	}
	
	RTN rtnpthread_mutex_trylock = RTN_FindByName(img, "pthread_mutex_trylock");
	if ( RTN_Valid( rtnpthread_mutex_trylock ))
	{
		RTN_Open(rtnpthread_mutex_trylock);
		RTN_InsertCall(rtnpthread_mutex_trylock, IPOINT_BEFORE, AFUNPTR(BeforePthread_mutex_trylock), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_THREAD_ID, IARG_RETURN_IP, IARG_END);
		RTN_Close(rtnpthread_mutex_trylock);
	}
	
	RTN rtnpthread_mutex_unlock = RTN_FindByName(img, "pthread_mutex_unlock");
	if ( RTN_Valid( rtnpthread_mutex_unlock ))
	{
		RTN_Open(rtnpthread_mutex_unlock);
		RTN_InsertCall(rtnpthread_mutex_unlock, IPOINT_BEFORE, AFUNPTR(BeforePthread_mutex_unlock), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_mutex_unlock);
	}
	
	RTN rtnpthread_cond_signal = RTN_FindByName(img, "pthread_cond_signal");
	if ( RTN_Valid( rtnpthread_cond_signal ))
	{
		RTN_Open(rtnpthread_cond_signal);
		RTN_InsertCall(rtnpthread_cond_signal, IPOINT_BEFORE, AFUNPTR(BeforePthread_cond_signal), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_cond_signal);
	}
	
	RTN rtnpthread_cond_broadcast = RTN_FindByName(img, "pthread_cond_broadcast");
	if ( RTN_Valid( rtnpthread_cond_broadcast ))
	{
		RTN_Open(rtnpthread_cond_broadcast);
		RTN_InsertCall(rtnpthread_cond_broadcast, IPOINT_BEFORE, AFUNPTR(BeforePthread_cond_broadcast), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_cond_broadcast);
	}
	
	RTN rtnpthread_cond_wait = RTN_FindByName(img, "pthread_cond_wait");
	if ( RTN_Valid( rtnpthread_cond_wait ))
	{
		RTN_Open(rtnpthread_cond_wait);
		RTN_InsertCall(rtnpthread_cond_wait, IPOINT_BEFORE, AFUNPTR(BeforePthread_cond_wait), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_cond_wait);
	}
	
	RTN rtnpthread_cond_timedwait = RTN_FindByName(img, "pthread_cond_timedwait");
	if ( RTN_Valid( rtnpthread_cond_timedwait ))
	{
		RTN_Open(rtnpthread_cond_timedwait);
		RTN_InsertCall(rtnpthread_cond_timedwait, IPOINT_BEFORE, AFUNPTR(BeforePthread_cond_timedwait), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_THREAD_ID, IARG_END);
		RTN_Close(rtnpthread_cond_timedwait);
	}
	
	RTN rtnpthread_barrier_wait = RTN_FindByName(img, "pthread_barrier_wait");
    if(RTN_Valid(rtnpthread_barrier_wait))
    {
        RTN_Open(rtnpthread_barrier_wait);
        RTN_InsertCall(rtnpthread_barrier_wait, IPOINT_BEFORE, AFUNPTR(BeforePthread_barrier_wait), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_THREAD_ID, IARG_END);
        RTN_Close(rtnpthread_barrier_wait);
    }
}
