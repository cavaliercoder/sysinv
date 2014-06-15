#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include "baseboard.h"

// 7.3.1 Baseboard — feature flags
LPCTSTR BOARD_FEATURES[] = {
	_T("Hosting board"),					// Bit 0
	_T("Requires daughter board"),			// Bit 1
	_T("Removable"),						// Bit 2
	_T("Replaceable"),						// Bit 3
	_T("Hot swappable")						// Bit 4
};

// 7.3.2 Baseboard — Board Type
LPCTSTR BOARD_TYPE[] = {
	_T("Invalid"),							// 0x00 Invalid
	_T("Unknown"),							// 0x01 Unknown
	_T("Other"),							// 0x02 Other
	_T("Server Blade"),						// 0x03 Server Blade
	_T("Connectivity Switch"),				// 0x04 Connectivity Switch
	_T("System Management Module"),			// 0x05 System Management Module
	_T("Processor Module"),					// 0x06 Processor Module
	_T("I/O Module"),						// 0x07 I / O Module
	_T("Memory Module"),					// 0x08 Memory Module
	_T("Daughter board"),					// 0x09 Daughter board
	_T("Motherboard"),						// 0x0A Motherboard
	_T("Processor/Memory Module"),			// 0x0B Processor / Memory Module
	_T("Processor/IO Module"),				// 0x0C Processor / IO Module
	_T("Interconnect board")				// 0x0D Interconnect board
};

// Baseboard Table Type 2
PNODE EnumBaseboards()
{
	PNODE baseBoardsNode = node_alloc(_T("Baseboards"), 0);
	PNODE node = NULL;
	PNODE slotsNode = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PSMBIOS_STRUCT_HEADER childHeader = NULL;
	
	LPTSTR unicode = NULL;
	DWORD i = 0;

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_BASEBOARD))) {
		node = node_append_new(baseBoardsNode, _T("BaseBoard"), 0);

		// 0x04 Manufacturer
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x04));
		node_att_set(node, _T("Manufacturer"), unicode, 0);
		LocalFree(unicode);

		// 0x05 Product
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x05));
		node_att_set(node, _T("Product"), unicode, 0);
		LocalFree(unicode);

		// 0x06 Version String
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x06));
		node_att_set(node, _T("Version"), unicode, 0);
		LocalFree(unicode);

		// 0x07 Serial Number
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x07));
		node_att_set(node, _T("SerialNumber"), unicode, 0);
		LocalFree(unicode);

		// 0x08 Asset Tag
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x08));
		node_att_set(node, _T("AssetTag"), unicode, 0);
		LocalFree(unicode);

		// 0x09 Feature flags
		unicode = NULL;
		for (i = 0; i < ARRAYSIZE(BOARD_FEATURES); i++) {
			if (CHECK_BIT(BYTE_AT_OFFSET(header, 0x09), i)) {
				AppendMultiString(&unicode, BOARD_FEATURES[i]);
			}
		}
		node_att_set_multi(node, _T("Features"), unicode, 0);
		LocalFree(unicode);

		// 0x0A Location in Chassis
		unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x0A));
		node_att_set(node, _T("Location"), unicode, 0);
		LocalFree(unicode);
			
		// 0x0D Board Type
		node_att_set(node, _T("Type"), SAFE_INDEX(BOARD_TYPE, BYTE_AT_OFFSET(header, 0x0D)), 0);

		// Children
		slotsNode = node_append_new(node, _T("Slots"), NODE_FLAG_TABLE);


		if (0 == BYTE_AT_OFFSET(header, 0x0E)) {
			// Assume everything is a child
			// Start with slots
			while (NULL != (childHeader = GetNextStructureOfType(childHeader, SMB_TABLE_SLOTS))) {
				node_append_child(slotsNode, GetSlotDetail(smbios, childHeader));
			}
		}
		else {
			// Enumerate explicitely linked children
			for (i = 0; i < BYTE_AT_OFFSET(header, 0x0E); i++) {
				// Fetch child by handle
				childHeader = GetStructureByHandle(WORD_AT_OFFSET(header, 0x0F + (i * 0x02)));

				// Add node according to type
				switch (childHeader->Type) {
				case SMB_TABLE_SLOTS:
					node_append_child(slotsNode, GetSlotDetail(smbios, childHeader));
					break;
				}
				
			}
		}
	}

	return baseBoardsNode;
}