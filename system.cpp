#include "stdafx.h"
#include "sysinv.h"

PNODE GetSystemNode()
{
	TCHAR hostname[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufferSize = MAX_COMPUTERNAME_LENGTH + 1;
	SYSTEM_INFO systemInfo;

	SYSTEMTIME systemTime;
	TCHAR szSystemTime[MAX_PATH + 1];
	DWORD cursor = 0;

	PNODE node = node_alloc(L"System", 0);

	// Get host name
	GetComputerName(hostname, &bufferSize);
	node_att_set(node, L"Hostname", hostname, NODE_ATT_FLAG_KEY);

	// Get time stamp (Universal full format eg. 2009-06-15 20:45:30Z)
	GetSystemTime(&systemTime);
	if (cursor = GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &systemTime, _T("yyyy-MM-dd"), szSystemTime, MAX_PATH + 1)) {
		szSystemTime[cursor - 1] = ' ';
		if (GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &systemTime, _T("HH:mm:ssZ"), &szSystemTime[cursor], MAX_PATH + 1)) {
			node_att_set(node, _T("Timestamp"), szSystemTime, 0);
		}
	}

	// Get system info
	GetNativeSystemInfo(&systemInfo);
	
	// System architecture
	switch(systemInfo.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_INTEL:
		node_att_set(node, L"Architecture", L"x86", 0);
		break;

	case PROCESSOR_ARCHITECTURE_AMD64:
		node_att_set(node, L"Architecture", L"x86_64", 0);
		break;

	case PROCESSOR_ARCHITECTURE_ARM:
		node_att_set(node, L"Architecture", L"ARM", 0);
		break;

	case PROCESSOR_ARCHITECTURE_IA64:
		node_att_set(node, L"Architecture", L"IA64", 0);
		break;

	default:
		node_att_set(node, L"Architecture", L"Unknown", 0);
	}

	return node;
}