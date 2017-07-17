//There are some types that all call use.
#include <vector>
#include <string>
#include <map>
#include <tr1/unordered_map>
#include <set>
#include <vector>
#include <queue>
#include <stdint.h>
#include <list>
#include <pthread.h>
#include <map>
#include <algorithm>
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

#ifndef HUST_BASICTYPE_CGCL
#define HUST_BASICTYPE_CGCL

#define DEST 1
#define SOURCE 2
#define LENGTH 3

using namespace std::tr1;

struct StringTuple
{
    string one;
    string two;
    
    bool operator <(const StringTuple& rhs) const
    {
        if(one<rhs.one)
            return true;
        else if(one==rhs.one)
            return two < rhs.two;
        else
            return false;
    }
    
    bool operator >(const StringTuple& rhs) const
    {
        if(one>rhs.one)
            return true;
        else if(one==rhs.one)
            return two > rhs.two;
        else
            return false;
    }
};

struct RecordType //Record the necessary information to the detector
{
    int style;
    UINT32 threadID;
    ADDRINT object;
};

struct RecordCond //Record the necessary information to the conditional variable
{
    int style;
    UINT32 threadID;
    ADDRINT cond;
    ADDRINT mutex;
};

struct ShareAddreeStruct //This is the share memory space address
{
    ADDRINT address_name;
    USIZE address_size;
    
    bool operator <(const ShareAddreeStruct& rhs) const
    {
        return address_name < rhs.address_name;
    }
    
    bool operator >(const ShareAddreeStruct& rhs) const
    {
        return address_name > rhs.address_name;
    }
};

struct MallocInf
{
    USIZE size;
    string codeinf;
};

struct ResultInf
{
    long number;
    string codeinf;
};

struct LockInf //Each share address' lock information
{
    int R;
    int W;
    std::vector<ADDRINT> lockID;
};

struct ThreadInf //Each thread information
{
    int status; //ThreadStart is 1;
    UINT32 threadID;
    UINT32 fartherthreadID;
    struct MemoryData *data;
    struct ThreadInf *next;
};

struct ThreadParent //The relationship between the farther and child
{
    bool liveflag;
    THREADID fatherthreadid;
};

struct CreateThreadInf
{
    UINT32 threadID;
    struct MemoryData *data;
};

struct RtnNameNumber //Get the function name
{
    string RtnName;
    long InsNumber;
};

struct RtnAddress //Get the last access address to get the code address
{
    string ImgName;
    ADDRINT MemoryAddress;
};

struct LockVecTime //Record the vector time of each lock in each thread
{
    map<UINT32, long> OldAcquire;
    map<UINT32, long> NewRelease;
};

struct SharedMemoryAccessInf //This is just an access but store enough information
{
    bool Rstatus, Wstatus;
    set<ADDRINT> InsR;
    set<ADDRINT> InsW;
//    long Rcount, Wcount;
//    ADDRINT the_ins_in_memory_r;
//    ADDRINT the_ins_in_memory_w;
};

struct RWVecTime
{
    //flag
    bool Writeflag;
    bool Accessflag;
    //other information
    string SynName;
    string LockCode; //the code address of pthread_lock
    map<UINT32, long> VecTime;
    set<ADDRINT> locks; // acquired lock set
    //store the related function information
    unordered_map<ADDRINT, SharedMemoryAccessInf> SharedLocation;
//    map<ADDRINT, string> CMRMemory;
//    map<ADDRINT, MallocInf> FreeMemory;
//    map<ADDRINT, string> DoubleFree;
};

struct ThreadVecTime
{
    list<struct RWVecTime>::iterator ListAddress;
    list<struct RWVecTime> VecTimeList;
};
#endif
