#include "stdafx.h"

typedef unsigned long long QWORD;

#define SMB_TABLE_BIOS			0
#define SMB_TABLE_SYSTEM		1
#define SMB_TABLE_BASEBOARD		2
#define SMB_TABLE_CHASSIS		3
#define SMB_TABLE_PROCESSOR		4

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

typedef struct _BiosInfo24
{
	SMBIOS_STRUCT_HEADER Header;
	BYTE VendorNameStringIndex;
	BYTE BiosVersionStringIndex;
	BYTE BiosStartAddess[2];
	BYTE BiosBuildDateStringIndex;
	BYTE BiosRomIs128K;
	BYTE BiosCharacteristics[8];
	BYTE BiosCharExtention1;
	BYTE BiosCharExtention2;
	BYTE BiosVersionMajor;
	BYTE BiosVersionMinor;
	BYTE EmbeddedVersionMajor;
	BYTE EmbeddedVersionMinor;
} BIOS_INFO_24, *PBIOS_INFO_24;

typedef struct _SystemInfo24
{
	SMBIOS_STRUCT_HEADER Header;
	BYTE ManufacturerStringIndex;
	BYTE ProductNameStringIndex;
	BYTE VersionStringIndex;
	BYTE SerialNumberStringIndex;
	GUID Uuid;
	BYTE WakeUpType;
	BYTE SkuStringIndex;
	BYTE FamilyStringIndex;
} SYSTEM_INFO_24, *PSYSTEM_INFO_24;

typedef struct _BaseBoardInfo
{
	SMBIOS_STRUCT_HEADER Header;
	BYTE ManufacturerStringIndex;
	BYTE ProductNameStringIndex;
	BYTE VersionStringIndex;
	BYTE SerialNumberStringIndex;
	BYTE AssetTagStringIndex;
	BYTE Features;
	BYTE LocationInChassisStringIndex;
	WORD ChassisHandle;
	BYTE BoardType;
	BYTE ObjectHandleCount;
} BASEBAORD_INFO, *PBASEBOARD_INFO;

typedef struct _ChassisInfo23
{
	SMBIOS_STRUCT_HEADER Header;
	BYTE ManufacturerStringIndex;
	BYTE Type;
	BYTE VersionStringIndex;
	BYTE SerialNumberStringIndex;
	BYTE AssetTagStringIndex;
	BYTE BootupState;
	BYTE PowerSupplyState;
	BYTE ThermalState;
	BYTE SecurityStatus;
	DWORD OemInfo;
	BYTE HeightU;
	BYTE PowerCordCount;
	BYTE ContainElementCount;
} CHASSIS_INFO_23, *PCHASSIS_INFO_23;

typedef struct _ProcInfo26
{
	SMBIOS_STRUCT_HEADER Header;
	/* v2.0+ */
	BYTE DesignationStringIndex;
	BYTE ProcessorType;
	BYTE ProcessorFamily;
	BYTE ManufacturerStringIndex;
	BYTE Identification[8];
	BYTE VersionStringIndex;
	BYTE Voltage;
	WORD ExternalClock;
	WORD MaxSpeed;
	WORD CurrentSpeed;
	BYTE Status;
	BYTE Upgrade;
	/* v2.1+ */
	WORD L1CacheHandle;
	WORD L2CacheHandle;
	WORD L3CacheHandle;
	/* v2.3+ */
	BYTE SerialNumberStringIndex;
	BYTE AssetTagStringIndex;
	BYTE PartNumberStringIndex;
	/* v2.5+ */
	BYTE CoreCount;
	BYTE CoresEnabled;
	BYTE ThreadCount;
	WORD Characteristics;
	/* v2.6+ */
	WORD ProcessorFamily2;
} PROC_INFO_26, *PPROC_INFO_26;