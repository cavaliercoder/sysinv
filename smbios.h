#include "stdafx.h"

#ifndef SMBIOS_H
#define SMBIOS_H

// SMBIOS Table Type numbers
#define SMB_TABLE_BIOS				0
#define SMB_TABLE_SYSTEM			1
#define SMB_TABLE_BASEBOARD			2
#define SMB_TABLE_CHASSIS			3
#define SMB_TABLE_PROCESSOR			4
#define SMB_TABLE_MEMCTRL			5
#define SMB_TABLE_MEMMODULES		6
#define SMB_TABLE_PORTS				8
#define SMB_TABLE_SLOTS				9
#define SMB_TABLE_OEM_STRINGS		11
#define SMB_TABLE_SYS_CFG_OPTIONS	12
#define SMB_TABLE_MEM_ARRAY			16
#define SMB_TABLE_MEM_DEVICE		17
#define SMB_TABLE_END_OF_TABLE		127

// CPU registers returned by __cpuid()
#define EAX							0
#define EBX							1
#define ECX							2
#define EDX							3

// Virtualization platform providers
#define VIRT_PLATFORM_NONE			0
#define VIRT_PLATFORM_VMWARE		1
#define VIRT_PLATFORM_VBOX			2
#define VIRT_PLATFORM_QEMU			3
#define VIRT_PLATFORM_KVM			4
#define VIRT_PLATFORM_MSVPC			5
#define VIRT_PLATFORM_HYPERV		6
#define VIRT_PLATFORM_XEN			7
#define VIRT_PLATFORM_VTZO			8
#define VIRT_PLATFORM_PARA			9

// 64bit Word type
typedef unsigned long long QWORD;

/*
* Structures
*/
typedef struct _RawSmbiosData
{
	BYTE    Used20CallingMethod;
	BYTE    SMBIOSMajorVersion;
	BYTE    SMBIOSMinorVersion;
	BYTE    DmiRevision;
	DWORD   Length;
	BYTE    SMBIOSTableData[ANYSIZE_ARRAY];
} RAW_SMBIOS_DATA, * PRAW_SMBIOS_DATA;

typedef struct _SmbiosStructHeader
{
	BYTE Type;
	BYTE Length;
	WORD Handle;
} SMBIOS_STRUCT_HEADER, *PSMBIOS_STRUCT_HEADER;

/*
 * Functions
 */
PRAW_SMBIOS_DATA GetSmbiosData();
void ReleaseSmbiosData();

PSMBIOS_STRUCT_HEADER GetNextStructure(PSMBIOS_STRUCT_HEADER previous);
PSMBIOS_STRUCT_HEADER GetNextStructureOfType(PSMBIOS_STRUCT_HEADER previous, DWORD type);
PSMBIOS_STRUCT_HEADER GetStructureByHandle(WORD handle);
LPTSTR GetSmbiosString(PSMBIOS_STRUCT_HEADER table, BYTE index);

#endif