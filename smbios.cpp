#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

/*
 * See http://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.8.0.pdf
 */

LPTSTR CHASSIS_TYPE_STRINGS[] = {
	_T("Unknown"),					// 0x00 Invalid
	_T("Other"),					// 0x01
	_T("Unknown"),					// 0x02
	_T("Desktop"),					// 0x03
	_T("Low Profile Desktop"),		// 0x04
	_T("Pizza Box"),				// 0x05
	_T("Mini Tower"),				// 0x06
	_T("Tower"),					// 0x07
	_T("Portable"),					// 0x08
	_T("Laptop"),					// 0x09
	_T("Notebook"),					// 0x0A
	_T("Hand Held"),				// 0x0B
	_T("Docking Station"),			// 0x0C
	_T("All in One"),				// 0x0D
	_T("Sub Notebook"),				// 0x0E
	_T("Space-saving"),				// 0x0F
	_T("Lunch Box"),				// 0x10
	_T("Main Server Chassis"),		// 0x11
	_T("Expansion Chassis"),		// 0x12
	_T("Sub Chassis"),				// 0x13
	_T("Bus Expansion Chassis"),	// 0x14
	_T("Peripheral Chassis"),		// 0x15
	_T("RAID Chassis"),				// 0x16
	_T("Rack Mount Chassis"),		// 0x17
	_T("Sealed-case PC"),			// 0x18
	_T("Multi-system Chassis"),		// 0x19
	_T("Compact PCI"),				// 0x1A
	_T("Advanced TCA")				// 0x1B
	_T("Blade")						// 0x1C
	_T("Blade Enclosure")			// 0x1D
};

LPTSTR GetSmbiosString(PSMBIOS_STRUCT_HEADER table, BYTE index);
PNODE GetBiosDetail(PSMBIOS_STRUCT_HEADER header);
PNODE GetBiosSystemDetail(PSMBIOS_STRUCT_HEADER header);
PNODE GetBaseBoardDetail(PSMBIOS_STRUCT_HEADER header);
PNODE GetChassisDetail(PSMBIOS_STRUCT_HEADER header);
PNODE GetProcSocketDetail(PSMBIOS_STRUCT_HEADER header);

LPTSTR GetSmbiosString(PSMBIOS_STRUCT_HEADER table, BYTE index)
{
	DWORD i = 0;
	DWORD len = 0;
	LPTSTR unicode = _wcsdup(_T(""));

	if (0 == index)
		return unicode;

	char *c = NULL;

	for (i = 1, c = (char *)table + table->Length; '\0' != *c; c += strlen(c) + 1, i++) {
		if (i == index) {
			LocalFree(unicode);

			len = MultiByteToWideChar(CP_UTF8, 0, c, -1, NULL, 0);
			unicode = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR)* len);

			MultiByteToWideChar(CP_UTF8, 0, c, -1, unicode, len);
			break;
		}
	}
	
	return unicode;
}

PSMBIOS_STRUCT_HEADER GetNextStructure(PRAW_SMBIOS_DATA smbios, PSMBIOS_STRUCT_HEADER previous)
{
	PSMBIOS_STRUCT_HEADER next = NULL;
	PBYTE c = NULL;

	if (NULL == previous)
		return (PSMBIOS_STRUCT_HEADER)smbios->SMBIOSTableData;

	// Move to the end of the formatted structure
	c = ((PBYTE)previous) + previous->Length;
	
	// Search for the end of the unformatted structure (\0\0)
	while (true) {
		if ('\0' == *c && '\0' == *(c + 1))
			return (PSMBIOS_STRUCT_HEADER)(c + 2);
		c++;

		// Todo: Ensure no overflow of smbios
	}

	return NULL;
}

