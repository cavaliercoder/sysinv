#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

// Baseboard Table Type 2
PNODE EnumBaseboards()
{
	PNODE baseBoardsNode = node_alloc(_T("Baseboards"), 0);
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_BASEBOARD))) {
		cursor = (PBYTE)header;

		// v2.0+
		if (2 <= smbios->SMBIOSMajorVersion) {
			node = node_append_new(baseBoardsNode, _T("BaseBoard"), 0);

			// 0x04 Manufacturer
			unicode = GetSmbiosString(header, *(cursor + 0x04));
			node_att_set(node, _T("Manufacturer"), unicode, 0);
			LocalFree(unicode);

			// 0x05 Product
			unicode = GetSmbiosString(header, *(cursor + 0x05));
			node_att_set(node, _T("Product"), unicode, 0);
			LocalFree(unicode);

			// 0x06 Version String
			unicode = GetSmbiosString(header, *(cursor + 0x06));
			node_att_set(node, _T("Version"), unicode, 0);
			LocalFree(unicode);

			// 0x07 Serial Number
			unicode = GetSmbiosString(header, *(cursor + 0x07));
			node_att_set(node, _T("SerialNumber"), unicode, 0);
			LocalFree(unicode);

			// 0x08 Asset Tag
			unicode = GetSmbiosString(header, *(cursor + 0x08));
			node_att_set(node, _T("AssetTag"), unicode, 0);
			LocalFree(unicode);

			// 0x0A Location in Chassis
			unicode = GetSmbiosString(header, *(cursor + 0x0A));
			node_att_set(node, _T("Location"), unicode, 0);
			LocalFree(unicode);

			// 0x0D Board Type
			switch (*(cursor + 0x0D)) {
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
		}
	}

	return baseBoardsNode;
}