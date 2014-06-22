#include "stdafx.h"
#include "sysinv.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <IPHlpApi.h>

PNODE EnumIpv4Routes();

// MIB_IPFORWARDROW.dwForwardType
// http://www.ietf.org/rfc/rfc1354.txt
static LOOKUP_ENTRY FORWARD_TYPES[] = {
		{ MIB_IPROUTE_TYPE_OTHER, _T("Other"), _T("Some other type not specified in RFC 1354") },
		{ MIB_IPROUTE_TYPE_INVALID, _T("Invalid"), _T("An invalid route. This value can result from a route added by an ICMP redirect.") },
		{ MIB_IPROUTE_TYPE_DIRECT, _T("Local"), _T("A local route where the next hop is the final destination (a local interface)") },
		{ MIB_IPROUTE_TYPE_INDIRECT, _T("Remote"), _T("The remote route where the next hop is not the final destination (a remote destination)") }
};

// MIB_IPFORWARDROW.dwForwardProto
// http://www.ietf.org/rfc/rfc1354.txt
static LOOKUP_ENTRY FORWARD_PROTOCOLS[] = {
		{ MIB_IPPROTO_OTHER, _T("Other"), _T("Some other protocol not specified in RFC 1354.") },
		{ MIB_IPPROTO_LOCAL, _T("Local"), _T("A local interface.") },
		{ MIB_IPPROTO_NETMGMT, _T("Static"), _T("A static route. This value is used to identify route information for IP routing set through network management such as the Dynamic Host Configuration Protocol (DCHP), the Simple Network Management Protocol (SNMP), or by calls to the CreateIpForwardEntry, DeleteIpForwardEntry, or SetIpForwardEntry functions.") },
		{ MIB_IPPROTO_ICMP, _T("ICMP Redirect"), _T("The result of ICMP redirect.") },
		{ MIB_IPPROTO_EGP, _T("EGP"), _T("The Exterior Gateway Protocol (EGP), a dynamic routing protocol.") },
		{ MIB_IPPROTO_GGP, _T("GGP"), _T("The Gateway-to-Gateway Protocol (GGP), a dynamic routing protocol.") },
		{ MIB_IPPROTO_HELLO, _T("Hello"), _T("The Hellospeak protocol, a dynamic routing protocol. This is a historical entry no longer in use and was an early routing protocol used by the original ARPANET routers that ran special software called the Fuzzball routing protocol, sometimes called Hellospeak, as described in RFC 891 and RFC 1305. For more information, see http://www.ietf.org/rfc/rfc891.txt and http://www.ietf.org/rfc/rfc1305.txt.") },
		{ MIB_IPPROTO_RIP, _T("RIP"), _T("The Berkeley Routing Information Protocol (RIP) or RIP-II, a dynamic routing protocol.") },
		{ MIB_IPPROTO_IS_IS, _T("IS-IS"), _T("The Intermediate System-to-Intermediate System (IS-IS) protocol, a dynamic routing protocol. The IS-IS protocol was developed for use in the Open Systems Interconnection (OSI) protocol suite.") },
		{ MIB_IPPROTO_ES_IS, _T("ES-IS"), _T("The End System-to-Intermediate System (ES-IS) protocol, a dynamic routing protocol. The ES-IS protocol was developed for use in the Open Systems Interconnection (OSI) protocol suite.") },
		{ MIB_IPPROTO_CISCO, _T("IGRP"), _T("The Cisco Interior Gateway Routing Protocol (IGRP), a dynamic routing protocol.") },
		{ MIB_IPPROTO_BBN, _T("BBN SPF IGP"), _T("The Bolt, Beranek, and Newman (BBN) Interior Gateway Protocol (IGP) that used the Shortest Path First (SPF) algorithm. This was an early dynamic routing protocol.") },
		{ MIB_IPPROTO_OSPF, _T("OSPF"), _T("The Open Shortest Path First (OSPF) protocol, a dynamic routing protocol.") },
		{ MIB_IPPROTO_BGP, _T("BGP"), _T("The Border Gateway Protocol (BGP), a dynamic routing protocol.") },
		{ 0x0F, _T("IDPR"), _T("Interdomain Policy Routing") },
		{ MIB_IPPROTO_NT_AUTOSTATIC, _T(""), _T("A Windows specific entry added originally by a routing protocol, but which is now static.") },
		{ MIB_IPPROTO_NT_STATIC, _T(""), _T("A Windows specific entry added as a static route from the routing user interface or a routing command.") },
		{ MIB_IPPROTO_NT_STATIC_NON_DOD, _T(""), _T("A Windows specific entry added as a static route from the routing user interface or a routing command, except these routes do not cause Dial On Demand (DOD).") },
};

