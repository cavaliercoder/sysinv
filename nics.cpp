#include "stdafx.h"
#include "sysinv.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <IPHlpApi.h>

#define BUFFER_SIZE			16384

// http://www.iana.org/assignments/ianaiftype-mib/ianaiftype-mib
static LOOKUP_ENTRY IF_TYPES[] = {
		{ 1, _T("other"), _T("Other") },
		{ 2, _T("regular1822"), _T("Regular 1822") },
		{ 3, _T("hdh1822"), _T("HDH1822") },
		{ 4, _T("ddnX25"), _T("DDNX25") },
		{ 5, _T("rfc877x25"), _T("RFC877x25") },
		{ 6, _T("ethernetCsmacd"), _T("Ethernet") },
		{ 7, _T("iso88023Csmacd"), _T("Deprecated via RFC3635") },
		{ 8, _T("iso88024TokenBus"), _T("Token Bus") },
		{ 9, _T("iso88025TokenRing"), _T("Token Ring") },
		{ 10, _T("iso88026Man"), _T("") },
		{ 11, _T("starLan"), _T("Deprecated via RFC3635") },
		{ 12, _T("proteon10Mbit"), _T("Proteon 10Mbit") },
		{ 13, _T("proteon80Mbit"), _T("Proteon 80Mbit") },
		{ 14, _T("hyperchannel"), _T("HyperChanne;") },
		{ 15, _T("fddi"), _T("") },
		{ 16, _T("lapb"), _T("") },
		{ 17, _T("sdlc"), _T("") },
		{ 18, _T("ds1"), _T("DS1-MIB") },
		{ 19, _T("e1"), _T("E1") },
		{ 20, _T("basicISDN"), _T("Basic ISDN") },
		{ 21, _T("primaryISDN"), _T("Primary ISDN") },
		{ 22, _T("propPointToPointSerial"), _T("Proprietary PPP Serial") },
		{ 23, _T("ppp"), _T("PPP") },
		{ 24, _T("softwareLoopback"), _T("Loopback") },
		{ 25, _T("eon"), _T("CLNP over IP") },
		{ 26, _T("ethernet3Mbit"), _T("") },
		{ 27, _T("nsip"), _T("XNS over IP") },
		{ 28, _T("slip"), _T("generic SLIP") },
		{ 29, _T("ultra"), _T("ULTRA technologies") },
		{ 30, _T("ds3"), _T("DS3-MIB") },
		{ 31, _T("sip"), _T("SMDS, coffee") },
		{ 32, _T("frameRelay"), _T("DTE only.") },
		{ 33, _T("rs232"), _T("") },
		{ 34, _T("para"), _T("parallel-port") },
		{ 35, _T("arcnet"), _T("arcnet") },
		{ 36, _T("arcnetPlus"), _T("arcnet plus") },
		{ 37, _T("atm"), _T("ATM cells") },
		{ 38, _T("miox25"), _T("") },
		{ 39, _T("sonet"), _T("SONET or SDH") },
		{ 40, _T("x25ple"), _T("") },
		{ 41, _T("iso88022llc"), _T("") },
		{ 42, _T("localTalk"), _T("") },
		{ 43, _T("smdsDxi"), _T("") },
		{ 44, _T("frameRelayService"), _T("FRNETSERV-MIB") },
		{ 45, _T("v35"), _T("") },
		{ 46, _T("hssi"), _T("") },
		{ 47, _T("hippi"), _T("") },
		{ 48, _T("modem"), _T("Generic modem") },
		{ 49, _T("aal5"), _T("AAL5 over ATM") },
		{ 50, _T("sonetPath"), _T("") },
		{ 51, _T("sonetVT"), _T("") },
		{ 52, _T("smdsIcip"), _T("SMDS InterCarrier Interface") },
		{ 53, _T("propVirtual"), _T("proprietary virtual/internal") },
		{ 54, _T("propMultiplexor"), _T("proprietary multiplexing") },
		{ 55, _T("ieee80212"), _T("100BaseVG") },
		{ 56, _T("fibreChannel"), _T("Fibre Channel") },
		{ 57, _T("hippiInterface"), _T("HIPPI interfaces") },
		{ 58, _T("frameRelayInterconnect"), _T("Obsolete, use either") },
		{ 59, _T("aflane8023"), _T("ATM Emulated LAN for 802.3") },
		{ 60, _T("aflane8025"), _T("ATM Emulated LAN for 802.5") },
		{ 61, _T("cctEmul"), _T("ATM Emulated circuit") },
		{ 62, _T("fastEther"), _T("Obsoleted via RFC3635") },
		{ 63, _T("isdn"), _T("ISDN and X.25") },
		{ 64, _T("v11"), _T("CCITT V.11/X.21") },
		{ 65, _T("v36"), _T("CCITT V.36") },
		{ 66, _T("g703at64k"), _T("CCITT G703 at 64Kbps") },
		{ 67, _T("g703at2mb"), _T("Obsolete see DS1-MIB") },
		{ 68, _T("qllc"), _T("SNA QLLC") },
		{ 69, _T("fastEtherFX"), _T("Obsoleted via RFC3635") },
		{ 70, _T("channel"), _T("channel") },
		{ 71, _T("ieee80211"), _T("802.11 Wireless") },
		{ 72, _T("ibm370parChan"), _T("IBM System 360/370 OEMI Channel") },
		{ 73, _T("escon"), _T("IBM Enterprise Systems Connection") },
		{ 74, _T("dlsw"), _T("Data Link Switching") },
		{ 75, _T("isdns"), _T("ISDN S/T interface") },
		{ 76, _T("isdnu"), _T("ISDN U interface") },
		{ 77, _T("lapd"), _T("Link Access Protocol D") },
		{ 78, _T("ipSwitch"), _T("IP Switching Objects") },
		{ 79, _T("rsrb"), _T("Remote Source Route Bridging") },
		{ 80, _T("atmLogical"), _T("ATM Logical Port") },
		{ 81, _T("ds0"), _T("Digital Signal Level 0") },
		{ 82, _T("ds0Bundle"), _T("group of ds0s on the same ds1") },
		{ 83, _T("bsc"), _T("Bisynchronous Protocol") },
		{ 84, _T("async"), _T("Asynchronous Protocol") },
		{ 85, _T("cnr"), _T("Combat Net Radio") },
		{ 86, _T("iso88025Dtr"), _T("ISO 802.5r DTR") },
		{ 87, _T("eplrs"), _T("Ext Pos Loc Report Sys") },
		{ 88, _T("arap"), _T("Appletalk Remote Access Protocol") },
		{ 89, _T("propCnls"), _T("Proprietary Connectionless Protocol") },
		{ 90, _T("hostPad"), _T("CCITT-ITU X.29 PAD Protocol") },
		{ 91, _T("termPad"), _T("CCITT-ITU X.3 PAD Facility") },
		{ 92, _T("frameRelayMPI"), _T("Multiproto Interconnect over FR") },
		{ 93, _T("x213"), _T("CCITT-ITU X213") },
		{ 94, _T("adsl"), _T("Asymmetric Digital Subscriber Loop") },
		{ 95, _T("radsl"), _T("Rate-Adapt. Digital Subscriber Loop") },
		{ 96, _T("sdsl"), _T("Symmetric Digital Subscriber Loop") },
		{ 97, _T("vdsl"), _T("Very H-Speed Digital Subscrib. Loop") },
		{ 98, _T("iso88025CRFPInt"), _T("ISO 802.5 CRFP") },
		{ 99, _T("myrinet"), _T("Myricom Myrinet") },
		{ 100, _T("voiceEM"), _T("voice recEive and transMit") },
		{ 101, _T("voiceFXO"), _T("voice Foreign Exchange Office") },
		{ 102, _T("voiceFXS"), _T("voice Foreign Exchange Station") },
		{ 103, _T("voiceEncap"), _T("voice encapsulation") },
		{ 104, _T("voiceOverIp"), _T("voice over IP encapsulation") },
		{ 105, _T("atmDxi"), _T("ATM DXI") },
		{ 106, _T("atmFuni"), _T("ATM FUNI") },
		{ 107, _T("atmIma"), _T("ATM IMA") },
		{ 108, _T("pppMultilinkBundle"), _T("PPP Multilink Bundle") },
		{ 109, _T("ipOverCdlc"), _T("IBM ipOverCdlc") },
		{ 110, _T("ipOverClaw"), _T("IBM Common Link Access to Workstn") },
		{ 111, _T("stackToStack"), _T("IBM stackToStack") },
		{ 112, _T("virtualIpAddress"), _T("IBM VIPA") },
		{ 113, _T("mpc"), _T("IBM multi-protocol channel support") },
		{ 114, _T("ipOverAtm"), _T("IBM ipOverAtm") },
		{ 115, _T("iso88025Fiber"), _T("ISO 802.5j Fiber Token Ring") },
		{ 116, _T("tdlc"), _T("IBM twinaxial data link control") },
		{ 117, _T("gigabitEthernet"), _T("Obsoleted via RFC3635") },
		{ 118, _T("hdlc"), _T("HDLC") },
		{ 119, _T("lapf"), _T("LAP F") },
		{ 120, _T("v37"), _T("V.37") },
		{ 121, _T("x25mlp"), _T("Multi-Link Protocol") },
		{ 122, _T("x25huntGroup"), _T("X25 Hunt Group") },
		{ 123, _T("transpHdlc"), _T("Transp HDLC") },
		{ 124, _T("interleave"), _T("Interleave channel") },
		{ 125, _T("fast"), _T("Fast channel") },
		{ 126, _T("ip"), _T("IP (for APPN HPR in IP networks)") },
		{ 127, _T("docsCableMaclayer"), _T("CATV Mac Layer") },
		{ 128, _T("docsCableDownstream"), _T("CATV Downstream interface") },
		{ 129, _T("docsCableUpstream"), _T("CATV Upstream interface") },
		{ 130, _T("a12MppSwitch"), _T("Avalon Parallel Processor") },
		{ 131, _T("tunnel"), _T("Tunnel Encapsulation") },
		{ 132, _T("coffee"), _T("coffee pot") },
		{ 133, _T("ces"), _T("Circuit Emulation Service") },
		{ 134, _T("atmSubInterface"), _T("ATM Sub Interface") },
		{ 135, _T("l2vlan"), _T("Layer 2 Virtual LAN using 802.1Q") },
		{ 136, _T("l3ipvlan"), _T("Layer 3 Virtual LAN using IP") },
		{ 137, _T("l3ipxvlan"), _T("Layer 3 Virtual LAN using IPX") },
		{ 138, _T("digitalPowerline"), _T("IP over Power Lines") },
		{ 139, _T("mediaMailOverIp"), _T("Multimedia Mail over IP") },
		{ 140, _T("dtm"), _T("Dynamic syncronous Transfer Mode") },
		{ 141, _T("dcn"), _T("Data Communications Network") },
		{ 142, _T("ipForward"), _T("IP Forwarding Interface") },
		{ 143, _T("msdsl"), _T("Multi-rate Symmetric DSL") },
		{ 144, _T("ieee1394"), _T("IEEE1394 High Performance Serial Bus") },
		{ 145, _T("if-gsn"), _T("  HIPPI-6400") },
		{ 146, _T("dvbRccMacLayer"), _T("DVB-RCC MAC Layer") },
		{ 147, _T("dvbRccDownstream"), _T("DVB-RCC Downstream Channel") },
		{ 148, _T("dvbRccUpstream"), _T("DVB-RCC Upstream Channel") },
		{ 149, _T("atmVirtual"), _T("ATM Virtual Interface") },
		{ 150, _T("mplsTunnel"), _T("MPLS Tunnel Virtual Interface") },
		{ 151, _T("srp"), _T("Spatial Reuse Protocol") },
		{ 152, _T("voiceOverAtm"), _T("Voice Over ATM") },
		{ 153, _T("voiceOverFrameRelay"), _T("Voice Over Frame Relay") },
		{ 154, _T("idsl"), _T("Digital Subscriber Loop over ISDN") },
		{ 155, _T("compositeLink"), _T("Avici Composite Link Interface") },
		{ 156, _T("ss7SigLink"), _T("SS7 Signaling Link") },
		{ 157, _T("propWirelessP2P"), _T(" Prop. P2P wireless interface") },
		{ 158, _T("frForward"), _T("Frame Forward Interface") },
		{ 159, _T("rfc1483"), _T("Multiprotocol over ATM AAL5") },
		{ 160, _T("usb"), _T("USB Interface") },
		{ 161, _T("ieee8023adLag"), _T("IEEE 802.3ad Link Aggregate") },
		{ 162, _T("bgppolicyaccounting"), _T("BGP Policy Accounting") },
		{ 163, _T("frf16MfrBundle"), _T("FRF .16 Multilink Frame Relay") },
		{ 164, _T("h323Gatekeeper"), _T("H323 Gatekeeper") },
		{ 165, _T("h323Proxy"), _T("H323 Voice and Video Proxy") },
		{ 166, _T("mpls"), _T("MPLS") },
		{ 167, _T("mfSigLink"), _T("Multi-frequency signaling link") },
		{ 168, _T("hdsl2"), _T("High Bit-Rate DSL - 2nd generation") },
		{ 169, _T("shdsl"), _T("Multirate HDSL2") },
		{ 170, _T("ds1FDL"), _T("Facility Data Link 4Kbps on a DS1") },
		{ 171, _T("pos"), _T("Packet over SONET/SDH Interface") },
		{ 172, _T("dvbAsiIn"), _T("DVB-ASI Input") },
		{ 173, _T("dvbAsiOut"), _T("DVB-ASI Output") },
		{ 174, _T("plc"), _T("Power Line Communtications") },
		{ 175, _T("nfas"), _T("Non Facility Associated Signaling") },
		{ 176, _T("tr008"), _T("TR008") },
		{ 177, _T("gr303RDT"), _T("Remote Digital Terminal") },
		{ 178, _T("gr303IDT"), _T("Integrated Digital Terminal") },
		{ 179, _T("isup"), _T("ISUP") },
		{ 180, _T("propDocsWirelessMaclayer"), _T("Cisco proprietary Maclayer") },
		{ 181, _T("propDocsWirelessDownstream"), _T("Cisco proprietary Downstream") },
		{ 182, _T("propDocsWirelessUpstream"), _T("Cisco proprietary Upstream") },
		{ 183, _T("hiperlan2"), _T("HIPERLAN Type 2 Radio Interface") },
		{ 184, _T("propBWAp2Mp"), _T("PropBroadbandWirelessAccesspt2multipt") },
		{ 185, _T("sonetOverheadChannel"), _T("SONET Overhead Channel") },
		{ 186, _T("digitalWrapperOverheadChannel"), _T("Digital Wrapper") },
		{ 187, _T("aal2"), _T("ATM adaptation layer 2") },
		{ 188, _T("radioMAC"), _T("MAC layer over radio links") },
		{ 189, _T("atmRadio"), _T("ATM over radio links") },
		{ 190, _T("imt"), _T("Inter Machine Trunks") },
		{ 191, _T("mvl"), _T("Multiple Virtual Lines DSL") },
		{ 192, _T("reachDSL"), _T("Long Reach DSL") },
		{ 193, _T("frDlciEndPt"), _T("Frame Relay DLCI End Point") },
		{ 194, _T("atmVciEndPt"), _T("ATM VCI End Point") },
		{ 195, _T("opticalChannel"), _T("Optical Channel") },
		{ 196, _T("opticalTransport"), _T("Optical Transport") },
		{ 197, _T("propAtm"), _T(" Proprietary ATM") },
		{ 198, _T("voiceOverCable"), _T("Voice Over Cable Interface") },
		{ 199, _T("infiniband"), _T("Infiniband") },
		{ 200, _T("teLink"), _T("TE Link") },
		{ 201, _T("q2931"), _T("Q.2931") },
		{ 202, _T("virtualTg"), _T("Virtual Trunk Group") },
		{ 203, _T("sipTg"), _T("SIP Trunk Group") },
		{ 204, _T("sipSig"), _T("SIP Signaling") },
		{ 205, _T("docsCableUpstreamChannel"), _T("CATV Upstream Channel") },
		{ 206, _T("econet"), _T("Acorn Econet") },
		{ 207, _T("pon155"), _T("FSAN 155Mb Symetrical PON interface") },
		{ 208, _T("pon622"), _T("FSAN622Mb Symetrical PON interface") },
		{ 209, _T("bridge"), _T("Transparent bridge interface") },
		{ 210, _T("linegroup"), _T("Interface common to multiple lines") },
		{ 211, _T("voiceEMFGD"), _T("voice E&M Feature Group D") },
		{ 212, _T("voiceFGDEANA"), _T("voice FGD Exchange Access North American") },
		{ 213, _T("voiceDID"), _T("voice Direct Inward Dialing") },
		{ 214, _T("mpegTransport"), _T("MPEG transport interface") },
		{ 215, _T("sixToFour"), _T("6to4 interface (DEPRECATED)") },
		{ 216, _T("gtp"), _T("GTP (GPRS Tunneling Protocol)") },
		{ 217, _T("pdnEtherLoop1"), _T("Paradyne EtherLoop 1") },
		{ 218, _T("pdnEtherLoop2"), _T("Paradyne EtherLoop 2") },
		{ 219, _T("opticalChannelGroup"), _T("Optical Channel Group") },
		{ 220, _T("homepna"), _T("HomePNA ITU-T G.989") },
		{ 221, _T("gfp"), _T("Generic Framing Procedure (GFP)") },
		{ 222, _T("ciscoISLvlan"), _T("Layer 2 Virtual LAN using Cisco ISL") },
		{ 223, _T("actelisMetaLOOP"), _T("Acteleis proprietary MetaLOOP High Speed Link") },
		{ 224, _T("fcipLink"), _T("FCIP Link") },
		{ 225, _T("rpr"), _T("Resilient Packet Ring Interface Type") },
		{ 226, _T("qam"), _T("RF Qam Interface") },
		{ 227, _T("lmp"), _T("Link Management Protocol") },
		{ 228, _T("cblVectaStar"), _T("Cambridge Broadband Networks Limited VectaStar") },
		{ 229, _T("docsCableMCmtsDownstream"), _T("CATV Modular CMTS Downstream Interface") },
		{ 230, _T("adsl2"), _T("Asymmetric Digital Subscriber Loop Version 2") },
		{ 231, _T("macSecControlledIF"), _T("MACSecControlled") },
		{ 232, _T("macSecUncontrolledIF"), _T("MACSecUncontrolled") },
		{ 233, _T("aviciOpticalEther"), _T("Avici Optical Ethernet Aggregate") },
		{ 234, _T("atmbond"), _T("atmbond") },
		{ 235, _T("voiceFGDOS"), _T("voice FGD Operator Services") },
		{ 236, _T("mocaVersion1"), _T("MultiMedia over Coax Alliance (MoCA) Interface") },
		{ 237, _T("ieee80216WMAN"), _T("IEEE 802.16 WMAN interface") },
		{ 238, _T("adsl2plus"), _T("Asymmetric Digital Subscriber Loop Version 2,") },
		{ 239, _T("dvbRcsMacLayer"), _T("DVB-RCS MAC Layer") },
		{ 240, _T("dvbTdm"), _T("DVB Satellite TDM") },
		{ 241, _T("dvbRcsTdma"), _T("DVB-RCS TDMA") },
		{ 242, _T("x86Laps"), _T("LAPS based on ITU-T X.86/Y.1323") },
		{ 243, _T("wwanPP"), _T("3GPP WWAN") },
		{ 244, _T("wwanPP2"), _T("3GPP2 WWAN") },
		{ 245, _T("voiceEBS"), _T("voice P-phone EBS physical interface") },
		{ 246, _T("ifPwType"), _T("Pseudowire interface type") },
		{ 247, _T("ilan"), _T("Internal LAN on a bridge per IEEE 802.1ap") },
		{ 248, _T("pip"), _T("Provider Instance Port on a bridge per IEEE 802.1ah PBB") },
		{ 249, _T("aluELP"), _T("Alcatel-Lucent Ethernet Link Protection") },
		{ 250, _T("gpon"), _T("Gigabit-capable passive optical networks (G-PON) as per ITU-T G.948") },
		{ 251, _T("vdsl2"), _T("Very high speed digital subscriber line Version 2 (as per ITU-T Recommendation G.993.2)") },
		{ 252, _T("capwapDot11Profile"), _T("WLAN Profile Interface") },
		{ 253, _T("capwapDot11Bss"), _T("WLAN BSS Interface") },
		{ 254, _T("capwapWtpVirtualRadio"), _T("WTP Virtual Radio Interface") },
		{ 255, _T("bits"), _T("bitsport") },
		{ 256, _T("docsCableUpstreamRfPort"), _T("DOCSIS CATV Upstream RF Port") },
		{ 257, _T("cableDownstreamRfPort"), _T("CATV downstream RF port") },
		{ 258, _T("vmwareVirtualNic"), _T("VMware Virtual Network Interface") },
		{ 259, _T("ieee802154"), _T("IEEE 802.15.4 WPAN interface") },
		{ 260, _T("otnOdu"), _T("OTN Optical Data Unit") },
		{ 261, _T("otnOtu"), _T("OTN Optical channel Transport Unit") },
		{ 262, _T("ifVfiType"), _T("VPLS Forwarding Instance Interface Type") },
		{ 263, _T("g9981"), _T("G.998.1 bonded interface") },
		{ 264, _T("g9982"), _T("G.998.2 bonded interface") },
		{ 265, _T("g9983"), _T("G.998.3 bonded interface") },
		{ 266, _T("aluEpon"), _T("Ethernet Passive Optical Networks (E-PON)") },
		{ 267, _T("aluEponOnu"), _T("EPON Optical Network Unit") },
		{ 268, _T("aluEponPhysicalUni"), _T("EPON physical User to Network interface") },
		{ 269, _T("aluEponLogicalLink"), _T("The emulation of a point-to-point link over the EPON layer") },
		{ 270, _T("aluGponOnu"), _T("GPON Optical Network Unit") },
		{ 271, _T("aluGponPhysicalUni"), _T("GPON physical User to Network interface") },
		{ 272, _T("vmwareNicTeam"), _T("VMware NIC Team") }
};