PNODE EnumSmbiosTables()
{
	PNODE biosNode = NULL;
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = NULL;
	DWORD bufferSize = 0;
	DWORD i = 0;

	PSMBIOS_STRUCT_HEADER header = NULL;
	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	// Get required buffer size
	bufferSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
	if (bufferSize) {
		smbios = (PRAW_SMBIOS_DATA)LocalAlloc(LPTR, bufferSize);
		bufferSize = GetSystemFirmwareTable('RSMB', 0, (PVOID)smbios, bufferSize);
	}

	// Create a node
	biosNode = node_alloc(_T("Smbios"), 0);

	swprintf(buffer, _T("%u.%u"), smbios->SMBIOSMajorVersion, smbios->SMBIOSMinorVersion);
	node_att_set(biosNode, _T("Version"), buffer, 0);
	
	swprintf(buffer, _T("%u"), smbios->DmiRevision);
	node_att_set(biosNode, _T("DmiRevision"), buffer, 0);

	// Parse tables
	while (NULL != (header = GetNextStructure(smbios, header))) {
		switch (header->Type) {
		case SMB_TABLE_BIOS:
			node = GetBiosDetail(header);
			node_append_child(biosNode, node);
			break;

		case SMB_TABLE_SYSTEM:
			node = GetBiosSystemDetail(header);
			node_append_child(biosNode, node);
			break;

		case SMB_TABLE_BASEBOARD:
			node = GetBaseBoardDetail(header);
			node_append_child(biosNode, node);
			break;

		case SMB_TABLE_CHASSIS:
			node = GetChassisDetail(header);
			node_append_child(biosNode, node);
			break;

		case SMB_TABLE_PROCESSOR:
			node = GetProcSocketDetail(header);
			node_append_child(biosNode, node);			
			break;

		default:
			goto done;
			break;
		}
	}

done:

	/*
	for (i = 0; i < smbios->Length; i++){
		printf("0x%02X %c ", smbios->SMBIOSTableData[i], smbios->SMBIOSTableData[i]);
		if (0 == (i + 1) % 4)
			printf("\n");
	}
	*/
	LocalFree(smbios);

	return biosNode;
}