PNODE EnumNetworkRoutes()
{
	PNODE routesNode = node_alloc(_T("Routes"), NFLG_PLACEHOLDER);
	node_append_child(routesNode, EnumIpv4Routes());

	return routesNode;
}

PNODE EnumIpv4Routes()
{
	PNODE routesNode = NULL;
	PNODE routeNode = NULL;
	DWORD i = 0;
	DWORD dwRetVal = 0;
	ULONG dwBufferSize = 0;
	PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
	PMIB_IPFORWARDROW pRoute = NULL;
	TCHAR szBuffer[SZBUFFERLEN];
	PLOOKUP_ENTRY lookup = NULL;

	dwBufferSize = sizeof(MIB_IPFORWARDTABLE);
	pIpForwardTable = (PMIB_IPFORWARDTABLE)MALLOC(dwBufferSize);
	while (ERROR_INSUFFICIENT_BUFFER == (dwRetVal = GetIpForwardTable(pIpForwardTable, &dwBufferSize, 1)))
	{
		FREE(pIpForwardTable);
		pIpForwardTable = (PMIB_IPFORWARDTABLE)MALLOC(dwBufferSize);
	}

	if (NO_ERROR != dwRetVal) {
		SetError(ERR_CRIT, dwRetVal, _T("Failed to enumerate network routes"));
		return routesNode;
	}

	// Parse each route
	routesNode = node_alloc(_T("Ipv4"), NFLG_TABLE);
	for (i = 0; i < pIpForwardTable->dwNumEntries; i++) {
		pRoute = &pIpForwardTable->table[i];

		routeNode = node_append_new(routesNode, _T("Route"), NFLG_TABLE_ROW);

		// Lookup route type
		if (NULL != (lookup = Lookup(FORWARD_TYPES, pRoute->dwForwardType)))
			node_att_set(routeNode, _T("Type"), lookup->Code, 0);

		// Protocol
		if (NULL != (lookup = Lookup(FORWARD_PROTOCOLS, pRoute->dwForwardProto)))
			node_att_set(routeNode, _T("Protocol"), lookup->Code, 0);

		
		// Destination IP
		PRINTIPV4(szBuffer, pRoute->dwForwardDest);
		node_att_set(routeNode, _T("Destination"), szBuffer, NAFLG_FMT_IPADDR);

		PRINTIPV4(szBuffer, pRoute->dwForwardMask);
		node_att_set(routeNode, _T("DestinationMask"), szBuffer, NAFLG_FMT_IPADDR);
		
		// Interface index
		SWPRINTF(szBuffer, _T("%u"), pRoute->dwForwardIfIndex);
		node_att_set(routeNode, _T("InterfaceIndex"), szBuffer, NAFLG_FMT_NUMERIC);

		PRINTIPV4(szBuffer, pRoute->dwForwardNextHop);
		node_att_set(routeNode, _T("NextHop"), szBuffer, NAFLG_FMT_IPADDR);

		// Primary Metric
		SWPRINTF(szBuffer, _T("%u"), pRoute->dwForwardMetric1);
		node_att_set(routeNode, _T("Metric"), szBuffer, NAFLG_FMT_NUMERIC);
	}

	// Cleanup
	if (NULL != pIpForwardTable)
		FREE(pIpForwardTable);

	return routesNode;
}