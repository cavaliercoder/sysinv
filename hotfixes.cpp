#include "stdafx.h"
#include <wuapi.h>
#include <iostream>
#include <ATLComTime.h>
#include <wuerror.h>

using namespace std;

// Product Behavior:	http://msdn.microsoft.com/en-us/library/ee917276.aspx
// Delphi example:		http://theroadtodelphi.wordpress.com/2011/03/02/search-for-installed-windows-updates-using-delphi-wmi-and-wua/

int HotFixTest()
{
	HRESULT hr;
	hr = CoInitialize(NULL);

	IUpdateSession* iUpdate;
	IUpdateSearcher* searcher;
	ISearchResult* results;
	BSTR criteria = SysAllocString(L"IsInstalled=1 or IsHidden=1 or IsPresent=1"); // or IsHidden=1 or IsPresent=1

	hr = CoCreateInstance(CLSID_UpdateSession, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (LPVOID*)&iUpdate);
	hr = iUpdate->CreateUpdateSearcher(&searcher);

	// Searcher options
	searcher->put_IncludePotentiallySupersededUpdates(false);
		searcher->put_Online(false);

	wcout << L"Searching for updates ..." << endl;
	hr = searcher->Search(criteria, &results);
	SysFreeString(criteria);

	switch (hr)
	{
	case S_OK:
		wcout << L"List of applicable items on the machine:" << endl;
		break;
	case WU_E_LEGACYSERVER:
		wcout << L"No server selection enabled" << endl;
		return 0;
	case WU_E_INVALID_CRITERIA:
		wcout << L"Invalid search criteria" << endl;
		return 0;
	}

	IUpdateCollection *updateList;
	IUpdate *updateItem;
	LONG updateSize;
	BSTR updateName;
	DATE retdate;

	results->get_Updates(&updateList);
	updateList->get_Count(&updateSize);

	if (updateSize == 0)
	{
		wcout << L"No updates found" << endl;
	}

	for (LONG i = 0; i < updateSize; i++)
	{
		updateList->get_Item(i, &updateItem);
		updateItem->get_Title(&updateName);
		updateItem->get_LastDeploymentChangeTime(&retdate);
		
		COleDateTime odt;
		odt.m_dt = retdate;
		wcout << i + 1 << " - " << updateName << "  Release Date " << (LPCTSTR)odt.Format(_T("%A, %B %d, %Y")) << endl;
	}
	::CoUninitialize();
	return 0;
}