#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

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

PNODE EnumChassis()
{
	PNODE parentNode = node_alloc(_T("Chassis"), 0);
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_CHASSIS))) {
		cursor = (PBYTE)header;

		// v2.0+
		if (2 <= smbios->SMBIOSMajorVersion) {
			node = node_append_new(parentNode, _T("Chassis"), 0);

			// 0x04 Manufacturer
			unicode = GetSmbiosString(header, *(cursor + 0x04));
			node_att_set(node, _T("Manufacturer"), unicode, 0);
			LocalFree(unicode);

			// 0x05 Chassis Type
			if (*(cursor + 0x05) < ARRAYSIZE(CHASSIS_TYPE_STRINGS))
				node_att_set(node, _T("Type"), CHASSIS_TYPE_STRINGS[*(cursor + 0x05)], 0);
			else
				node_att_set(node, _T("Type"), CHASSIS_TYPE_STRINGS[0], 0);

			// 0x06 Version String
			unicode = GetSmbiosString(header, *(cursor + 0x06));
			node_att_set(node, _T("Version"), unicode, 0);
			LocalFree(unicode);

			// 0x07 Serial Number
			unicode = GetSmbiosString(header, *(cursor + 0x06));
			node_att_set(node, _T("SerialNumber"), unicode, 0);
			LocalFree(unicode);

			// 0x08 Asset Tag
			unicode = GetSmbiosString(header, *(cursor + 0x08));
			node_att_set(node, _T("AssetTag"), unicode, 0);
			LocalFree(unicode);
		}		
	}

	return parentNode;
}