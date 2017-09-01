#ifndef __SXS__
#define __SXS__

#define INDEX_NAME      0
#define INDEX_LANGUAGE  1
#define INDEX_TYPE      2
#define INDEX_VERSION   3
#define INDEX_PUBLICKEYTOKEN 4
#define INDEX_PROCESSORARCHITECTURE 5

typedef  struct{
        WCHAR   *wsRealName;
        CHAR    *szAttribute;
        CHAR    *szValue;
}DEPENDENCY, *PDEPENDENCY;   


typedef struct{
        HMODULE hModule;
        DWORD           dwError;
        ULONGLONG       versionhash;
        ULONGLONG       hash;
        DEPENDENCY      dependency[6];
}SXSHANDLE, *PSXSHANDLE;

#define ERROR_SXS_SUCCESS               0
#define ERROR_SXS_LOAD_DLL              1
#define ERROR_SXS_NO_MANIFEST           2
#define ERROR_SXS_INVALID_MANIFEST      3



PVOID   sxsInit();
VOID    sxsFree(__in PVOID hndl);
BOOL    sxsInitLibrary(__in PVOID hndl, __in WCHAR *wsLibrary);
BOOL    sxsGetWinnersKey(__in PVOID hndl, __in WCHAR *wsWinnersKey);
BOOL    sxsUpdateVersion(__in PVOID hndl, __in WCHAR *wsVersion);
BOOL    sxsGetVersion(__in PVOID hndl, WCHAR *wsVersion);
BOOL    sxsGetDllFolder(__in PVOID hndl, __in WCHAR *wsPath);
DWORD   sxsGetError(__in PVOID hndl);

ULONGLONG       sxsHashNoVersion(__in PVOID hndl);
ULONGLONG       sxsHash(__in PVOID hndl);
#endif