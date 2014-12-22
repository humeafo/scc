/*
This Header should support code compiled for _UNICODE, but in practice, _UNICODE should be avoided
This Header contains pure codes to hide details from client file and make client cleaner, should be
included only once.

this header is tested under win7 x86/x64 version and can be compiled using msvc/clang/mingw-gcc

note: some codes are copied from TK's shell-code template with bug fixes for winx64.
by hume.
*/

#ifndef __WSHLCHDR_H__
#define __WSHLCHDR_H__ 1

#ifndef _WIN32
  #error "this header is intended for Windows, consult author for other platforms"
#endif

#include <intrin.h>
#include <tchar.h>
#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>

#if defined(__clang__) && defined(_WIN64)
  #undef NTAPI
  #define NTAPI
  #undef WINAPI
  #define WINAPI
  #undef CALLBACK
  #define CALLBACK
#endif

#ifndef NO_WINTERN_H
#include "winternl.h" /* this already included in SDK, but other versions may not be enough*/
#endif

#define SCC_SEGNAME "_sc_main"

#if !defined(__clang__)
  #if defined(_MSC_VER) && !defined(__cplusplus)
  #define inline _inline
  #endif

  #if defined(_MSC_VER)
  #pragma section(SCC_SEGNAME, execute)
  #define SCC __declspec(allocate(SCC_SEGNAME))
  #define __attribute__(x)
  #else
  #define SCC
  #endif
#else
  #define SCC
#endif


#pragma comment(lib, "psapi.lib")

