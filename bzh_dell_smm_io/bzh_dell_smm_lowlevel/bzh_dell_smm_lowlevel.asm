;   lowlevel.asm - low level hardware access for x64 systems
;   
;   FanIO driver V2.3
;   Copyright(c) 2001-2007 Christian Diefer
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License version 2 as
;   published by the Free Software Foundation.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

.code

PUBLIC dell_smm_lowlevel
PUBLIC msr_read

;rcx:SMBIOS_PKG* cmd
dell_smm_lowlevel PROC
  push rcx
  push rdx
  push r8
  push r9
  push rbx

  mov sptr,rcx
  mov r8,rcx
  xor rax,rax
  mov eax,dword ptr [r8+0]
  mov cmd,eax
  
  xor rbx,rbx
  mov ebx,dword ptr [r8+4]
  mov data,ebx

  xor rcx,rcx
  mov ecx,dword ptr [r8+8]
  mov stat1,ecx
  
  xor rdx,rdx
  mov edx,dword ptr [r8+12]
  mov stat2,edx

  out 0b2h,al
  out 084h,al

  cmp eax,dword ptr [cmd]
  jne cmdok
  cmp ebx,dword ptr [data]
  jne cmdok
  cmp ecx,dword ptr [stat1]
  jne cmdok
  cmp edx,dword ptr [stat2]
  je error

cmdok:
  mov r8,sptr
  mov dword ptr [r8+0],eax
  mov dword ptr [r8+4],ebx
  mov dword ptr [r8+8],ecx
  mov dword ptr [r8+12],edx
  and eax,0ffffh
  cmp eax,0ffffh
  jne ok

error:
  xor rax,rax     ; Fehler (rax=0)
  jmp ende

ok:
  xor rax,rax     ; Erfolg (rax=1)
  inc rax

ende:
  pop rbx
  pop r9
  pop r8
  pop rdx
  pop rcx
  RET
dell_smm_lowlevel ENDP

; rcx: MSR
msr_read PROC
  push rcx
  push rdx
  
  rdmsr
  shl rax,32
  or rax,rdx
  
  pop rdx
  pop rcx
  RET
msr_read ENDP

.data
sptr  dq  0
cmd   dd  0,0
data  dd  0,0
stat1 dd  0,0
stat2 dd  0,0

END
