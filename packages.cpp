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

#pragma comment(lib, "Msi.lib")
#include <Msi.h>

PNODE_ATT MsiQueryProperty(PNODE node, LPTSTR key, LPCTSTR szProductCode, LPCTSTR szUserSid, MSIINSTALLCONTEXT dwContext, LPCTSTR szProperty);

PNODE EnumPackages()
{
	PNODE packages = node_alloc(L"Packages", NODE_FLAG_TABLE);
	PNODE node = NULL;
	PNODE_ATT att = NULL;
	UINT ret;
	DWORD index = 0;
	TCHAR szInstalledProductCode[39] = {0};
	TCHAR szSid[128] = {0};
	DWORD cchSid;
	MSIINSTALLCONTEXT dwInstalledContext;;

	memset(szInstalledProductCode, 0, sizeof(szInstalledProductCode));
	cchSid = sizeof(szSid) / sizeof(szSid[0]);

	while(ERROR_SUCCESS == (ret = MsiEnumProductsEx(
			NULL,
			L"s-1-0-0", // All users
			MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE,
			index++,
			szInstalledProductCode,
			&dwInstalledContext,
			szSid,
			&cchSid))) {

		node = node_alloc(_T("Package"), NODE_FLAG_TABLE_ENTRY);

		// Product Code
		node_att_set(node, _T("ProductCode"), szInstalledProductCode, NODE_ATT_FLAG_KEY);

		// Product name
		att = MsiQueryProperty(node, _T("Name"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_INSTALLEDPRODUCTNAME);

		// Publisher
		att = MsiQueryProperty(node, _T("Publisher"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_PUBLISHER);

		// Version
		att = MsiQueryProperty(node, _T("Version"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_VERSIONSTRING);
		
		// Registered Owner
		att = MsiQueryProperty(node, _T("RegisteredUser"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, _T("RegOwner"));
		
		// Registered Owner Company
		att = MsiQueryProperty(node, _T("RegisteredCompany"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, _T("RegCompany"));

		// Install Path
		att = MsiQueryProperty(node, _T("InstallPath"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_INSTALLLOCATION);

		// Install Date (Needs parsing)
		att = MsiQueryProperty(node, _T("InstallDate"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_INSTALLDATE);

		// Install Source
		att = MsiQueryProperty(node, _T("InstallSource"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_INSTALLSOURCE);

		// Local cache file
		att = MsiQueryProperty(node, _T("LocalCache"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_LOCALPACKAGE);

		// Help Link
		att = MsiQueryProperty(node, _T("HelpUrl"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_HELPLINK);

		// Help Telephone
		att = MsiQueryProperty(node, _T("HelpPhone"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_HELPTELEPHONE);

		// About URL
		att = MsiQueryProperty(node, _T("AboutUrl"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_URLINFOABOUT);

		// Update URL
		att = MsiQueryProperty(node, _T("UpdateUrl"), szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_URLUPDATEINFO);


		node_append_child(packages, node);
	}

	if (ERROR_NO_MORE_ITEMS != ret) {
		SetError(ERR_CRIT, ret, _T("Failed to enumerate installed products"));
	}

	return packages;
}

PNODE_ATT MsiQueryProperty(PNODE node, LPTSTR key, LPCTSTR szProductCode, LPCTSTR szUserSid, MSIINSTALLCONTEXT dwContext, LPCTSTR szProperty)
{
	PNODE_ATT att = NULL;
    LPTSTR value = NULL;
 
    DWORD cchValue = 0;
    UINT ret = MsiGetProductInfoEx(
        szProductCode,
        szUserSid,
        dwContext,
        szProperty,
        NULL,
        &cchValue);
 
    if(ret == ERROR_SUCCESS)
    {
        cchValue++;
        value = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR) * cchValue);
 
        ret = MsiGetProductInfoEx(
            szProductCode,
            szUserSid,
            dwContext,
            szProperty,
            (LPTSTR)&value[0],
            &cchValue);

		att = node_att_set(node, key, value, 0);

		LocalFree(value);
    }
 
    return att;
}