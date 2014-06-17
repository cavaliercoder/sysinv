#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

// 7.4.1 System Enclosure or Chassis Types 
static LPTSTR CHASSIS_TYPES[] = {
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

// 7.4.2 System Enclosure or Chassis States
static LPCTSTR CHASSIS_STATES[] = {
	_T("Unknown"),							// 0x00 Invalid
	_T("Other"),							// 0x01 Other 
	_T("Unknown"),							// 0x02 Unknown 
	_T("Safe"),								// 0x03 Safe 
	_T("Warning"),							// 0x04 Warning 
	_T("Critical"),							// 0x05 Critical 
	_T("Non-recoverable")					// 0x06 Non-recoverable 
};

// 7.4.3 System Enclosure or Chassis Security Status 
static LPCTSTR CHASSIS_SECURITY_STATUS[] = {
	_T("Unknown"),							// 0x00 Invalid
	_T("Other"),							// 0x01 Other
	_T("Unknown"),							// 0x02 Unknown
	_T("None"),								// 0x03 None
	_T("External interface locked out"),	// 0x04 External interface locked out
	_T("External interface enabled")		// 0x05 External interface enabled
};

// 7.4 System Enclosure or Chassis (Type 3) 
PNODE EnumChassis()
{
	PNODE parentNode = node_alloc(_T("Chassis"), 0);
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_CHASSIS))) {
		// v2.0+
		if (header->Length < 0x09) continue;
		node = node_append_new(parentNode, _T("Chassis"), 0);

		// 0x04 Manufacturer
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x04));
		node_att_set(node, _T("Manufacturer"), unicode, 0);
		LocalFree(unicode);

		// 0x05 Chassis Type
		node_att_set(node, _T("Type"), SAFE_INDEX(CHASSIS_TYPES, BYTE_AT_OFFSET(header, 0x05)), 0);

		// 0x06 Version String
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x06));
		node_att_set(node, _T("Version"), unicode, 0);
		LocalFree(unicode);

		// 0x07 Serial Number
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x06));
		node_att_set(node, _T("SerialNumber"), unicode, 0);
		LocalFree(unicode);

		// 0x08 Asset Tag
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x08));
		node_att_set(node, _T("AssetTag"), unicode, 0);
		LocalFree(unicode);

		// v2.1+
		if (header->Length < 0x0D) continue;

		// 0x09 Boot-up state
		node_att_set(node, _T("BootupState"), SAFE_INDEX(CHASSIS_STATES, BYTE_AT_OFFSET(header, 0x09)), 0);

		// 0x0A Power supply state
		node_att_set(node, _T("PowerSupplyState"), SAFE_INDEX(CHASSIS_STATES, BYTE_AT_OFFSET(header, 0x0A)), 0);

		// 0x0B Thermal State
		node_att_set(node, _T("ThermalState"), SAFE_INDEX(CHASSIS_STATES, BYTE_AT_OFFSET(header, 0x0B)), 0);

		// 0x0C Security State
		node_att_set(node, _T("SecurityState"), SAFE_INDEX(CHASSIS_SECURITY_STATUS, BYTE_AT_OFFSET(header, 0x0C)), 0);


		// v2.3+
		if (header->Length < 0x11) continue;
		
		// 0x0D OEM Defined
		swprintf(buffer, _T("0x%X"), DWORD_AT_OFFSET(header, 0x0D));
		node_att_set(node, _T("OemInfo"), buffer, 0);
		
		if (header->Length < 0x13) continue;

		// 0x11 Height (U|1.75"|4.445cm)
		if (0 != BYTE_AT_OFFSET(header, 0x11)) {
			swprintf(buffer, _T("%u"), BYTE_AT_OFFSET(header, 0x11));
			node_att_set(node, _T("HeightU"), buffer, 0);
		}

		// 0x12 Number of power cords
		if (0 != BYTE_AT_OFFSET(header, 0x12)) {
			swprintf(buffer, _T("%u"), BYTE_AT_OFFSET(header, 0x12));
			node_att_set(node, _T("PowerCordCount"), buffer, 0);
		}

		// v2.7+
		if (header->Length < 0x15) continue;

		// 0x15 + n*m SKU Number
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x15 + BYTE_AT_OFFSET(header, 0x15)));
		node_att_set(node, _T("SkuNumber"), unicode, 0);
		LocalFree(unicode);
	}

	return parentNode;
}