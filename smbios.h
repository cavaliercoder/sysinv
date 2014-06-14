#include "stdafx.h"

#ifndef SMBIOS_H
#define SMBIOS_H

#define SMB_TABLE_BIOS				0
#define SMB_TABLE_SYSTEM			1
#define SMB_TABLE_BASEBOARD			2
#define SMB_TABLE_CHASSIS			3
#define SMB_TABLE_PROCESSOR			4
#define SMB_TABLE_MEMCTRL			5
#define SMB_TABLE_MEMMODULES		6
#define SMB_TABLE_SLOTS				9
#define SMB_TABLE_OEM_STRINGS		11
#define SMB_TABLE_SYS_CFG_OPTIONS	12
#define SMB_TABLE_MEM_ARRAY			16
#define SMB_TABLE_END_OF_TABLE		127

#define SAFE_INDEX(a, i)			a[ARRAYSIZE(a) > i ? i : 0]

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
	BYTE    SMBIOSTableData[];
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

LPTSTR GetSmbiosString(PSMBIOS_STRUCT_HEADER table, BYTE index);

typedef struct _MemoryController
{
	/* v2.0+ */
	SMBIOS_STRUCT_HEADER Header;
	BYTE ErrorDetectionMethod;
	BYTE ErrorCorrectingCapability;
	BYTE SupportedInterleave;
	BYTE CurrentInterleave;
	BYTE MaximumModuleSize;
	WORD SupportedSpeeds;
	WORD SupportedMemoryTypes;
	BYTE ModuleVoltage;
	BYTE AssociatedSlots;
	BYTE ModuleConfigHandles; // Variable length
	/* v2.1+ */
	// BYTE EnabledErrorCorrectionCapabilities;
} MEMORY_CONTROLLER, *PMEMORY_CONTROLLER;

#endif