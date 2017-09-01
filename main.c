#include        "defs.h"

static void    tolowerW(__in WCHAR * szName){
        while (*szName != 0){
                *szName = (char)towlower(*szName);
                szName++;        
        }               
}

BOOL    DeleteFolder(__in WCHAR *wsFolderPath){
        HANDLE  hFind;
        WIN32_FIND_DATA wfd;
       
        if (!SetCurrentDirectory(wsFolderPath)) return FALSE;
        
        hFind = FindFirstFile(L"*", &wfd);
        if (hFind == INVALID_HANDLE_VALUE) goto __Exit0;
        if (hFind == INVALID_HANDLE_VALUE) goto __Exit0;
        do{
                if (wfd.cFileName[0] == '.' && wfd.cFileName[1] == 0) continue;
                if (wfd.cFileName[0] == '.' && wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0) continue;
                        
                if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                        DeleteFolder(wfd.cFileName);
                }else if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY){
                        SetFileAttributes(wfd.cFileName, wfd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                }              
                
                DeleteFile(wfd.cFileName);
        }while(FindNextFile(hFind, &wfd));
        
        if (hFind != INVALID_HANDLE_VALUE)
                FindClose(hFind);
__Exit0:
        SetCurrentDirectory(L"..");
        RemoveDirectory(wsFolderPath);
        return TRUE;        
}

int __cdecl wmain(int argc, wchar_t **argv){
        HKEY            hKey;
        HKEY            hSubKey;
        HKEY            hVersionKey;
        WCHAR           wsWinnersKey[MAX_PATH];
        WCHAR           wsWinnersSubKey[MAX_PATH];
        WCHAR           wsVersion[MAX_PATH];
        WCHAR           wsFinalVersion[MAX_PATH];
        
        PVOID           psxs;
        
        ULONG           major, minor, build, revision;
        DWORD           dwType, dwSize;
        WCHAR           wsDllPath[MAX_PATH];
        LONG            err;
        
        STARTUPINFO     sinfo;
        PROCESS_INFORMATION     pinfo;
        WCHAR           wsCurrentProcess[MAX_PATH];
        WCHAR           wsProcessFolder[MAX_PATH];
        WCHAR           wsWinSxsPath[MAX_PATH];
        WCHAR           wsWinSxsPathTmp[MAX_PATH];
        HMODULE         hComCtl;
        PLDR_DATA_TABLE_ENTRY   pLdrData;
        
        //get file path...
        GetModuleFileName(GetModuleHandle(0), wsCurrentProcess, MAX_PATH);
        
        InitCommonControls();
        
        hComCtl = GetModuleHandle(L"comctl32.dll");
        LdrFindEntryForAddress(hComCtl, &pLdrData);

        tolowerW(pLdrData->FullDllName.Buffer);
        
        if (wcsstr(pLdrData->FullDllName.Buffer, L".local")){
                MessageBox(0,L"loaded comctl32.dll from .local...", L"ok",0);
                ExitProcess(0);                        
        }
        
        
        psxs = sxsInit();
        if (!sxsInitLibrary(psxs, wsCurrentProcess)){ //L"C:\\windows\\system32\\msconfig.exe")){
                printf("[-] Failed to map library or to get need info from it...\n");
                return 1;
        } 
        if (!sxsGetWinnersKey(psxs, wsWinnersKey)){
                printf("[-] Failed to get Winners key...\n");
                return 1;
        }
        printf("[+] Winners Key        : %S\n", wsWinnersKey);
        
        err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           L"Software\\Microsoft\\Windows\\CurrentVersion\\SideBySide\\Winners",
                           0,
                           KEY_READ | KEY_WOW64_64KEY,
                           &hKey);
        if (err != ERROR_SUCCESS){
                printf("[X] Failed to open Winners key...\n");
                return 1;
        }
        err = RegOpenKeyEx(hKey,
                           wsWinnersKey,
                           0,
                           KEY_READ | KEY_WOW64_64KEY,
                           &hSubKey);
        if (err != ERROR_SUCCESS){
                printf("[X] Failed to open %S key\n", wsWinnersKey);
                return 1;
        }
        
        sxsGetVersion(psxs, wsVersion);
        swscanf(wsVersion, L"%u.%u.%u.%u", &major, &minor, &build, &revision);
        StringCchPrintf(wsWinnersSubKey, MAX_PATH, L"%u.%u", major, minor);
        
        
        printf("[+] Trying version key : %S\n", wsWinnersSubKey); 
        
        memset(wsFinalVersion, 0, sizeof(wsFinalVersion));
        err = RegOpenKeyEx(hSubKey,
                           wsWinnersSubKey,
                           0,
                           KEY_READ | KEY_WOW64_64KEY,
                           &hVersionKey);
        if (err != ERROR_SUCCESS){
                printf("[-] Failed to open version Winners key : %S\\%S", wsWinnersKey, wsWinnersSubKey);
                return 1;
        }
        
        
        dwType = REG_SZ;
        dwSize = MAX_PATH;
        err = RegQueryValueEx(hVersionKey,
                              NULL,
                              0,
                              &dwType,
                              (BYTE *)wsFinalVersion,
                              &dwSize);
        
        if (err != ERROR_SUCCESS){
                printf("[-] Failed to get new version...\n");
                return 1;
        }
        printf("[+] New version        : %S\n", wsFinalVersion);
        
        sxsUpdateVersion(psxs, wsFinalVersion); 
        
        sxsGetDllFolder(psxs, wsDllPath);
        printf("[+] dll path           : %S\n", wsDllPath);  
        sxsFree(psxs);       
        
        //oki now we can put this to the test...
        
        StringCchPrintf(wsProcessFolder, MAX_PATH, L"%s.local", wsCurrentProcess);
        
        memset(wsWinSxsPathTmp, 0, sizeof(wsWinSxsPathTmp));
        memset(wsWinSxsPath, 0, sizeof(wsWinSxsPath));
        
        if (!CreateDirectory(wsProcessFolder, NULL)){
                printf("[-] Failed to create : %S\n", wsProcessFolder);
                return 1;
        }
        StringCchCat(wsProcessFolder, MAX_PATH, L"\\");
        StringCchCat(wsProcessFolder, MAX_PATH, wsDllPath);
        if (!CreateDirectory(wsProcessFolder, NULL)){
                printf("[-] Failed to create : %S\n", wsProcessFolder);
                goto __Exit0;
        }        
        StringCchCat(wsProcessFolder, MAX_PATH, L"\\comctl32.dll");
        
        StringCchPrintf(wsWinSxsPathTmp, MAX_PATH, L"%%windir%%\\winsxs\\%s\\comctl32.dll", wsDllPath);
        
        ExpandEnvironmentStrings(wsWinSxsPathTmp, wsWinSxsPath, MAX_PATH);   
        if (!CopyFile(wsWinSxsPath, wsProcessFolder, TRUE)){
                printf("[-] Failed to copy comctl32.dll\n");
                goto __Exit0;
        } 
        
        memset(&sinfo, 0, sizeof(sinfo));
        memset(&pinfo, 0, sizeof(pinfo));
        
        if (!CreateProcess(0,
                      wsCurrentProcess,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      &sinfo,
                      &pinfo)){
                printf("[-] Failed to create child process\n");
                goto __Exit0;
        }
        
        WaitForSingleObject(pinfo.hProcess, INFINITE);

__Exit0:        
        StringCchPrintf(wsProcessFolder, MAX_PATH, L"%s.local", wsCurrentProcess);
        DeleteFolder(wsProcessFolder);
           
}