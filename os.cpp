#include "stdafx.h"
#include "sysinv.h"
#include "os.h"

PNODE GetOperatingSystemNode()
{
	PNODE node = NULL;
	PNODE osNode = node_alloc(_T("OperatingSystem"), 0);

	if (NULL != (node = GetOsVersionNode()))
		node_append_child(osNode, node);
	
	if (NULL != (node = GetOsIdentityNode()))
		node_append_child(osNode, node);

	return osNode;
}

PNODE GetOsVersionNode()
{
	PNODE node = node_alloc(L"VersionInfo", 0);
	OSVERSIONINFOEX osinfo;
	TCHAR strBuffer[MAX_PATH + 1];
	HKEY hKey = 0;
	DWORD dwType = REG_SZ;
	DWORD bufferSize;

	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFOW) &osinfo);

	// Get product name from
	// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProductName
	RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey);
	RegQueryValueEx(hKey, L"ProductName", NULL, &dwType, (LPBYTE) &strBuffer, &bufferSize);
	node_att_set(node, L"Name", strBuffer, 0);

	// Break down OS name
	switch(osinfo.wProductType) {
	case VER_NT_WORKSTATION:
		switch(osinfo.dwMajorVersion) {
			case 5:
				switch(osinfo.dwMinorVersion) {
				case 0:	
					wcscpy(strBuffer, L"Windows 2000");
					break;
				case 1:
					wcscpy(strBuffer, L"Windows XP");
					break;
				}

				break;

			case 6:
				switch(osinfo.dwMinorVersion) {
				case 0:
					wcscpy(strBuffer, L"Windows Vista");
					break;

				case 1:
					wcscpy(strBuffer, L"Windows 7");
					break;

				case 2:
					wcscpy(strBuffer, L"Windows 8");
					break;

				case 3:
					wcscpy(strBuffer, L"Windows 8.1");
					break;
				}
			}
		break;

	case VER_NT_DOMAIN_CONTROLLER:
	case VER_NT_SERVER:
		switch(osinfo.dwMajorVersion) {
			case 5:
				switch(osinfo.dwMinorVersion) {
				case 0:
					wcscpy(strBuffer, L"Windows 2000 Server");
					break;
				case 1:
					wcscpy(strBuffer, L"Windows Server 2003");
					break;
				case 2:
					wcscpy(strBuffer, L"Windows Server 2003 R2");
					break;
				}
				break;

			case 6:
				switch(osinfo.dwMinorVersion) {
				case 0:
					wcscpy(strBuffer, L"Windows Server 2008");
					break;

				case 1:
					wcscpy(strBuffer, L"Windows Server 2008 R2");
					break;

				case 2:
					wcscpy(strBuffer, L"Windows Server 2012");
					break;

				case 3:
					wcscpy(strBuffer, L"Windows Server 2012 R2");
					break;
				}
		}
		break;

	default:
		wcscpy(strBuffer, L"Unknown");
		break;
	}
	node_att_set(node, L"BaseName", strBuffer, 0);

	// Determine OS Edition
	if(VER_SUITE_BLADE & osinfo.wSuiteMask)
		node_att_set(node, L"Edition", L"Web Edition", 0);

	else if(VER_SUITE_COMPUTE_SERVER & osinfo.wSuiteMask)
		node_att_set(node, L"Edition", L"Computer Cluster Edition", 0);

	else if (VER_SUITE_DATACENTER & osinfo.wSuiteMask)
		node_att_set(node, L"Edition", L"Datacenter Edition", 0);

	else if (VER_SUITE_ENTERPRISE & osinfo.wSuiteMask)
		node_att_set(node, L"Edition", L"Enterprise", 0);

	else if (VER_SUITE_EMBEDDEDNT & osinfo.wSuiteMask)
		node_att_set(node, L"Edition", L"Embedded", 0);

	else if (VER_SUITE_PERSONAL & osinfo.wSuiteMask)
		node_att_set(node, L"Edition", L"Home", 0);
	
	// OS Version numbers
	swprintf(strBuffer, L"%u.%u.%u", osinfo.dwMajorVersion, osinfo.dwMinorVersion, osinfo.dwBuildNumber);
	node_att_set(node, L"Version", strBuffer, 0);

	// Service pack
	swprintf(strBuffer, L"%u.%u", osinfo.wServicePackMajor, osinfo.wServicePackMinor);
	node_att_set(node, L"ServicePack", strBuffer, 0);

	switch(osinfo.wProductType) {
	case VER_NT_DOMAIN_CONTROLLER:
		node_att_set(node, L"Role", L"Domain Controller", 0);
		break;

	case VER_NT_SERVER:
		node_att_set(node, L"Role", L"Server", 0);
		break;

	case VER_NT_WORKSTATION:
		node_att_set(node, L"Role", L"Workstation", 0);
		break;
	}

	return node;
}

PNODE GetOsIdentityNode()
{
	PNODE identityNode = NULL;
	TCHAR *c = NULL;
	TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
	TCHAR computerAccountName[MAX_COMPUTERNAME_LENGTH + 2];
	LPTSTR domainName = NULL;
	LPTSTR szSid = NULL;
	DWORD bufferlen = MAX_COMPUTERNAME_LENGTH + 1;
	DWORD cbSid = 0;
	DWORD refDomainLen = 0;
	PSID sid = NULL;
	SID_NAME_USE sidNameUse;
	LPCTSTR pszSubKey = _T("SOFTWARE\\Microsoft\\Cryptography");
	HKEY hKey = 0;
	TCHAR machineGuid[38];
	DWORD machineGuidLen = sizeof(machineGuid);
	identityNode = node_alloc(_T("Identification"), 0);

	DWORD result = 0;

	// Get host name (for machine account)
	GetComputerName(computerName, &bufferlen);
	node_att_set(identityNode, _T("ComputerName"), computerName, 0);

	// Append '\' to build machine account name
	swprintf(computerAccountName, _T("%s\\"), computerName);

	// Get required buffer sizes
	if (! LookupAccountName(NULL, computerAccountName, NULL, &cbSid, NULL, &refDomainLen, &sidNameUse)) {
		// Allocate
		sid = (PSID) new BYTE[cbSid];
		domainName = new TCHAR[refDomainLen];

		// Get SID and domain
		if (LookupAccountName(NULL, computerAccountName, sid, &cbSid, domainName, &refDomainLen, &sidNameUse)) {
			node_att_set(identityNode, _T("Domain"), domainName, 0);

			// Convert SID to string
			if (ConvertSidToStringSid(sid, &szSid)) {
				node_att_set(identityNode, _T("MachineSid"), szSid, 0);
				LocalFree(szSid);
			}
		}
	}

	// Get Cryptography GUID
	if (ERROR_SUCCESS == (result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszSubKey, 0, KEY_READ, &hKey))){
		if (ERROR_SUCCESS == (result = RegQueryValueEx(hKey, _T("MachineGuid"), NULL, NULL, (LPBYTE)&machineGuid, &machineGuidLen))) {
			node_att_set(identityNode, _T("MachineGuid"), machineGuid, 0);
		}

		RegCloseKey(hKey);
	}
	
	else {
		SetError(ERR_WARN, GetLastError(), _T("Failed to get Machine GUID from Registry"));
	}

	return identityNode;
}