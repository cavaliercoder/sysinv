/*
 * Complements to:
 * http://mariusbancila.ro/blog/2011/05/01/finding-installed-applications-with-vc/
 * 
 * Potential registry locations of hotfixes per version:
 * http://support.microsoft.com/kb/184305
 * http://support.microsoft.com/kb/262841
 * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Component Based Servicing\Packages
 */
#include "stdafx.h"
#include "sysinv.h"
#include <time.h>

#define PACKAGES_REG_PATH			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall")
#define HOTFIXES_REG_PATH			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages")
#define HOTFIXES_INSTALL_CLIENT		_T("WindowsUpdateAgent")

#define DELIM_KB_REF				" identifier=\""
#define DELIM_KB_URL				" supportInformation=\""
#define DELIM_DESC					" description=\""
#define DELIM_COMPANY				" company=\""
#define DELIM_TYPE					" releaseType=\""
#define DELIM_TIME					" creationTimeStamp=\""

#define MAX_KEY_LEN					255

void EnumWin51Hotfixes(PNODE hotfixesNode);
void EnumWin6Hotfixes(PNODE hotfixesNode);

BOOL GetAttributeValue(PCHAR attribute, PCHAR szBuffer, DWORD dwBufferSize);

PNODE EnumPackages()
{
	PNODE packagesNode = NULL;
	PNODE packageNode = NULL;
	HKEY hKey = NULL;
	HKEY hSubkey = NULL;
	DWORD dwRetVal = 0;
	DWORD dwIndex = 0;
	DWORD dwBufferSize = MAX_KEY_LEN;
	TCHAR szBuffer[MAX_KEY_LEN];
	LPTSTR pszBuffer = NULL;

	// Get handle to packages registry key
	// We could use MSI API but it won't return packages built 
	// with Windows Installer versions prior to v3.0.
	if (ERROR_SUCCESS != (dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PACKAGES_REG_PATH, 0, KEY_READ, &hKey))) {
		SetError(ERR_CRIT, dwRetVal, _T("Failed to enumerate installed packages"));
		return packagesNode;
	}

	packagesNode = node_alloc(_T("Packages"), NFLG_TABLE);
	
	// Get all subkeys (one for each package)
	dwIndex = 0;
	while (ERROR_SUCCESS == (dwRetVal = RegEnumKeyEx(hKey, dwIndex++, szBuffer, &dwBufferSize, 0, NULL, NULL, NULL))) {
		// Prevent KBXXXXXXXX packages from being read on Win2003/XP systems
		// These are enumerated as Hotfixes
		if (0 == wcsncmp(szBuffer, _T("KB"), 2))
			goto cleanup_package;

		if (ERROR_SUCCESS == (dwRetVal = RegOpenKeyEx(hKey, szBuffer, 0, KEY_READ, &hSubkey))) {

			// Get value of 'DisplayName' value
			if (NULL != (pszBuffer = GetRegString(hSubkey, _T("DisplayName")))) {
				packageNode = node_append_new(packagesNode, _T("Package"), NFLG_TABLE_ROW);
				node_att_set(packageNode, _T("Name"), pszBuffer, 0);
				FREE(pszBuffer);

				// Add GUID for Windows Installer v3+ Packages (GUID key name)
				if (38 == wcslen(szBuffer) && '{' == szBuffer[0] && '}' == szBuffer[37])
					node_att_set(packageNode, _T("UpgradeCode"), szBuffer, NAFLG_FMT_GUID);

				// Publisher
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("Publisher")))) {
					node_att_set(packageNode, _T("Publisher"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// Version
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("DisplayVersion")))) {
					node_att_set(packageNode, _T("Version"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// Install source
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("InstallSource")))) {
					node_att_set(packageNode, _T("InstallSource"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// Install path
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("InstallLocation")))) {
					node_att_set(packageNode, _T("InstallLocation"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// Install date
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("InstallDate")))) {
					node_att_set(packageNode, _T("InstallDate"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// Help URL
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("HelpLink")))) {
					node_att_set(packageNode, _T("HelpUrl"), pszBuffer, NAFLG_FMT_URI);
					FREE(pszBuffer);
				}

				// Help telephone
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("HelpTelephone")))) {
					node_att_set(packageNode, _T("HelpTelephone"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// About URL
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("URLInfoAbout")))) {
					node_att_set(packageNode, _T("AboutUrl"), pszBuffer, NAFLG_FMT_URI);
					FREE(pszBuffer);
				}

				// Upgrade URL
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("URLUpdateInfo")))) {
					node_att_set(packageNode, _T("UpdateUrl"), pszBuffer, NAFLG_FMT_URI);
					FREE(pszBuffer);
				}
			}

			RegCloseKey(hSubkey);
		}

	cleanup_package:

		dwBufferSize = MAX_KEY_LEN;
	}

	RegCloseKey(hKey);
	
    return packagesNode;
}

