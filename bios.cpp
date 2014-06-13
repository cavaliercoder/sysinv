#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

// BIOS Info table (7.1)
PNODE GetBiosDetail()
{
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	header = GetNextStructureOfType(NULL, SMB_TABLE_BIOS);
	if (NULL == header)
		return node;

	cursor = (PBYTE)header;

	// v2.0+
	if (2 <= smbios->SMBIOSMajorVersion) {
		node = node_alloc(_T("Bios"), 0);

		// 0x04 Vendor
		unicode = GetSmbiosString(header, *(cursor + 0x04));
		node_att_set(node, _T("Vendor"), unicode, 0);
		LocalFree(unicode);

		// 0x05 BIOS Version
		unicode = GetSmbiosString(header, *(cursor + 0x05));
		node_att_set(node, _T("Version"), unicode, 0);
		LocalFree(unicode);

		// 0x08 BIOS Release Date
		unicode = GetSmbiosString(header, *(cursor + 0x08));
		node_att_set(node, _T("Date"), unicode, 0);
		LocalFree(unicode);

		// v2.4 +
		if (2 < smbios->SMBIOSMajorVersion || 4 <= smbios->SMBIOSMinorVersion) {
			// 0x14, 0x15 System BIOS Release
			swprintf(buffer, _T("%u.%u"), *(cursor + 0x14), *(cursor + 0x15));
			node_att_set(node, _T("Revision"), buffer, 0);

			// 0x16, 0x17 Embedded Controller Firmware Release
			swprintf(buffer, _T("%u.%u"), *(cursor + 0x16), *(cursor + 0x17));
			node_att_set(node, _T("FirmwareRevision"), buffer, 0);
		}
	}

	return node;
}