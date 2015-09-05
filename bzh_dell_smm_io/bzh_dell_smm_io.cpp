#define BZH_DEBUG
#ifndef BZH_DEBUG
//bzh_debug
#endif

#include "bzh_dell_smm_drv.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Function definition section
// -----------------------------------------------------------------
NTSTATUS Create(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS Close(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
void Unload(IN PDRIVER_OBJECT DriverObject);

// -----------------------------------------------------------------

// Installable driver initialization entry point.
// This entry point is called directly by the I/O system.
NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject,
                      IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
    UNICODE_STRING  DeviceNameUnicodeString;
    UNICODE_STRING  DeviceLinkUnicodeString;
    NTSTATUS        ntStatus;
    PDEVICE_OBJECT  DeviceObject = NULL;

    KdPrint(("Entering DriverEntry"));

    RtlInitUnicodeString (&DeviceNameUnicodeString, L"\\Device\\BZHDELLSMMIO");

    // Create a device object

    ntStatus = IoCreateDevice (DriverObject,
                               0,
                               &DeviceNameUnicodeString,
                               FILE_DEVICE_BZH_DELL_SMM,
                               0,
                               FALSE,
                               &DeviceObject);

    if (NT_SUCCESS(ntStatus))
    {
        // Create dispatch points for device control, create, close.


		DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Dispatch;
        DriverObject->DriverUnload = Unload;

        // Create a symbolic link, e.g. a name that a Win32 app can specify
        // to open the device.

        RtlInitUnicodeString (&DeviceLinkUnicodeString, L"\\DosDevices\\BZHDELLSMMIO");

        ntStatus = IoCreateSymbolicLink (&DeviceLinkUnicodeString,
                                         &DeviceNameUnicodeString);

        if (!NT_SUCCESS(ntStatus))
        {
            // Symbolic link creation failed- note this & then delete the
            // device object (it's useless if a Win32 app can't get at it).
            KdPrint(("ERROR: IoCreateSymbolicLink failed"));
            IoDeleteDevice (DeviceObject);
        }
    }
    else
    {
        KdPrint(("ERROR: IoCreateDevice failed"));
    }

    KdPrint(("Leaving DriverEntry"));

    return ntStatus;
}


// Process the IRPs sent to this device

NTSTATUS Dispatch(IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	ULONG InBufferSize;           // actual input buffer size
	ULONG OutBufferSize;          // actual output buffer size
	ULONG ExpInBufferSize = sizeof(SMBIOS_PKG);        // expected input buffer size
	ULONG ExpOutBufferSize = sizeof(SMBIOS_PKG);       // expected input buffer size
	ULONG IoctlCode;              // Ioctl code from process
	PVOID pIOBuffer;             // buffered memory for communication with process
    NTSTATUS ntStatus;
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

    KdPrint(("Entering Dispatch"));

    // Init to default settings
	Irp->IoStatus.Information = 0;    // default return value for IoStatus
    Irp->IoStatus.Status      = STATUS_SUCCESS;

	// retrieve IO control code
	IoctlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;

	// retrieve actual buffer lengths
	InBufferSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
	OutBufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	// NT copies inbuf here before entry and copies this to outbuf after return,
	// for METHOD_BUFFERED IOCTL's (one buffer for input and output)
	pIOBuffer = Irp->AssociatedIrp.SystemBuffer;


    switch (IrpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:
        KdPrint(("IRP_MJ_CREATE"));
		Create(DeviceObject, Irp);
        break;
    case IRP_MJ_CLOSE:
		KdPrint(("IRP_MJ_CLOSE"));
		Close(DeviceObject, Irp);
        break;
    case IRP_MJ_DEVICE_CONTROL:
        KdPrint(("IRP_MJ_DEVICE_CONTROL"));
        switch (IoctlCode)
        {
        case IOCTL_BZH_DELL_SMM_RWREG:
            KdPrint(("IOCTL_BZH_DELL_SMM_RWREG"));
			if (InBufferSize < ExpInBufferSize || OutBufferSize < ExpOutBufferSize)
			{
				KdPrint(("ERROR: STATUS_BUFFER_TOO_SMALL"));
				Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			else
			{
				SMBIOS_PKG smm_pkg;
				bool smm_opt_status = false;
				memcpy(&smm_pkg, pIOBuffer, InBufferSize);
				smm_opt_status = dell_smm_cmd_send(&smm_pkg);
				memcpy(pIOBuffer, &smm_pkg, OutBufferSize);
                Irp->IoStatus.Information = ExpOutBufferSize;
				if (smm_opt_status)
				{
					Irp->IoStatus.Status = STATUS_SUCCESS;
				} 
				else
				{
					Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				}
				break;
            }
        default:
            KdPrint(("ERROR: Unknown IRP_MJ_DEVICE_CONTROL"));
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            break;
        }
        break;
    }

    // DON'T get cute and try to use the status field of the irp in the
    // return status.  That IRP IS GONE as soon as you call IoCompleteRequest.

    ntStatus = Irp->IoStatus.Status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    // We never have pending operation so always return the status code.

    KdPrint(("Leaving Dispatch"));

    return ntStatus;
}

// Delete the associated device and return

NTSTATUS Create(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	KdPrint(("Entering Create"));
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	KdPrint(("Leaving Create"));
	return STATUS_SUCCESS;
}
NTSTATUS Close(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	KdPrint(("Entering Close"));
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	KdPrint(("Leaving Close"));
	return STATUS_SUCCESS;
}
void Unload(IN PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING DeviceLinkUnicodeString;
    NTSTATUS ntStatus;

    KdPrint(("Entering Unload"));

    RtlInitUnicodeString (&DeviceLinkUnicodeString, L"\\DosDevices\\BZHDELLSMMIO");

    ntStatus = IoDeleteSymbolicLink (&DeviceLinkUnicodeString);

    if (NT_SUCCESS(ntStatus))
    {
        IoDeleteDevice (DriverObject->DeviceObject);
    }
    else
    {
        KdPrint(("ERROR: IoDeleteSymbolicLink"));
    }

    KdPrint(("Leaving Unload"));
}

#ifdef __cplusplus
}
#endif