/*
 * CBS Store details:
 * http://technet.microsoft.com/en-us/library/ee619779(v=ws.10).aspx
 *
 * Files: C:\Windows\servicing\packages\Package_for_KB*.mum
 */
PNODE EnumHotfixes()
{
	PNODE hotfixesNode = NULL;
	hotfixesNode = node_alloc(_T("Hotfixes"), NFLG_TABLE);

	EnumWin6Hotfixes(hotfixesNode);
	EnumWin51Hotfixes(hotfixesNode);

	return hotfixesNode;
}


void EnumWin51Hotfixes(PNODE hotfixesNode)
{
	PNODE hotfixNode = NULL;
	HKEY hKey = NULL;
	HKEY hSubkey = NULL;
	DWORD dwRetVal = 0;
	DWORD dwIndex = 0;
	DWORD dwBufferSize = MAX_KEY_LEN;
	TCHAR szBuffer[MAX_KEY_LEN];
	LPTSTR pszBuffer = NULL;

	// Get handle to packages registry key
	// We could use MSI API but it won't return packages built 
	// with Windows Installer versions prior to v3.0.
	if (ERROR_SUCCESS != (dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PACKAGES_REG_PATH, 0, KEY_READ, &hKey))) {
		SetError(ERR_CRIT, dwRetVal, _T("Failed to enumerate installed packages"));
		return;
	}

	// Get all subkeys (one for each package)
	dwIndex = 0;
	while (ERROR_SUCCESS == (dwRetVal = RegEnumKeyEx(hKey, dwIndex++, szBuffer, &dwBufferSize, 0, NULL, NULL, NULL))) {
		// Only read KBXXXXXXXX registry keys
		if (0 == wcsncmp(szBuffer, _T("KB"), 2) && ERROR_SUCCESS == (dwRetVal = RegOpenKeyEx(hKey, szBuffer, 0, KEY_READ, &hSubkey))) {
			// Get value of 'DisplayName' value
			if (NULL != (pszBuffer = GetRegString(hSubkey, _T("DisplayName")))) {
				// Create hotfix node!
				hotfixNode = node_alloc(_T("Hotfix"), NFLG_TABLE_ROW);

				// KB Reference
				node_att_set(hotfixNode, _T("Id"), szBuffer, NAFLG_KEY);

				// Display name
				hotfixNode = node_append_new(hotfixesNode, _T("Hotfix"), NFLG_TABLE_ROW);
				node_att_set(hotfixNode, _T("Name"), pszBuffer, NAFLG_KEY);
				FREE(pszBuffer);

				// Release type
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("ReleaseType")))) {
					node_att_set(hotfixNode, _T("Category"), pszBuffer, 0);
					FREE(pszBuffer);
				}

				// URL
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("HelpLink")))) {
					node_att_set(hotfixNode, _T("HelpUrl"), pszBuffer, NAFLG_FMT_URI);
					FREE(pszBuffer);
				}

				// Install date
				// TODO: Parse Install Date for a useful format
				if (NULL != (pszBuffer = GetRegString(hSubkey, _T("InstallDate")))) {
					node_att_set(hotfixNode, _T("InstallDate"), pszBuffer, NAFLG_FMT_DATETIME);
					FREE(pszBuffer);
				}
			}

			RegCloseKey(hSubkey);
		}

	cleanup_package:

		dwBufferSize = MAX_KEY_LEN;
	}

	RegCloseKey(hKey);
}

