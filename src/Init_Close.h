#include "main_basictype.h"

#ifndef HUST_INIT_CLOSE_CGCL
#define HUST_INIT_CLOSE_CGCL

void InitFileOutput();
void CloseFileOutput();
int Usage();
VOID Fini(INT32 code, VOID *v);
void RecordEvents(int Type, UINT32 threadID, ADDRINT ObjectAddress);

#endif
