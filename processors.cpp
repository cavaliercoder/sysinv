#include "stdafx.h"
#include "sysinv.h"
#include <intrin.h>

#define BUFLEN	128

PNODE GetProcessorsNode()
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