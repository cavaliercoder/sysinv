#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include <intrin.h>

#define BUFLEN	128

PNODE EnumProcessors()
{
	int cpuinfo[4];
	int i;
	int nExIds;
	char buffer[BUFLEN];
	TCHAR strBuffer[BUFLEN];
	PNODE node = node_alloc(L"Processors", NODE_FLAG_TABLE);

	// Get cpu manufacturor
	__cpuid(cpuinfo, 0);
	memset(buffer, 0, BUFLEN);
	memcpy(&buffer[0], &cpuinfo[1], 4);
	memcpy(&buffer[4], &cpuinfo[3], 4);
	memcpy(&buffer[8], &cpuinfo[2], 4);
	MultiByteToWideChar(CP_UTF8, 0, buffer, -1, strBuffer, BUFLEN);
	node_att_set(node, L"Manufacturer", strBuffer, 0);


	// Get model info
	__cpuid(cpuinfo, 1);
	swprintf(strBuffer, L"%u", (cpuinfo[0] & 0xF));
	node_att_set(node, L"Stepping", strBuffer, 0);

	swprintf(strBuffer, L"%u", (cpuinfo[0] >> 4 )& 0xF);
	node_att_set(node, L"Model", strBuffer, 0);

	swprintf(strBuffer, L"%u", (cpuinfo[0]  >> 8) & 0xF);
	node_att_set(node, L"Family", strBuffer, 0);

	swprintf(strBuffer, L"%u", (cpuinfo[0]  >> 12) & 0xF);
	node_att_set(node, L"Type", strBuffer, 0);

	swprintf(strBuffer, L"%u", (cpuinfo[0]  >> 16) & 0xF);
	node_att_set(node, L"ExtendedModel", strBuffer, 0);

	swprintf(strBuffer, L"%u", (cpuinfo[0]  >> 20) & 0xFF);
	node_att_set(node, L"ExtendedFamily", strBuffer, 0);

	// Get CPU Brand String
	memset(buffer, 0, BUFLEN);
	__cpuid(cpuinfo, 0x80000000);
	nExIds = cpuinfo[0];
	for(i = 0x80000000; i < nExIds; i++) {
		__cpuid(cpuinfo, i);
		switch(i) {
		case 0x80000002:
			memcpy(&buffer[0], cpuinfo, sizeof(cpuinfo));
			break;

		case 0x80000003:
			memcpy(&buffer[16], cpuinfo, sizeof(cpuinfo));
			break;

		case 0x80000004:
			memcpy(&buffer[32], cpuinfo, sizeof(cpuinfo));
			break;
		}
	}
	MultiByteToWideChar(CP_UTF8, 0, buffer, -1, strBuffer, BUFLEN);
	node_att_set(node, L"BrandString", strBuffer, 0);

	return node;
}

// SMBIOS Table Type 4
PNODE EnumProcSockets()
{
	PNODE procSocketsNode = node_alloc(_T("ProcessorSockets"), 0);
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_PROCESSOR))) {
		cursor = (PBYTE)header;

		// v2.0+
		if (2 <= smbios->SMBIOSMajorVersion) {
			// Ignore unpopulated sockets
			// Bit 6 = CPU Sock Populated
			if (!(*(cursor + 0x18) >> 6))
				return node;

			node = node_append_new(procSocketsNode, _T("ProcessorSockets"), 0);

			// 0x04 Designation
			unicode = GetSmbiosString(header, *(cursor + 0x04));
			node_att_set(node, _T("Designation"), unicode, 0);
			LocalFree(unicode);

			// 0x07 Manufacturer
			unicode = GetSmbiosString(header, *(cursor + 0x07));
			node_att_set(node, _T("Manufacturer"), unicode, 0);
			LocalFree(unicode);

			// 0x10 Processor Version String
			unicode = GetSmbiosString(header, *(cursor + 0x10));
			node_att_set(node, _T("Version"), unicode, 0);
			LocalFree(unicode);

			// 0x14 Max Speed
			swprintf(buffer, _T("%uMhz"), *((WORD *)(cursor + 0x14)));
			node_att_set(node, _T("MaxSpeed"), buffer, 0);

			// 0x16 Current Speed
			swprintf(buffer, _T("%uMhz"), *((WORD *)(cursor + 0x16)));
			node_att_set(node, _T("CurrentSpeed"), buffer, 0);

			// v2.3+
			if (2 < smbios->SMBIOSMajorVersion || (2 == smbios->SMBIOSMajorVersion && 3 <= smbios->SMBIOSMinorVersion)) {
				// 0x20 Serial Number
				unicode = GetSmbiosString(header, *(cursor + 0x20));
				node_att_set(node, _T("SerialNumber"), unicode, 0);
				LocalFree(unicode);

				// 0x21 Asset Tag
				unicode = GetSmbiosString(header, *(cursor + 0x21));
				node_att_set(node, _T("AssetTag"), unicode, 0);
				LocalFree(unicode);

				// 0x22 Part Number
				unicode = GetSmbiosString(header, *(cursor + 0x22));
				node_att_set(node, _T("PartNumber"), unicode, 0);
				LocalFree(unicode);
			}
		}
	}

	return procSocketsNode;
}