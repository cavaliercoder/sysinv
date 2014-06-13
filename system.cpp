#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

PNODE GetSystemDetail()
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

// System Info Table (7.2)
PNODE GetBiosSystemDetail()
{
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	header = GetNextStructureOfType(NULL, SMB_TABLE_SYSTEM);
	if (NULL == header)
		return node;

	cursor = (PBYTE)header;

	// v2.0+
	if (2 <= smbios->SMBIOSMajorVersion) {
		node = node_alloc(_T("SystemInfo"), 0);

		// 0x04 Manufacturer
		unicode = GetSmbiosString(header, *(cursor + 0x04));
		node_att_set(node, _T("Manufacturer"), unicode, 0);
		LocalFree(unicode);

		// 0x05 Product Name
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

		// v2.1+
		if (2 < smbios->SMBIOSMajorVersion || (2 == smbios->SMBIOSMajorVersion) && (1 <= smbios->SMBIOSMinorVersion)) {
			// 0x08 UUID Byte Array
			unicode = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)* 40);
			StringFromGUID2(*((GUID *)(cursor + 0x08)), unicode, 40);
			node_att_set(node, _T("Uuid"), unicode, 0);
			LocalFree(unicode);

			// 0x18 Wake-Up Type
			switch (*(cursor + 0x18)) {
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
		}
	}

	return node;
}
