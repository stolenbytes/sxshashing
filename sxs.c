#include        "defs.h"

static void    tolowerA(__in LPSTR szName){
        while (*szName != 0){
                *szName = (char)tolower(*szName);
                szName++;        
        }               
}

static void    unicode2ansi(__in WCHAR *wsString, __in LPSTR szString){
        do{
                *szString = (char)*wsString;        
                szString++;
                wsString++;
        }while (*wsString != 0);     
        *szString = 0;   
}

static void    ansi2unicode(__in LPSTR szString, WCHAR *wsString){
        do{
                *wsString = *szString;        
                szString++;
                wsString++;
        }while (*szString != 0);  
        *wsString=0;      
}

static BOOL    sxsParseAssemblyIdentity(__in IXmlReader *pReader, __in SXSHANDLE *phandle){
        HRESULT hr;
        XmlNodeType nodeType;
        BOOL    b_ret = FALSE;
        WCHAR   *wsLocalName;
        WCHAR   *wsVar;
        ULONG   index;
        PIMAGE_DOS_HEADER       pmz;
        PPEHEADER32             pe32;
        
        //parse attributes and store them... using nontefficient method : MoveToAttributeByName
        for (;;){
                hr = pReader->lpVtbl->Read(pReader, &nodeType);                    
                if (hr != S_OK) goto __Exit0;
                if (nodeType == XmlNodeType_EndElement) break;
                        
                if (nodeType != XmlNodeType_Element) continue;
                
                pReader->lpVtbl->GetLocalName(pReader, &wsLocalName, NULL);       
                if (_wcsicmp(wsLocalName, L"assemblyIdentity")) continue;
        
                for (index = 0; index < 6; index++){
                        hr = pReader->lpVtbl->MoveToAttributeByName(pReader, phandle->dependency[index].wsRealName, NULL); 
                        if (hr != S_OK) goto __Exit0;               
                        hr = pReader->lpVtbl->GetValue(pReader, &wsVar, NULL);
                        if (hr != S_OK) goto __Exit0;
                        unicode2ansi(wsVar, phandle->dependency[index].szValue);
                        tolowerA(phandle->dependency[index].szValue);
                        
                        if (phandle->dependency[index].szValue[0] == '*' && !_wcsicmp(phandle->dependency[index].wsRealName, L"language")){
                                memset(phandle->dependency[index].szValue, 0, MAX_PATH);
                                strncpy(phandle->dependency[index].szValue, "none", 4);
                        }
                
                        if (phandle->dependency[index].szValue[0] == '*' && !_wcsicmp(phandle->dependency[index].wsRealName, L"processorArchitecture")){
                                pmz = (PIMAGE_DOS_HEADER)(phandle->hModule);
                                pe32= (PPEHEADER32)((ULONG_PTR)phandle->hModule + pmz->e_lfanew);
                                memset(phandle->dependency[index].szValue, 0, MAX_PATH);
                                if (pe32->pe_machine == IMAGE_FILE_MACHINE_AMD64){
                                        strncpy(phandle->dependency[index].szValue, "amd64", 5);        
                                }else if (pe32->pe_machine == IMAGE_FILE_MACHINE_I386){
                                        strncpy(phandle->dependency[index].szValue, "x86", 3);        
                                }
                        }
                }
                
                b_ret = TRUE;
                break;
        }        
__Exit0:
        return b_ret;        
        
}

static BOOL    sxsParseManifest(__in CHAR *lpszXmlData, __in SXSHANDLE *phandle){
        IStream *pStream = NULL;
        IXmlReader *pReader = NULL;
        XmlNodeType nodeType;
        HGLOBAL hMem;
        PVOID   lpMemory;
        HRESULT hr;
        WCHAR   *wsLocalName;
        BOOL    b_ret = FALSE;
        
        CoInitialize(NULL);
        
        hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(lpszXmlData));
        lpMemory = GlobalLock(hMem);
        
        memcpy(lpMemory, lpszXmlData, strlen(lpszXmlData));
        GlobalUnlock(lpMemory);
        
        hr = CreateStreamOnHGlobal(hMem, FALSE, &pStream);
        if (hr != S_OK) goto __Exit0;
                
        hr = CreateXmlReader(&IID_IXmlReader, &pReader, NULL);
        if (hr != S_OK) goto __Exit0;
                
        hr = pReader->lpVtbl->SetInput(pReader, (IUnknown *)pStream);
        if (hr != S_OK) goto __Exit0;
        
        for (;;){                      
                hr = pReader->lpVtbl->Read(pReader, &nodeType);
                if (hr != S_OK) goto __Exit0;
                
                if (nodeType == XmlNodeType_Element){
                        pReader->lpVtbl->GetLocalName(pReader, &wsLocalName, NULL);
                        if (!_wcsicmp(wsLocalName, L"dependentAssembly")){
                                if (sxsParseAssemblyIdentity(pReader, phandle)){
                                        b_ret = TRUE;
                                        goto __Exit0;
                                }
                        }           
                        
                }
        }
                