// BIOS Info table (7.1)
PNODE GetBiosDetail(PSMBIOS_STRUCT_HEADER header)
{
	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	PBIOS_INFO_24 biosInfo = (PBIOS_INFO_24)header;
	PNODE node = node_alloc(_T("BiosInfo"), 0);

	unicode = GetSmbiosString(header, biosInfo->VendorNameStringIndex);
	node_att_set(node, _T("Vendor"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, biosInfo->BiosVersionStringIndex);
	node_att_set(node, _T("Version"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, biosInfo->BiosBuildDateStringIndex);
	node_att_set(node, _T("Date"), unicode, 0);
	LocalFree(unicode);

	swprintf(buffer, _T("%u.%u"), biosInfo->BiosVersionMajor, biosInfo->BiosVersionMinor);
	node_att_set(node, _T("Revision"), buffer, 0);

	swprintf(buffer, _T("%u.%u"), biosInfo->EmbeddedVersionMajor, biosInfo->EmbeddedVersionMinor);
	node_att_set(node, _T("FirmwareRevision"), buffer, 0);

	return node;
}

// System Info Table (7.2)
PNODE GetBiosSystemDetail(PSMBIOS_STRUCT_HEADER header)
{
	LPTSTR unicode = NULL;
	PSYSTEM_INFO_24 sysInfo = (PSYSTEM_INFO_24)header;
	PNODE node = node_alloc(_T("SystemInfo"), 0);

	unicode = GetSmbiosString(header, sysInfo->ManufacturerStringIndex);
	node_att_set(node, _T("Manufacturer"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, sysInfo->ProductNameStringIndex);
	node_att_set(node, _T("Product"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, sysInfo->VersionStringIndex);
	node_att_set(node, _T("Version"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, sysInfo->SerialNumberStringIndex);
	node_att_set(node, _T("SerialNumber"), unicode, 0);
	LocalFree(unicode);

	unicode = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * 40);
	StringFromGUID2(sysInfo->Uuid, unicode, 40);
	node_att_set(node, _T("Uuid"), unicode, 0);
	LocalFree(unicode);

	switch (sysInfo->WakeUpType) {
	case 0x00:
		node_att_set(node, _T("WakeUpType"), _T("Reserved"), 0);
		break;
	case 0x01:
		node_att_set(node, _T("WakeUpType"), _T("Other"), 0);
		break;
	case 0x03:
		node_att_set(node, _T("WakeUpType"), _T("APM Timer"), 0);
		break;
	case 0x04:
		node_att_set(node, _T("WakeUpType"), _T("Modem Ring"), 0);
		break;
	case 0x05:
		node_att_set(node, _T("WakeUpType"), _T("LAN Remote"), 0);
		break;
	case 0x06:
		node_att_set(node, _T("WakeUpType"), _T("Power Switch"), 0);
		break;
	case 0x07:
		node_att_set(node, _T("WakeUpType"), _T("PCI PME#"), 0);
		break;
	case 0x08:
		node_att_set(node, _T("WakeUpType"), _T("AC Power Restored"), 0);
		break;

	default:
		node_att_set(node, _T("WakeUpType"), _T("Unknown"), 0);
		break;
	}

	return node;
}

PNODE GetBaseBoardDetail(PSMBIOS_STRUCT_HEADER header)
{
	LPTSTR unicode = NULL;
	PBASEBOARD_INFO baseBoardInfo = (PBASEBOARD_INFO)header;
	PNODE node = node_alloc(_T("BaseBoard"), 0);

	switch (baseBoardInfo->BoardType) {
	case 0x02:
		node_att_set(node, _T("Type"), _T("Other"), 0);
		break;
	case 0x03:
		node_att_set(node, _T("Type"), _T("Server Blade"), 0);
		break;
	case 0x04:
		node_att_set(node, _T("Type"), _T("Connectivity Switch"), 0);
		break;
	case 0x05:
		node_att_set(node, _T("Type"), _T("Server Management Module"), 0);
		break;
	case 0x06:
		node_att_set(node, _T("Type"), _T("Processor Module"), 0);
		break;
	case 0x07:
		node_att_set(node, _T("Type"), _T("I/O Module"), 0);
		break;
	case 0x08:
		node_att_set(node, _T("Type"), _T("Memory Module"), 0);
		break;
	case 0x09:
		node_att_set(node, _T("Type"), _T("Daughter Board"), 0);
		break;
	case 0x0A:
		node_att_set(node, _T("Type"), _T("Motherboard"), 0);
		break;
	case 0x0B:
		node_att_set(node, _T("Type"), _T("Processor/Memory Module"), 0);
		break;
	case 0x0C:
		node_att_set(node, _T("Type"), _T("Processor/IO Module"), 0);
		break;
	case 0x0D:
		node_att_set(node, _T("Type"), _T("Interconnect board"), 0);
		break;
	default:
		node_att_set(node, _T("Type"), _T("Unknown"), 0);
		break;
	}

	unicode = GetSmbiosString(header, baseBoardInfo->ManufacturerStringIndex);
	node_att_set(node, _T("Manufacturer"), unicode, 0);
	LocalFree(unicode);


	unicode = GetSmbiosString(header, baseBoardInfo->ProductNameStringIndex);
	node_att_set(node, _T("Product"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, baseBoardInfo->VersionStringIndex);
	node_att_set(node, _T("Version"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, baseBoardInfo->SerialNumberStringIndex);
	node_att_set(node, _T("SerialNumber"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, baseBoardInfo->AssetTagStringIndex);
	node_att_set(node, _T("AssetTag"), unicode, 0);
	LocalFree(unicode);

	return node;
}

PNODE GetChassisDetail(PSMBIOS_STRUCT_HEADER header)
{
	LPTSTR unicode = NULL;
	PCHASSIS_INFO_23 chassisInfo = (PCHASSIS_INFO_23)header;
	PNODE node = node_alloc(_T("Chassis"), 0);

	unicode = GetSmbiosString(header, chassisInfo->ManufacturerStringIndex);
	node_att_set(node, _T("Manufacturer"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, chassisInfo->VersionStringIndex);
	node_att_set(node, _T("Version"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, chassisInfo->SerialNumberStringIndex);
	node_att_set(node, _T("SerialNumber"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, chassisInfo->AssetTagStringIndex);
	node_att_set(node, _T("AssetTag"), unicode, 0);
	LocalFree(unicode);

	if (chassisInfo->Type < ARRAYSIZE(CHASSIS_TYPE_STRINGS))
		node_att_set(node, _T("Type"), CHASSIS_TYPE_STRINGS[chassisInfo->Type], 0);
	else
		node_att_set(node, _T("Type"), CHASSIS_TYPE_STRINGS[0], 0);

	return node;
}

PNODE GetProcSocketDetail(PSMBIOS_STRUCT_HEADER header)
{
	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];
	PNODE node = NULL;
	PPROC_INFO_26 procInfo = (PPROC_INFO_26)header;

	// Ignore unpopulated sockets
	// Bit 6 = CPU Sock Populated
	if (!(procInfo->Status >> 6)) 
		return node;

	node = node_alloc(_T("Processor"), 0);

	unicode = GetSmbiosString(header, procInfo->DesignationStringIndex);
	node_att_set(node, _T("Designation"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, procInfo->ManufacturerStringIndex);
	node_att_set(node, _T("Manufacturer"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, procInfo->VersionStringIndex);
	node_att_set(node, _T("Version"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, procInfo->SerialNumberStringIndex);
	node_att_set(node, _T("SerialNumber"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, procInfo->AssetTagStringIndex);
	node_att_set(node, _T("AssetTag"), unicode, 0);
	LocalFree(unicode);

	unicode = GetSmbiosString(header, procInfo->PartNumberStringIndex);
	node_att_set(node, _T("PartNumber"), unicode, 0);
	LocalFree(unicode);

	swprintf(buffer, _T("%uMhz"), procInfo->MaxSpeed);
	node_att_set(node, _T("MaxSpeed"), buffer, 0);

	swprintf(buffer, _T("%uMhz"), procInfo->CurrentSpeed);
	node_att_set(node, _T("CurrentSpeed"), buffer, 0);

	return node;
}