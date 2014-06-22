#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

/*
 * See http://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.8.0.pdf
 */

PNODE GetMemoryControllerDetail(PSMBIOS_STRUCT_HEADER header);
PNODE GetOemStringsDetail(PSMBIOS_STRUCT_HEADER header);

PRAW_SMBIOS_DATA smbios = NULL;

PRAW_SMBIOS_DATA GetSmbiosData()
{
	DWORD bufferSize = 0;

	if (NULL != smbios)
		return smbios;

	// Get required buffer size
	bufferSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
	if (bufferSize) {
		smbios = (PRAW_SMBIOS_DATA)LocalAlloc(LPTR, bufferSize);
		bufferSize = GetSystemFirmwareTable('RSMB', 0, (PVOID)smbios, bufferSize);
	}

	return smbios;
}

void ReleaseSmbiosData()
{
	if (NULL != smbios) {
		LocalFree(smbios);
		smbios = NULL;
	}
}

PSMBIOS_STRUCT_HEADER GetNextStructure(PSMBIOS_STRUCT_HEADER previous)
{
	PSMBIOS_STRUCT_HEADER next = NULL;
	PBYTE c = NULL;

	// Init SMBIOS data
	if (NULL == smbios)
		smbios = GetSmbiosData();

	// Return NULL is no data found
	if (NULL == smbios)
		return NULL;
	
	// Return first table if previous was NULL
	if (NULL == previous)
		return (PSMBIOS_STRUCT_HEADER)(&smbios->SMBIOSTableData[0]);

	// Move to the end of the formatted structure
	c = ((PBYTE)previous) + previous->Length;
	
	// Search for the end of the unformatted structure (\0\0)
	while (true) {
		if ('\0' == *c && '\0' == *(c + 1)) {
			/* Make sure next table is not beyond end of SMBIOS data
			 * (Thankyou Microsoft for ommitting the structure count
			 * in GetSystemFirmwareTable
			 */
			if ((c + 2) < ((PBYTE)smbios->SMBIOSTableData + smbios->Length))
				return (PSMBIOS_STRUCT_HEADER)(c + 2);
			else
				return NULL; // We reached the end
		}
			
		c++;
	}

	return NULL;
}

PSMBIOS_STRUCT_HEADER GetNextStructureOfType(PSMBIOS_STRUCT_HEADER previous, DWORD type)
{
	PSMBIOS_STRUCT_HEADER next = previous;
	while (NULL != (next = GetNextStructure(next))) {
		if (type == next->Type)
			return next;
	}

	return NULL;
}

PSMBIOS_STRUCT_HEADER GetStructureByHandle(WORD handle)
{
	PSMBIOS_STRUCT_HEADER header = NULL;

	while (NULL != (header = GetNextStructure(header)))
		if (handle == header->Handle)
			return header;

	return NULL;
}

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

PNODE GetSmbiosDetail()
{
	PNODE biosNode = NULL;
	PNODE node = NULL;
	DWORD i = 0;

	PSMBIOS_STRUCT_HEADER header = NULL;
	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	GetSmbiosData();

	// Create a node
	biosNode = node_alloc(_T("Smbios"), 0);

	swprintf(buffer, _T("%u.%u"), smbios->SMBIOSMajorVersion, smbios->SMBIOSMinorVersion);
	node_att_set(biosNode, _T("Version"), buffer, 0);

	swprintf(buffer, _T("%u"), smbios->DmiRevision);
	node_att_set(biosNode, _T("DmiRevision"), buffer, NAFLG_FMT_NUMERIC);

done:

	return biosNode;
}

// Type 5
PNODE GetMemoryControllerDetail(PSMBIOS_STRUCT_HEADER header)
{
	return NULL;
}

// Type 11
PNODE EnumOemStrings()
{
	PNODE node = NULL;
	PNODE stringNode = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[64];

	DWORD i;

	header = GetNextStructureOfType(header, SMB_TABLE_OEM_STRINGS);
	if (NULL == header)
		return node;

	node = node_alloc(_T("OemStrings"), NFLG_TABLE);
	cursor = (PBYTE)header;
	
	// 0x04 Count
	for (i = 1; i < *(cursor + 0x04); i++) {
		// String index
		_snwprintf(buffer, 64, _T("%u"), i);
		unicode = GetSmbiosString(header, i);

		// String value
		stringNode = node_append_new(node, _T("OemString"), NFLG_TABLE_ROW);
		node_att_set(stringNode, _T("Index"), buffer, NAFLG_FMT_NUMERIC);
		node_att_set(stringNode, _T("Value"), unicode, 0);
		LocalFree(unicode);
	}
	
	return node;
}