__Exit0:
        SAFE_RELEASE(pReader);
        SAFE_RELEASE(pStream);
        GlobalFree(hMem);
        CoUninitialize();    
        return b_ret;
}

static ULONG   hash_char(ULONG hash, unsigned char val){
        return hash * 0x1003F + val;        
}

static ULONGLONG hash_string(__in LPSTR  str){
        ULONG   hash[4];
        ULONG   index;
        ULONGLONG       final;
        memset(&hash, 0, sizeof(hash));
        
        for (index = 0; index < strlen(str); index++){
                hash[index % 4] = hash_char(hash[index % 4], str[index]);                
                
        }
        
        final = hash[0] * 0x1E5FFFFFD27 + hash[1] * 0xFFFFFFDC00000051 + hash[2] * 0x1FFFFFFF7 + hash[3];
        return final;
}

PVOID   sxsInit(){
        PSXSHANDLE      phandle;
        ULONG           index;
        WCHAR           *wsPtr;
        CHAR            *szPtr;
        
        phandle = GlobalAlloc(GPTR, sizeof(SXSHANDLE));
        
        for (index = 0; index < 6; index++){
                switch (index)
                {
                case    INDEX_NAME:
                        wsPtr = L"name";
                        szPtr = "name";
                        break;
                case    INDEX_LANGUAGE:
                        wsPtr = L"language";
                        szPtr = "culture";
                        break;
                case    INDEX_TYPE:
                        wsPtr = L"type";
                        szPtr = "type";
                        break;
                case    INDEX_VERSION:
                        wsPtr = L"version";
                        szPtr = "version";
                        break;
                case    INDEX_PUBLICKEYTOKEN:
                        wsPtr = L"publicKeyToken";
                        szPtr = "publickeytoken";
                        break;
                case    INDEX_PROCESSORARCHITECTURE:
                        wsPtr = L"processorArchitecture";
                        szPtr = "processorarchitecture";
                        break;
                }
                phandle->dependency[index].wsRealName = wsPtr;
                phandle->dependency[index].szAttribute= szPtr;
                phandle->dependency[index].szValue    = GlobalAlloc(GPTR, MAX_PATH);    
        }         
        phandle->dwError = ERROR_SXS_SUCCESS;
        return phandle;        
}

VOID    sxsFree(__in PVOID hndl){
        ULONG   index;
        PSXSHANDLE phandle = hndl;
        if (hndl == NULL) return;
    
        if (phandle->hModule)
                FreeLibrary(phandle->hModule);
        
        for (index = 0; index < 6; index++){
                if (phandle->dependency[index].szValue)
                        GlobalFree(phandle->dependency[index].szValue);        
        }
        GlobalFree(phandle);
}

BOOL    sxsInitLibrary(__in PVOID hndl, __in WCHAR *wsLibrary){
        PVOID   lpManifest, lpManifestTemp;
        DWORD   dwManifestSize;
        PSXSHANDLE phandle = hndl;
        if (hndl == NULL) return FALSE;        
        
        phandle->hModule = LoadLibraryEx(wsLibrary, 0, DONT_RESOLVE_DLL_REFERENCES);
        if (!phandle->hModule){
                phandle->dwError = ERROR_SXS_LOAD_DLL;
                return FALSE;
        }
        lpManifest = LockResource(LoadResource(phandle->hModule, FindResource(phandle->hModule, MAKEINTRESOURCE(1), MAKEINTRESOURCE(RT_MANIFEST))));
        dwManifestSize = SizeofResource(phandle->hModule, FindResource(phandle->hModule, MAKEINTRESOURCE(1), MAKEINTRESOURCE(RT_MANIFEST)));        
        if (!lpManifest){
                phandle->dwError = ERROR_SXS_NO_MANIFEST;
                return FALSE;
        }
                
        lpManifestTemp = GlobalAlloc(GPTR, dwManifestSize+2);
        memcpy(lpManifestTemp, lpManifest, dwManifestSize);
        
        if (!sxsParseManifest(lpManifestTemp, phandle)){
                GlobalFree(lpManifestTemp);
                return FALSE;
        }
        
        //do hashing right away, note that real hash is not good as version is wrong
        //we need to get version from Winners key...
        sxsHash(phandle);
        sxsHashNoVersion(phandle);
        
        GlobalFree(lpManifestTemp);
        
        phandle->dwError = ERROR_SXS_SUCCESS;
        return TRUE;
}

ULONGLONG       sxsHashNoVersion(__in PVOID hndl){
        PSXSHANDLE      phandle;
        ULONGLONG       hash  = 0;
        ULONGLONG       hash_attr;
        ULONGLONG       hash_val;
        ULONGLONG       both_hashes;
        ULONG           index;
        
        phandle = hndl;
        
        if (hndl == NULL) return 0;

        for (index = 0; index < 6; index++){
                if (!strcmp(phandle->dependency[index].szValue, "none")) continue;
                if (!strcmp(phandle->dependency[index].szAttribute, "version")) continue;
                
                hash_attr = hash_string(phandle->dependency[index].szAttribute);
                hash_val  = hash_string(phandle->dependency[index].szValue);            
                both_hashes = hash_val + 0x1FFFFFFF7 * hash_attr;
                hash      =  both_hashes + 0x1FFFFFFF7 * hash;
                        
        }                
        phandle->versionhash = hash;
        return hash;        
}     