// http://www.ietf.org/rfc/rfc2863.txt Pg 31
static LOOKUP_ENTRY IF_OPER_STATUSES[] = {
		{ 1, _T("Up"), NULL },
		{ 2, _T("Down"), NULL },
		{ 3, _T("Testing"), NULL},
		{ 4, _T("Unknown"), NULL },
		{ 5, _T("Dormant"), NULL },
		{ 6, _T("Not present"), NULL },
		{ 7, _T("Lower layer down"), NULL }
};

// NET_IF_CONNECTION_TYPE
static LOOKUP_ENTRY IF_CONN_TYPES[] = {
		{ NET_IF_CONNECTION_DEDICATED, _T("Dedicated"), NULL },
		{ NET_IF_CONNECTION_PASSIVE, _T("Passive"), NULL },
		{ NET_IF_CONNECTION_DEMAND, _T("On-Demand"), NULL }
};

static LOOKUP_ENTRY IF_FLAGS[] = {
		{ IP_ADAPTER_DDNS_ENABLED, NULL, _T("Dynamic DNS is enabled on this adapter.") },
		{ IP_ADAPTER_REGISTER_ADAPTER_SUFFIX, NULL, _T("Register the DNS suffix for this adapter.") },
		{ IP_ADAPTER_DHCP_ENABLED, NULL, _T("The Dynamic Host Configuration Protocol (DHCP) is enabled on this adapter.") },
		{ IP_ADAPTER_RECEIVE_ONLY, NULL, _T("The adapter is a receive-only adapter.") },
		{ IP_ADAPTER_NO_MULTICAST, NULL, _T("The adapter is not a multicast recipient.") },
		{ IP_ADAPTER_IPV6_OTHER_STATEFUL_CONFIG, NULL, _T("The adapter contains other IPv6-specific stateful configuration information.") },
		{ IP_ADAPTER_NETBIOS_OVER_TCPIP_ENABLED, NULL, _T("The adapter is enabled for NetBIOS over TCP/IP.") },
		{ IP_ADAPTER_IPV4_ENABLED, NULL, _T("The adapter is enabled for IPv4.") },
		{ IP_ADAPTER_IPV6_ENABLED, NULL, _T("The adapter is enabled for IPv6.") },
		{ IP_ADAPTER_IPV6_MANAGE_ADDRESS_CONFIG, NULL, _T("The adapter is enabled for IPv6 managed address configuration.") },
};

