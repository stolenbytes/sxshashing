#define         _CRT_SECURE_NO_WARNINGS
#include        <windows.h>
#include        <strsafe.h>
#include        <Objbase.h>
#include        <Commctrl.h>
#include        <xmllite.h>
#include        "pe64.h"
#include        "sxs.h"

#define SAFE_RELEASE(x) \
        if (x){         \
                x->lpVtbl->Release(x);  \
                x = NULL;       \
        } 

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PVOID  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
        
typedef struct _LDR_DATA_TABLE_ENTRY                         
{                                                            
        LIST_ENTRY InLoadOrderLinks;                 
        LIST_ENTRY InMemoryOrderLinks;               
        LIST_ENTRY InInitializationOrderLinks;       
        PVOID        DllBase;                                
        PVOID        EntryPoint;                             
        ULONG32      SizeOfImage;                            
        UNICODE_STRING FullDllName;                  
        UNICODE_STRING BaseDllName;     
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

NTSTATUS
NTAPI
LdrFindEntryForAddress(
    IN PVOID Address,
    OUT PVOID *TableEntry //PLDR_DATA_TABLE_ENTRY *TableEntry
    );
        