void EnumWin6Hotfixes(PNODE hotfixesNode)
{
	PNODE hotfixNode = NULL;
	HKEY hKey = NULL;
	HKEY hSubkey = NULL;
	DWORD dwRetVal = 0;
	DWORD dwIndex = 0;
	DWORD dwBufferSize = MAX_KEY_LEN;
	TCHAR szBuffer[MAX_KEY_LEN];
	TCHAR szPathBuffer[MAX_PATH + 1];
	TCHAR szPathBuffer2[MAX_PATH + 1];
	CHAR szFileBuffer[4096];
	CHAR szValueBuffer[4096];
	PCHAR c1 = NULL;
	LPTSTR pszBuffer = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD installTimeHigh, installTimeLow;
	FILETIME ftInstallTime;
	SYSTEMTIME stInstallTime;

	// Get handle to hotfixes registry key
	// We could use MSI API but it won't return hotfixes built 
	// with Windows Installer versions prior to v3.0.
	if (ERROR_SUCCESS != (dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, HOTFIXES_REG_PATH, 0, KEY_READ, &hKey))) {
		SetError(ERR_CRIT, dwRetVal, _T("Failed to enumerate installed hotfixes"));
		return;
	}

	// Get all subkeys (one for each hotfix)
	dwIndex = 0;
	while (ERROR_SUCCESS == (dwRetVal = RegEnumKeyEx(hKey, dwIndex++, szBuffer, &dwBufferSize, 0, NULL, NULL, NULL))) {
		// Get handle to subkey
		if (ERROR_SUCCESS == (dwRetVal = RegOpenKeyEx(hKey, szBuffer, 0, KEY_READ, &hSubkey))) {
			// Make sure key starts with 'Package_for_KB'
			if (0 != wcsncmp(szBuffer, _T("Package_for_KB"), 14))
				goto cleanup_key;

			// Get package installer
			if (NULL == (pszBuffer = GetRegString(hSubkey, _T("InstallClient"))))
				goto cleanup_key;

			// Ensure InstallClient == "WindowsUpdateAgent"
			if (0 != wcsicmp(pszBuffer, HOTFIXES_INSTALL_CLIENT)) {
				FREE(pszBuffer);
				goto cleanup_key;
			}

			// Ensure InstallName == "update.mum"
			if (NULL != (pszBuffer = GetRegString(hSubkey, _T("InstallName")))) {
				if (0 != wcsicmp(pszBuffer, _T("update.mum"))) {
					FREE(pszBuffer);
					goto cleanup_key;
				}
			}
			
			// Free buffer allocated for InstallClient
			FREE(pszBuffer);

			// Check for corresponding .mum file at C:\Windows\servicing\Packages\Package_for_KB*.mum
			SWPRINTF(szPathBuffer2, _T("%%SystemRoot%%\\servicing\\Packages\\%s.mum"), szBuffer);
			ExpandEnvironmentStrings(szPathBuffer2, szPathBuffer, MAX_PATH + 1);

			// Get a file handle to the package .mum file
			if (INVALID_HANDLE_VALUE == (hFile = CreateFile(szPathBuffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
				goto cleanup_key;

			// Create a node
			hotfixNode = node_append_new(hotfixesNode, _T("Hotfix"), NFLG_TABLE_ROW);

			// Install date
			if (-1 != (installTimeHigh = GetRegDword(hSubkey, _T("InstallTimeHigh"))) && -1 != (installTimeLow = GetRegDword(hSubkey, _T("InstallTimeLow")))) {
				if (FormatDateTime(installTimeHigh, installTimeLow, szBuffer, ARRAYSIZE(szBuffer))) {
					node_att_set(hotfixNode, _T("InstallDate"), szBuffer, NAFLG_FMT_DATETIME);
				}
			}

			// DEBUG: CurrentState should be 0x70???
			if (-1 != (dwRetVal = GetRegDword(hSubkey, _T("CurrentState")))) {
				SWPRINTF(szBuffer, L"0x%X (%u)", dwRetVal, dwRetVal);
				node_att_set(hotfixNode, L"State", szBuffer, 0);
			}

			// Parse the first chunk of the file for hostfix info
			if (ReadFile(hFile, szFileBuffer, ARRAYSIZE(szFileBuffer), &dwRetVal, NULL) && dwRetVal) {
				// KB Reference
				if (NULL != (c1 = strstr(szFileBuffer, DELIM_KB_REF))) {
					if (GetAttributeValue(c1, szValueBuffer, ARRAYSIZE(szValueBuffer))) {
						UTF8_TO_UNICODE(szValueBuffer, szBuffer, ARRAYSIZE(szBuffer));
						node_att_set(hotfixNode, _T("Id"), szBuffer, NAFLG_KEY);
					}
				}

				// Type
				if (NULL != (c1 = strstr(szFileBuffer, DELIM_TYPE))) {
					if (GetAttributeValue(c1, szValueBuffer, ARRAYSIZE(szValueBuffer))) {
						UTF8_TO_UNICODE(szValueBuffer, szBuffer, ARRAYSIZE(szBuffer));
						node_att_set(hotfixNode, _T("Category"), szBuffer, 0);
					}
				}

				// Description
				if (NULL != (c1 = strstr(szFileBuffer, DELIM_DESC))) {
					if (GetAttributeValue(c1, szValueBuffer, ARRAYSIZE(szValueBuffer))) {
						UTF8_TO_UNICODE(szValueBuffer, szBuffer, ARRAYSIZE(szBuffer));
						node_att_set(hotfixNode, _T("Name"), szBuffer, 0);
					}
				}

				// Company
				if (NULL != (c1 = strstr(szFileBuffer, DELIM_COMPANY))) {
					if (GetAttributeValue(c1, szValueBuffer, ARRAYSIZE(szValueBuffer))) {
						UTF8_TO_UNICODE(szValueBuffer, szBuffer, ARRAYSIZE(szBuffer));
						node_att_set(hotfixNode, _T("Publisher"), szBuffer, 0);
					}
				}

				// Get KB URL
				if (NULL != (c1 = strstr(szFileBuffer, DELIM_KB_URL))) {
					if (GetAttributeValue(c1, szValueBuffer, ARRAYSIZE(szValueBuffer))) {
						UTF8_TO_UNICODE(szValueBuffer, szBuffer, ARRAYSIZE(szBuffer));
						node_att_set(hotfixNode, _T("HelpUrl"), szBuffer, NAFLG_FMT_URI);
					}
				}
			} 
			CloseHandle(hFile);
		}

	cleanup_key:

		RegCloseKey(hSubkey);
		dwBufferSize = MAX_KEY_LEN;
	}

	RegCloseKey(hKey);
}

/*
 * This function is not bullet proof but should be resilient in the real world
 * as the .mum files are machine generated.
 */
BOOL GetAttributeValue(PCHAR attribute, PCHAR szBuffer, DWORD dwBufferSize)
{
	PCHAR in = attribute;
	DWORD i = 0;
	// Seek to the beginning of the attribute value (eg. key="value")
	//                                                      ^
	while ((*in) != '=') {
		if ((*in) == '\0')
			return 0;

		in++;
	}

	// Seek to the beginning of the value (eg. key = "value")
	//                                               ^
	while ((*in) != '"') {
		if ((*in) == '\0')
			return 0;
		in++;
	}

	// Start copying to the output buffer
	in++;
	for (i = 0; i < dwBufferSize; i++, in++) {
		if ((*in) == '\0')
			return 0;

		// Have we reached the end of a value quote?
		if ((*in) == '"') {
			szBuffer[i] = '\0';
			return i;
		}

		szBuffer[i] = (*in);
	}

	return 0;
}