#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned long cmd;    // = cmd
    unsigned long data;   // = data
    unsigned long stat1;  // = 0
    unsigned long stat2;  // = 0
} SMBIOS_PKG;


#ifndef _WIN64
int dell_smm_lowlevel(SMBIOS_PKG* sc)
{
	int status = 1;
	ULONG cmd = sc->cmd;
	ULONG data = sc->data;
	ULONG stat1 = sc->stat1;
	ULONG stat2 = sc->stat2;

	_asm
	{
		pushad
		mov eax, cmd
			mov ebx, data
			mov ecx, stat1
			mov edx, stat2
			out 0b2h,al
			out 084h,al
			mov cmd, eax
			mov data, ebx
			mov stat1, ecx
			mov stat2, edx
			popad
	}

	// did it work ?
	if ((sc->cmd == cmd && sc->data == data && sc->stat1 == stat1 && sc->stat2 == stat2)  // function not supported
		|| ((cmd & 0xffff) == 0xffff))     // or invalid device
		status = 0;

	sc->cmd = cmd;
	sc->data = data;
	sc->stat1 = stat1;
	sc->stat2 = stat2;

	return status;
}
/*
VOID msr_read(PULONG pIOBuffer;)
{
	__asm
	{
		push ebx
		mov ebx, pIOBuffer
		mov ecx, dword ptr[ebx]
		rdmsr
		mov dword ptr[ebx], edx
		mov dword ptr[ebx + 4], eax
		pop ebx
	}
}
*/
#else
//#pragma comment(lib, ".\\bzh_dell_smm_lowlevel\\bzh_dell_smm_lowlevel.lib")
extern int dell_smm_lowlevel(SMBIOS_PKG* sc);
extern UINT64 msr_read(int msrNo);
#endif
#ifdef __cplusplus
}
#endif