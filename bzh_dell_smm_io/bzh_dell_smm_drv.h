#pragma once

#include <ntddk.h>
#include "bzh_dell_smm_lowlevel.h"

#ifdef __cplusplus
extern "C"
{
#endif

DRIVER_INITIALIZE DriverEntry;
_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH Create;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH Close;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH Dispatch;
DRIVER_UNLOAD Unload;

// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.

#define FILE_DEVICE_BZH_DELL_SMM 0x0000B424

// Macro definition for defining IOCTL and FSCTL function control codes.
// Note that function codes 0-2047 are reserved for Microsoft Corporation,
// and 2048-4095 are reserved for customers.

#define BZH_DELL_SMM_IOCTL_KEY 0xB42 //2882

// Define our own private IOCTL

#define IOCTL_BZH_DELL_SMM_RWREG     CTL_CODE(FILE_DEVICE_BZH_DELL_SMM,  \
    BZH_DELL_SMM_IOCTL_KEY,      \
    METHOD_BUFFERED,        \
    FILE_ANY_ACCESS)

#define SMM_DISABLE_BIOS_METHOD1 0x30a3
#define SMM_ENABLE_BIOS_METHOD1  0x31a3
#define SMM_DISABLE_BIOS_METHOD2 0x34a3
#define SMM_ENABLE_BIOS_METHOD2  0x35a3
#define SMM_ENABLE_FN            0x32a3
#define SMM_SET_FAN              0x01a3
#define SMM_GET_FAN              0x00a3
#define SMM_FN_STATUS            0x0025
#define SMM_GET_SPEED            0x02a3

bool dell_smm_cmd_send(SMBIOS_PKG *smm_pkg)
{
	int x;

	x = dell_smm_lowlevel(smm_pkg);

	if (x != 1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

#ifdef __cplusplus
}
#endif