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

#define PACKAGES_REG_PATH			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")
#define MAX_KEY_LEN					255

LPTSTR GetRegString(HKEY hKey, LPCTSTR name);

LPTSTR GetRegString(HKEY hKey, LPCTSTR name)
{
	LPTSTR pszBuffer = NULL;
	DWORD dwBufferSize = 0;
	DWORD dwRetVal = 0;

	if (ERROR_SUCCESS == (dwRetVal = RegQueryValueEx(hKey, name, 0, NULL, NULL, &dwBufferSize))) {
		if (NULL != (pszBuffer = (LPTSTR)MALLOC(dwBufferSize * 2))) {
			if (ERROR_SUCCESS != (dwRetVal = RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE) pszBuffer, &dwBufferSize))) {
				FREE(pszBuffer);
				pszBuffer = NULL;
			}
		}
	}

	return pszBuffer;
}

PNODE EnumPackages()
{
	PNODE packagesNode = node_alloc(L"Packages", NFLG_TABLE);
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

		dwBufferSize = MAX_KEY_LEN;
	}

	RegCloseKey(hKey);
	
    return packagesNode;
}