PNODE EnumNetworkAdapters()
{
	PNODE nicsNode = node_alloc(_T("NetworkAdapters"), NODE_FLAG_TABLE);
	PNODE nicNode = NULL;
	ULONG bufferSize = BUFFER_SIZE;
	PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES) MALLOC(bufferSize);
	PIP_ADAPTER_ADDRESSES pCurrent = pAddresses;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	ULONG result = 0;
	TCHAR szBuffer[SZBUFFERLEN];
	LPTSTR pszBuffer = NULL;
	DWORD i = 0;
	PLOOKUP_ENTRY lookupResult = NULL;

	// Get network adapters
	while (ERROR_BUFFER_OVERFLOW == (result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &bufferSize))){
		FREE(pAddresses);
		pAddresses = (PIP_ADAPTER_ADDRESSES)MALLOC(bufferSize);
	}

	// Handle errors
	if (ERROR_SUCCESS != result) {
		SetError(ERR_CRIT, result, _T("Failed to enumerate Network Adapters."));
		return nicsNode;
	}

	// Parse each adapter
	while (NULL != pCurrent) {
		nicNode = node_append_new(nicsNode, _T("NetworkAdapter"), NODE_FLAG_TABLE_ENTRY);

		SWPRINTF(szBuffer, _T("%u"), pCurrent->IfIndex);
		node_att_set(nicNode, _T("Ipv4Index"), szBuffer, NODE_ATT_FLAG_KEY);

		SWPRINTF(szBuffer, _T("%u"), pCurrent->Ipv6IfIndex);
		node_att_set(nicNode, _T("Ipv6Index"), szBuffer, NODE_ATT_FLAG_KEY);

		UTF8_TO_UNICODE(pCurrent->AdapterName, szBuffer, SZBUFFERLEN);
		node_att_set(nicNode, _T("Guid"), szBuffer, 0);

		node_att_set(nicNode, _T("Name"), pCurrent->FriendlyName, 0);

		// Interface type
		if (NULL != (lookupResult = Lookup(IF_TYPES, pCurrent->IfType))) {
			node_att_set(nicNode, _T("Type"), lookupResult->Description, 0);
		}

		else {
			node_att_set(nicNode, _T("Type"), _T("Unknown"), NODE_ATT_FLAG_ERROR);
			SetError(ERR_WARN, 0, _T("Unknown Network Adapter Type: %u"), pCurrent->IfType);
		}

		// Connection type
		if (NULL != (lookupResult = Lookup(IF_CONN_TYPES, pCurrent->ConnectionType))) {
			node_att_set(nicNode, _T("ConnectionType"), lookupResult->Code, 0);
		}

		else {
			node_att_set(nicNode, _T("ConnectionType"), _T("Unknown"), NODE_ATT_FLAG_ERROR);
			SetError(ERR_WARN, 0, _T("Unknown Connection Type: %u"), pCurrent->IfType);
		}

		// Parse Physical Address (MAC)
		if (pCurrent->PhysicalAddressLength) {
			pszBuffer = &szBuffer[0];
			for (i = 0; i < pCurrent->PhysicalAddressLength; i++) {
				if (i < pCurrent->PhysicalAddressLength - 1) {
					swprintf_s(pszBuffer, 4, _T("%.2X:"), pCurrent->PhysicalAddress[i]);
					pszBuffer += 3;
				}

				else {
					swprintf_s(pszBuffer, 4, _T("%.2X"), pCurrent->PhysicalAddress[i]);
				}
			}

			node_att_set(nicNode, _T("PhysicalAddress"), szBuffer, 0);
		}

		// Operational status
		if (NULL != (lookupResult = Lookup(IF_OPER_STATUSES, pCurrent->OperStatus))) {
			node_att_set(nicNode, _T("OperationalState"), lookupResult->Code, 0);
		}
		else {
			node_att_set(nicNode, _T("OperationalState"), _T("Unknown"), NODE_ATT_FLAG_ERROR);
			SetError(ERR_WARN, 0, _T("Unknown Network Adapter Operational Status: %u"), pCurrent->OperStatus);
		}

		// Connection speed
		SWPRINTF(szBuffer, _T("%I64u"), pCurrent->TransmitLinkSpeed);
		node_att_set(nicNode, _T("TransmitSpeed"), szBuffer, 0);

		SWPRINTF(szBuffer, _T("%I64u"), pCurrent->ReceiveLinkSpeed);
		node_att_set(nicNode, _T("ReceiveSpeed"), szBuffer, 0);

		// Unicast addresses
		pUnicast = pCurrent->FirstUnicastAddress;
		// TODO: Add network adapter IPv4/6 addresses

		// Flags
		pszBuffer = NULL;
		for (i = 0; i < ARRAYSIZE(IF_FLAGS); i++) {
			if (CHECK_BIT(pCurrent->Flags, IF_FLAGS[i].Index))
				AppendMultiString(&pszBuffer, IF_FLAGS[i].Description);
		}
		node_att_set_multi(nicNode, _T("Flags"), pszBuffer, 0);
		LocalFree(pszBuffer);

		pCurrent = pCurrent->Next;
	}

	return nicsNode;
}