ULONGLONG       sxsHash(__in PVOID hndl){
        PSXSHANDLE      phandle;
        ULONGLONG       hash  = 0;
        ULONGLONG       hash_attr;
        ULONGLONG       hash_val;
        ULONGLONG       both_hashes;
        ULONG           index;
        
        phandle = hndl;
        
        if (hndl == NULL) return 0;

        for (index = 0; index < 6; index++){
                if (!strcmp(phandle->dependency[index].szValue, "none")) continue;
                
                hash_attr = hash_string(phandle->dependency[index].szAttribute);
                hash_val  = hash_string(phandle->dependency[index].szValue);            
                both_hashes = hash_val + 0x1FFFFFFF7 * hash_attr;
                hash      =  both_hashes + 0x1FFFFFFF7 * hash;
                        
        }                
        phandle->hash = hash;
        return hash;        
} 


BOOL    sxsGetWinnersKey(__in PVOID hndl, __in WCHAR *wsWinnersKey){
        PSXSHANDLE       phandle = hndl;
        CHAR             szWinnersKey[MAX_PATH];
        CHAR             szhash[MAX_PATH];
        
        if (hndl == NULL) return FALSE;        
        
        StringCchPrintfA(szhash, MAX_PATH, "%.016llx", phandle->versionhash);
        
        memset(szWinnersKey, 0, sizeof(szWinnersKey));
        
        StringCchCatA(szWinnersKey, MAX_PATH, phandle->dependency[INDEX_PROCESSORARCHITECTURE].szValue);
        StringCchCatA(szWinnersKey, MAX_PATH, "_");
        StringCchCatA(szWinnersKey, MAX_PATH, phandle->dependency[INDEX_NAME].szValue);
        StringCchCatA(szWinnersKey, MAX_PATH, "_");
        StringCchCatA(szWinnersKey, MAX_PATH, phandle->dependency[INDEX_PUBLICKEYTOKEN].szValue);
        StringCchCatA(szWinnersKey, MAX_PATH, "_");
        StringCchCatA(szWinnersKey, MAX_PATH, phandle->dependency[INDEX_LANGUAGE].szValue);
        StringCchCatA(szWinnersKey, MAX_PATH, "_");
        StringCchCatA(szWinnersKey, MAX_PATH, szhash);
        
        ansi2unicode(szWinnersKey, wsWinnersKey);
        return TRUE;
}

BOOL    sxsGetDllFolder(__in PVOID hndl, __in WCHAR *wsPath){
        PSXSHANDLE       phandle = hndl;
        CHAR             szPath[MAX_PATH];
        CHAR             szhash[MAX_PATH];
        
        if (hndl == NULL) return FALSE;        
        
        StringCchPrintfA(szhash, MAX_PATH, "%.016llx", phandle->hash);
        
        memset(szPath, 0, sizeof(szPath));
        StringCchCatA(szPath, MAX_PATH, phandle->dependency[INDEX_PROCESSORARCHITECTURE].szValue);
        StringCchCatA(szPath, MAX_PATH, "_");
        StringCchCatA(szPath, MAX_PATH, phandle->dependency[INDEX_NAME].szValue);
        StringCchCatA(szPath, MAX_PATH, "_");
        StringCchCatA(szPath, MAX_PATH, phandle->dependency[INDEX_PUBLICKEYTOKEN].szValue);
        StringCchCatA(szPath, MAX_PATH, "_");
        StringCchCatA(szPath, MAX_PATH, phandle->dependency[INDEX_VERSION].szValue);
        StringCchCatA(szPath, MAX_PATH, "_");
        StringCchCatA(szPath, MAX_PATH, phandle->dependency[INDEX_LANGUAGE].szValue);
        StringCchCatA(szPath, MAX_PATH, "_");
        StringCchCatA(szPath, MAX_PATH, szhash);
        
        ansi2unicode(szPath, wsPath);
        
        return TRUE;
}

BOOL    sxsUpdateVersion(__in PVOID hndl, __in WCHAR *wsVersion){
        PSXSHANDLE       phandle;
        
        phandle = hndl;
        if (hndl == NULL) return FALSE;        
        
        
        unicode2ansi(wsVersion, phandle->dependency[INDEX_VERSION].szValue);
        sxsHash(phandle);
        return TRUE;
}

BOOL    sxsGetVersion(__in PVOID hndl, WCHAR *wsVersion){
        PSXSHANDLE       phandle;
        
        phandle = hndl;
        if (hndl == NULL) return FALSE; 
                
        ansi2unicode(phandle->dependency[INDEX_VERSION].szValue, wsVersion);
        
        return TRUE;
        
}

DWORD   sxsGetError(__in PVOID hndl){
        PSXSHANDLE      phandle = hndl;
        
        if (hndl == 0) return ERROR_INVALID_PARAMETER;
        return phandle->dwError;
}
        
        
        
 