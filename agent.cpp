#include "stdafx.h"
#include "sysinv.h"
#include "version.h"

#define _TOWIDE(x)				L ## x
#define TOWIDE(x)				_TOWIDE(x)

PNODE GetAgentDetail()
{
	PNODE agentNode = node_alloc(_T("Agent"), 0);
	TCHAR buffer[MAX_PATH + 1];

	node_att_set(agentNode, _T("Name"), TOWIDE(VER_PRODUCTNAME_STR), 0);

	swprintf_s(buffer, L"%u.%u.%u.%u", VER_PRODUCT_VERSION);
	node_att_set(agentNode, _T("Version"), buffer, 0);
	
	/* Nope...
	swprintf_s(buffer, L"%s %s", __DATE__, __TIME__);
	node_att_set(agentNode, _T("BuildDate"), buffer, 0);
	*/

#if _DEBUG
	node_att_set(agentNode, _T("Build"), _T("Debug"), 0);
#else
	node_att_set(agentNode, _T("Build"), _T("Release"), 0);
#endif

#if _WIN64
	node_att_set(agentNode, _T("Architecture"), _T("64bit"), 0);
#else
	node_att_set(agentNode, _T("Architecture"), _T("32bit"), 0);
#endif

	return agentNode;
}