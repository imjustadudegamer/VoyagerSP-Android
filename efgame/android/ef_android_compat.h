/*
 * ef_android_compat.h — Win32/MSVC compatibility shim for building the
 * Elite Force single-player game module (Raven STEF SP source) on Android/Linux.
 *
 * Force-included by CMake (-include) so it applies to every TU. Maps the handful
 * of Win32 types, MSVC string intrinsics, and the ICARUS Tokenizer's Win32 file
 * API onto portable C. Keeps the Raven .cpp/.h sources byte-identical.
 *
 * Milestone-0/1 of the SP Android port. See SP_PORT_M0.md.
 */
#ifndef EF_ANDROID_COMPAT_H
#define EF_ANDROID_COMPAT_H
#if !defined(_WIN32)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>

/* ---- Win32 scalar types ---- */
typedef const char*   LPCTSTR;
typedef const char*   LPCSTR;
typedef char*         LPTSTR;
typedef char*         LPSTR;
typedef char          TCHAR;
typedef unsigned long DWORD;
typedef unsigned int  UINT;
typedef int           BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef long          LONG;
typedef unsigned long ULONG;
typedef void*         LPVOID;
typedef void*         HANDLE;
typedef void*         HWND;
typedef void*         HINSTANCE;
typedef unsigned long COLORREF;

#define RGB(r,g,b) ((COLORREF)(((BYTE)(r))|((WORD)((BYTE)(g))<<8)|(((DWORD)(BYTE)(b))<<16)))

/* ---- MSVC calling-convention keywords (no-ops on ARM) ---- */
#define __cdecl
#define __stdcall
#define __fastcall
#define WINAPI
#define PASCAL
#define conio_h_stub /* see android/conio.h */

typedef struct EF_SECURITY_ATTRIBUTES {
	DWORD  nLength;
	LPVOID lpSecurityDescriptor;
	BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES;

/* ---- MSVC string intrinsics ---- */
#define _stricmp   strcasecmp
#define stricmp    strcasecmp
#define _strcmpi   strcasecmp
#define strcmpi    strcasecmp
#define _strnicmp  strncasecmp
#define _strdup    strdup

static inline char* ef_strlwr(char* s){ for(char* p=s; p&&*p; ++p) *p=(char)tolower((unsigned char)*p); return s; }
static inline char* ef_strupr(char* s){ for(char* p=s; p&&*p; ++p) *p=(char)toupper((unsigned char)*p); return s; }
#define _strlwr ef_strlwr
#define _strupr ef_strupr

/* ---- EF's float random() collides with Android stdlib long random();
 *      rename the EF inline (and its bareword call sites) transparently. ---- */
#define random ef_frandom

/* ---- EF assumes Windows rand() (0..0x7fff); Android rand() is 0..2^31-1, which
 *      breaks Q_irand/crandom/ef_frandom (they divide/shift by 0x7fff). Mask to 15 bits.
 *      (rand) avoids re-expanding the function-like macro -> calls the real libc rand. ---- */
static inline int ef_rand15(void){ return (rand)() & 0x7fff; }
#define rand() ef_rand15()

/* ---- misc Win32 calls ---- */
#define OutputDebugString(s) ((void)(s))
static inline DWORD timeGetTime(void){ return (DWORD)(clock()*1000/CLOCKS_PER_SEC); }

/* ---- Win32 file API used only by ICARUS CParseFile (Tokenizer.cpp).
 *      HANDLE carries a FILE*; map to stdio. ---- */
#define INVALID_HANDLE_VALUE  ((HANDLE)-1)
#define GENERIC_READ          0x80000000UL
#define GENERIC_WRITE         0x40000000UL
#define FILE_SHARE_READ       0x00000001UL
#define FILE_SHARE_WRITE      0x00000002UL
#define OPEN_EXISTING         3
#define CREATE_ALWAYS         2
#define FILE_ATTRIBUTE_NORMAL 0x80UL
#define FILE_BEGIN            0
#define FILE_CURRENT          1
#define FILE_END             2

static inline HANDLE ef_CreateFile(LPCTSTR name, DWORD access, DWORD share,
                                   void* sa, DWORD create, DWORD attr, void* tmpl){
	(void)access;(void)share;(void)sa;(void)attr;(void)tmpl;
	FILE* f = fopen(name, (create==CREATE_ALWAYS) ? "wb" : "rb");
	return f ? (HANDLE)f : INVALID_HANDLE_VALUE;
}
static inline BOOL ef_ReadFile(HANDLE h, void* buf, DWORD n, DWORD* read, void* ovl){
	(void)ovl; size_t r = fread(buf, 1, n, (FILE*)h); if(read) *read=(DWORD)r; return 1;
}
static inline DWORD ef_SetFilePointer(HANDLE h, long dist, void* hi, DWORD method){
	(void)hi; int whence = (method==FILE_CURRENT)?SEEK_CUR : (method==FILE_END)?SEEK_END : SEEK_SET;
	fseek((FILE*)h, dist, whence); return (DWORD)ftell((FILE*)h);
}
static inline BOOL ef_CloseHandle(HANDLE h){ if(h && h!=INVALID_HANDLE_VALUE) fclose((FILE*)h); return 1; }
#define CreateFile     ef_CreateFile
#define ReadFile       ef_ReadFile
#define SetFilePointer ef_SetFilePointer
#define CloseHandle    ef_CloseHandle

#endif /* !_WIN32 */
#endif /* EF_ANDROID_COMPAT_H */