#ifdef __cplusplus
extern "C" {
#endif

/*
  imp_array_offset is an array that the sc-code api calls related data:
  [{API-address offset, API-name offset}] and terminated {0, 0}

  offsets are all relative to imp_array_offset, so imp_array_offset+offset = Pointer to real thing
  howto use the extra bytes to fix external symbols is up to you, following is a traditional
  fix scheme.

  scmain and .* *scmain(.*) functions will be marked auto in _sc_main section
*/

/*
make testing under other compiler possible  
*/

#define MAX_SCMAIN_ARGS_COUNT 16
typedef void* (*pscmain)(void*, ...);

pscmain scentry_ptr = 0;
void* scentry_args[MAX_SCMAIN_ARGS_COUNT];

void* call_scentry() {
  return scentry_ptr(scentry_args[0], scentry_args[1], scentry_args[2], scentry_args[3], scentry_args[4], scentry_args[5], scentry_args[6], scentry_args[7]);
}

/*we store -1 to because some codes reject \x00 in most cases*/
#if (((defined(__GNUC__) || defined(_MSC_VER))) && !defined(__clang__)) || defined(GENCODE_NORMAL)
SCC uintptr_t _imp_offset_array[100];
#else
  extern uintptr_t _imp_offset_array[100];
#endif

#ifdef _WIN64
  #define __FAST_CALL
#else
  #define __FAST_CALL __fastcall
#endif

#ifdef _WIN64
  #define SHC_FILENAME_EXT ".b64"
#else
  #define SHC_FILENAME_EXT ".b32"
#endif
/****************************************************************************/
#define MAX_PROC_MODULES 1024

static size_t hModNum = 0;
static HMODULE hMods[MAX_PROC_MODULES];

static size_t nameNum = 0;
static char* modNames[MAX_PROC_MODULES];
static unsigned char* allocBases[MAX_PROC_MODULES];

enum {
  ofmt_none,
  ofmt_binary,
  ofmt_carray,
  ofmt_cstring,
  ofmt_jsstring,
  ofmt_jsstring2,
  ofmt_last,
};

enum {
  test_none,
  test_fun,
  test_gen,
  test_dbg
};

enum {
  impFix_none,
  impFix_hash,
  impFix_name,
};

static int oFmt = ofmt_none;
static int testMode = test_none;
static int impFixMeth = impFix_name;
/****************************************************************************/

typedef PIMAGE_NT_HEADERS(NTAPI *pRtlImageNtHeader)(PVOID Base);

PIMAGE_SECTION_HEADER getSectionTable(IN PIMAGE_NT_HEADERS NtHeaders, IN PVOID ModBase, IN ULONG_PTR Address) {
  ULONG i;
  PIMAGE_SECTION_HEADER NtSection;

  Address -= (ULONG_PTR)ModBase;
  NtSection = IMAGE_FIRST_SECTION(NtHeaders);
  for (i = 0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
    if ((ULONG)Address >= NtSection->VirtualAddress && (ULONG)Address < NtSection->VirtualAddress + NtSection->SizeOfRawData) {
      return NtSection;
    }
    ++NtSection;
  }
  return NULL;
}

BOOL getSectionAddrSize(void* CodeAddr, void** oSecAddr, unsigned* oSecSize) {
  BOOL r = FALSE;

  PIMAGE_NT_HEADERS nthdr = 0;
  PIMAGE_SECTION_HEADER psec = 0;
  static PVOID cModBase = 0;
  static HMODULE hNtdll = 0;
  static pRtlImageNtHeader RtlImageNtHeader = 0;

  if (!cModBase) {
    cModBase = (PVOID)GetModuleHandle(NULL);
  }

  if (!hNtdll) {
    hNtdll = LoadLibrary(_TEXT("ntdll"));
  }

  if (!RtlImageNtHeader) {
    RtlImageNtHeader = (pRtlImageNtHeader)GetProcAddress(hNtdll, "RtlImageNtHeader");
  }

  if (RtlImageNtHeader) {
    nthdr = RtlImageNtHeader(cModBase);
    psec = getSectionTable(nthdr, cModBase, (ULONG_PTR)CodeAddr);
    if (psec) {
      if (oSecAddr) {
        *oSecAddr = (void*)((char*)cModBase + psec->VirtualAddress);
      }
      if (oSecSize) {
        *oSecSize = psec->Misc.VirtualSize;
      }
      return TRUE;
    }
  }
  return r;
}

size_t getOrInsertModName(LPSTR namp, size_t siz) {
  size_t i = 0;
  char* p;
  for (; i < nameNum; ++i) {
    if (strcmp(namp, modNames[i]) == 0) {
      return i;
    }
  }
  p = (char*)malloc(siz+1);
  memcpy(p, namp, siz + 1);
  modNames[nameNum++] = p;
  return nameNum - 1;
}
/*
a brute and not very elegant way to resolve import name's dll module, we just brute
force, the right method is to parse PE import table to get this done elegantly, but 
however, this method should work in every scene I encountered, I'm just a bit lazy.
*/
HMODULE resolveImpModule(LPCSTR impName) {
  size_t i;
  DWORD cbNeeded;
  HMODULE hRt;

  if (hModNum == 0) {
    if (!EnumProcessModules(GetCurrentProcess(), hMods + hModNum, sizeof(hMods), &cbNeeded)) {
      return NULL;
    }
    hModNum += cbNeeded / sizeof(HMODULE);
    
    // added msvcrt for normal clib resolution
    hRt = LoadLibrary("msvcrt.dll");
    if (hRt) hMods[hModNum++] = hRt;
  }

  for (i = 0; i < hModNum; ++i) {
    if (GetProcAddress(hMods[i], impName)) {
      return hMods[i];
    }
  }
  return NULL;
}

/*
the string is mallocd, so if you want no mem leak caller should free the memory manually
we use this just to make it C source compatible if someone just like using C
*/
size_t resolveImpModuleName(LPCSTR impName) {
  size_t i, nl;
  TCHAR buf[MAX_PATH + 1];
  char nbuf[MAX_PATH];

  HMODULE hMod = resolveImpModule(impName);
  if (hMod) {
    DWORD nLen = GetModuleFileName(hMod, buf, MAX_PATH);
    if (nLen) {
      // get dll name, ensured char based
      TCHAR* btp = buf + nLen;
      TCHAR* etp = btp;
      while (*btp-- != '\\');
      btp += 2;
      nl = etp - btp;
      for (i = 0; i < nl; ++i) {
        nbuf[i] = (char)(*btp++);
      }
      nbuf[nl] = 0;
      return getOrInsertModName(nbuf, nl);
    }
  }
  return -1;
}

/*
call this if you really do care about memory leak
*/
void wshlchdrCleanup() {
  size_t i = 0;
  for (; i < nameNum; ++i) {
    if (modNames[i]) {
      free(modNames[i]);
      modNames[i] = 0;
    }
  }
}
/************************************************************************/
#define KERNEL32_MAGIC 0xFFFFFFFE

void defaultUsage() {
  printf("<> [Option]\n");
  printf("  -h show this help\n");
  printf("  -fbin output as binary file\n");
  printf("  -fcary  output as c array\n");
  printf("  -fcstr  output as c string\n");
  printf("  -fjstr  output as js %%u string\n");
  printf("  -fjstr2  output as js \\u string\n");
  printf("  -ftest test functionality\n");
  printf("  -fgtest test generated code\n");
}

BOOL parseCmdLine(int argc, char* argv[]) {
  if (argc == 1) {
    oFmt = ofmt_carray;
    testMode = test_fun;
  } else if (argc >= 2) {
    if (strcmp(argv[1], "-h") == 0) {
      defaultUsage();
      exit(0);
    } else if (strcmp(argv[1], "-fcary") == 0) {
      oFmt = ofmt_carray;
    } else if (strcmp(argv[1], "-fbin") == 0) {
      oFmt = ofmt_binary;
    } else if (strcmp(argv[1], "-fcstr") == 0) {
      oFmt = ofmt_cstring;
    } else if (strcmp(argv[1], "-fjstr") == 0) {
      oFmt = ofmt_jsstring;
    } else if (strcmp(argv[1], "-fjstr2") == 0) {
      oFmt = ofmt_jsstring2;
    } else if (strcmp(argv[1], "-ftest") == 0) {
      testMode = test_fun;
    } else if (strcmp(argv[1], "-fgtest") == 0) {
      testMode = test_gen;
    } else if (strcmp(argv[1], "-fdbg") == 0) {
      testMode = test_dbg;
    } else {
      return FALSE;
    }
  }
  return TRUE;
}

void outputData(FILE* out, const unsigned char* p, size_t siz) {
  size_t i;
  wchar_t wc = 0;
  FILE* fp;

  if (oFmt == ofmt_binary) {
    char oFn[] = "wshlc.out" SHC_FILENAME_EXT ;
    fp = fopen(oFn, "wb");
    if (fp) {
      fwrite(p, siz, 1, fp);
      fclose(fp);
      fprintf(stdout, "Writeout binary file %s complete...\n", oFn);
    }
    return;
  }

  for (i = 0; i < siz; ++i) {
    if (oFmt == ofmt_carray) {
      fprintf(out, "0x%02x, ", p[i]);
    } else if (oFmt == ofmt_cstring) {
      fprintf(out, "\\x%02x", p[i]);
    } else if (oFmt == ofmt_jsstring || oFmt == ofmt_jsstring2) {

      const char* jsprfx = oFmt == ofmt_jsstring ? "%%u" : "\\u";
      
      if (i & 1) {
        wc |= p[i] << 8;
        fprintf(out, "%s%04x", jsprfx, wc);
      } else {
        wc = p[i];
      }
    }
  }

  if (oFmt == ofmt_jsstring || oFmt == ofmt_jsstring2) {
    if (siz & 1) {
      const char* jsprfx = oFmt == ofmt_jsstring ? "%u" : "\\u";
      fprintf(out, "%s%04x", jsprfx, wc);
    }
  }
}
/************************************************************************/
#ifdef _WIN64
inline struct _TEB * GetNtCurrentTeb() { return (struct _TEB *)__readgsqword(FIELD_OFFSET(NT_TIB, Self)); }
#else
inline struct _TEB * GetNtCurrentTeb(void) { return (struct _TEB *) (ULONG_PTR)__readfsdword(0x18); }
#endif // _WIN64

/*
the second list entry is always kernel32.dll in most cases
we can also use bigger codes to ensure that as conditional compilation shows
*/
inline void* getKernel32Base(void) {
  PPEB pPEB = GetNtCurrentTeb()->ProcessEnvironmentBlock;
  PLIST_ENTRY pListHead = pPEB->Ldr->InMemoryOrderModuleList.Flink;
  PLDR_DATA_TABLE_ENTRY pLdrEnt;
  
#if 0
  pListHead = pListHead->Flink;
  pLdrEnt = CONTAINING_RECORD(pListHead->Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
  return pLdrEnt->DllBase;
#else
  DWORD tmp;

  while (pListHead) {
    pLdrEnt = CONTAINING_RECORD(pListHead->Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
    if (pLdrEnt->FullDllName.Length >= 24) {
      tmp = *(DWORD*)((char*)pLdrEnt->FullDllName.Buffer + (pLdrEnt->FullDllName.Length - 24));
      if (tmp == 0x065006b || tmp == 0x0045004b) 
        break;
    }
    pListHead = pListHead->Flink;
  }
  return pLdrEnt->DllBase;
#endif
}

inline DWORD hashKey(unsigned char *key) {
  DWORD nHash = 0;
  while (*key) {
    nHash = (nHash << 5) + nHash + *key++;
  }
  return nHash;
}

void* __FAST_CALL getProcAddressByHash(PVOID baseAddr, DWORD fHash) {
  DWORD *pNameBase;
  void* Function;
  unsigned Ordinals;
  PIMAGE_DOS_HEADER pDos;
  PIMAGE_NT_HEADERS pNtHdr;
  PIMAGE_EXPORT_DIRECTORY pExport;
  BOOL Found = FALSE;

  pDos = (PIMAGE_DOS_HEADER)baseAddr;
  pNtHdr = (PIMAGE_NT_HEADERS)((char*)baseAddr + (DWORD)pDos->e_lfanew);
  pExport = (PIMAGE_EXPORT_DIRECTORY)((char*)baseAddr + pNtHdr->OptionalHeader.DataDirectory[0].VirtualAddress);
  pNameBase = (DWORD*)((char*)baseAddr + pExport->AddressOfNames);
  for (Ordinals = 0; Ordinals < pExport->NumberOfNames; Ordinals++) {
    unsigned char *pName = (unsigned char*)baseAddr + *pNameBase;
    if (hashKey(pName) == fHash) {
      Found = TRUE;
      break;
    }
    pNameBase++;
  }
  if (Found) {
    WORD Index;
    Index = ((WORD*)((char*)baseAddr + pExport->AddressOfNameOrdinals))[Ordinals];
    Function = (void *)((char*)baseAddr + ((DWORD*)((char*)baseAddr + pExport->AddressOfFunctions))[Index]);
    return Function;
  }
  return NULL;
}
/************************************************************************/
/*
This function and impFixDefault should match, this is not compiler hard-coded,
the user should choose the scheme for import processing methods
*/
typedef HMODULE (WINAPI* pLoadLibraryA)(_In_ LPCSTR lpLibFileName);
typedef FARPROC (WINAPI *pGetProcAddress)(_In_ HMODULE hModule, _In_ LPCSTR lpProcName);

inline void impFixHashK32Only() {
  void* k32Base = getKernel32Base();
  uintptr_t* pImp = _imp_offset_array, *pHash;

  while (*pImp) {
    pHash = (uintptr_t*)((char*)_imp_offset_array + *pImp);
    *pHash = (uintptr_t)getProcAddressByHash(k32Base, (DWORD)*pHash);
    pImp += 2;
  }
}

#define HASH_LoadLibraryA                   0x071d2c76
#define HASH_GetProcAddress                 0xc2cbc15a

inline void impFixNameCommon() {
  pLoadLibraryA dlopen = 0;
  pGetProcAddress dlsym = 0;

  HMODULE hMod;
  uintptr_t modOffset;
  uintptr_t* pAddr;
  char* pName;
  void* k32Base = getKernel32Base();
  uintptr_t* pImp;

  dlopen = (pLoadLibraryA)getProcAddressByHash(k32Base, HASH_LoadLibraryA);
  dlsym = (pGetProcAddress)getProcAddressByHash(k32Base, HASH_GetProcAddress);

  pImp = _imp_offset_array;

  while (*pImp) {
    pAddr = (uintptr_t*)((char*)_imp_offset_array + *pImp);
    pName = (char*)_imp_offset_array + (uintptr_t)(*pAddr);
    modOffset = *(pImp + 1);
    if (modOffset == KERNEL32_MAGIC) {
      hMod = (HMODULE)k32Base;
    } else {
      hMod = dlopen((char*)_imp_offset_array + modOffset);
    }
    *pAddr = (uintptr_t)dlsym(hMod, pName);
    pImp += 2;
  }
}

unsigned char* appendCString(unsigned char* ecode, unsigned char* s) {
  size_t sl = strlen((char*)s) + 1;
  memcpy(ecode, s, sl);
  return ecode + sl;
}

unsigned char* scImpFixDefault(uintptr_t* pImp, unsigned char* eCode) {
  unsigned char* impName;
  uintptr_t modOffset;
  uintptr_t* impStart = pImp, *pAddr;
  size_t idx;

  while (*pImp) {
    impName = (unsigned char*)(*(pImp + 1));
    if (!impName) {
      return 0;
    }
    pAddr = (uintptr_t*)((char*)impStart + *pImp);

    if(impFixMeth == impFix_hash) {
      *pAddr = hashKey(impName);
    } else if(impFixMeth == impFix_name) {
      *pAddr = (uintptr_t)(eCode - (unsigned char*)impStart);
      eCode = appendCString(eCode, impName);
    } else {
      fprintf(stderr, "Unknown impFix method %d!\n", impFixMeth);
      return 0;
    }

    /*fix modules*/
    idx = resolveImpModuleName((LPCSTR)impName);
    if (idx == -1) {
      fprintf(stderr, "Resolve import name %s module failed!\n", impName);
      return 0;
    }

    if (impFixMeth == impFix_hash && _stricmp(modNames[idx], "kernel32.dll") == 0) {
      *(pImp + 1) = KERNEL32_MAGIC;
    } else {
      if (!allocBases[idx]) {
        allocBases[idx] = eCode;
        eCode = appendCString(eCode, (unsigned char*)modNames[idx]);
      }
      modOffset = allocBases[idx] - (unsigned char*)impStart;
      *(pImp + 1) = modOffset;
    }
    fprintf(stdout, "Process %s in module %s ok!\n", impName, modNames[idx]);
    pImp += 2;
  }

  if (*impStart) {
    fprintf(stdout, "Dump of import table fix content:\n");
    pImp = impStart;
    while (*pImp) {
      pAddr = (uintptr_t*)((char*)impStart + *pImp);
      fprintf(stdout, "%p -- %p : %p\n", (void*)(*pImp), (void*)(*(pImp + 1)), (void*)(*pAddr));
      pImp += 2;
    }
  }
  return eCode;
}

typedef unsigned char* (scImpFixPtr)(uintptr_t* pImp, unsigned char* eCode);

unsigned char* codeGenDefault(void* entryF, unsigned* osize, scImpFixPtr scImpFixer) {
  unsigned secSize;
  size_t eOffset, impOffset;
  void* secStart;
  uintptr_t* impArray;

  unsigned char* pCode, *rCode, *eCode;

  if (!getSectionAddrSize(entryF, &secStart, &secSize)) {
    fprintf(stderr, "Get function section failed!\n");
    return 0;
  }

  fprintf(stdout, "Original code section at %p : %d bytes\n", secStart, secSize);

  /* +8192 should be large enough in most cases */
  pCode = (unsigned char*)VirtualAlloc(0, secSize + 8192, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (pCode) {
    rCode = pCode;
    eOffset = (char*)entryF - (char*)secStart;
    impOffset = (char*)_imp_offset_array - (char*)secStart;

    /* make code debugging easier*/
    if (testMode == test_dbg) {
      *rCode++ = 0xcc;
    }

    if (eOffset > 0) {
      if (eOffset < 128) {
        /*emit eb xx*/
        *rCode++ = 0xeb;
        *rCode++ = (unsigned char)eOffset;
      } else {
        *rCode++ = 0xe9;
        *(DWORD*)rCode = (DWORD)eOffset;
        rCode += 4;
      }
    }

    eCode = rCode + secSize;
    impArray = (uintptr_t*)(rCode + impOffset);
    memcpy(rCode, secStart, secSize);

    if(scImpFixer == 0) {
      eCode = scImpFixDefault(impArray, eCode);
    } else {
      eCode = scImpFixer(impArray, eCode);
    }

    if (eCode) {
      if (osize) {
        *osize = (unsigned)(eCode - pCode);
      }
      return pCode;
    }
  }
  return 0;
}

int mainDefault(int argc, char* argv[], scImpFixPtr scImpFixer) {
  unsigned char* pCode;
  unsigned codeSize;
  DWORD oldProt;

  if (!parseCmdLine(argc, argv)) {
    defaultUsage();
    exit(-1);
  }

  if (testMode == test_fun) {

    if (_imp_offset_array[0] != 0) {
      
      if (!getSectionAddrSize((void*)scentry_ptr, (void**)&pCode, &codeSize)) {
        fprintf(stderr, "Get scmain section failed!\n");
        exit(-2);
      }

      VirtualProtect(pCode, codeSize, PAGE_EXECUTE_READWRITE, &oldProt);
      _imp_offset_array[0] = 0;
    }
    
    call_scentry();
    return 0;
  }

  pCode = codeGenDefault((void*)scentry_ptr, &codeSize, scImpFixer);
  if (pCode) {
    fprintf(stdout, "Code generated: %d bytes start at: %p\n", codeSize, pCode);
    fprintf(stdout, "-------------------------\n\n");
    outputData(stdout, pCode, codeSize);
  } else {
    fprintf(stderr, "Code generation failed!\n");
    exit(-2);
  }

  if (testMode == test_gen || testMode == test_dbg) {
    scentry_ptr= (pscmain)pCode;
    call_scentry();
  }
  return 0;
}
/****************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
