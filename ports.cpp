#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include "baseboard.h"

// 7.9.2 Port Information — Connector Types 
LOOKUP_ENTRY PORT_CONN_TYPES[] = {
		{ 0x00, NULL, _T("None") },
		{ 0x01, NULL, _T("Centronics") },
		{ 0x02, NULL, _T("Mini Centronics") },
		{ 0x03, NULL, _T("Proprietary") },
		{ 0x04, NULL, _T("DB-25 pin male") },
		{ 0x05, NULL, _T("DB-25 pin female") },
		{ 0x06, NULL, _T("DB-15 pin male") },
		{ 0x07, NULL, _T("DB-15 pin female") },
		{ 0x08, NULL, _T("DB-9 pin male") },
		{ 0x09, NULL, _T("DB-9 pin female") },
		{ 0x0A, NULL, _T("RJ-11") },
		{ 0x0B, NULL, _T("RJ-45") },
		{ 0x0C, NULL, _T("50-pin MiniSCSI") },
		{ 0x0D, NULL, _T("Mini-DIN") },
		{ 0x0E, NULL, _T("Micro-DIN") },
		{ 0x0F, NULL, _T("PS/2") },
		{ 0x10, NULL, _T("Infrared") },
		{ 0x11, NULL, _T("HP-HIL") },
		{ 0x12, NULL, _T("Access Bus (USB)") },
		{ 0x13, NULL, _T("SSA SCSI") },
		{ 0x14, NULL, _T("Circular DIN-8 male") },
		{ 0x15, NULL, _T("Circular DIN-8 female") },
		{ 0x16, NULL, _T("On Board IDE") },
		{ 0x17, NULL, _T("On Board Floppy") },
		{ 0x18, NULL, _T("9-pin Dual Inline (pin 10 cut)") },
		{ 0x19, NULL, _T("25-pin Dual Inline (pin 26 cut)") },
		{ 0x1A, NULL, _T("50-pin Dual Inline") },
		{ 0x1B, NULL, _T("68-pin Dual Inline") },
		{ 0x1C, NULL, _T("On Board Sound Input from CD-ROM") },
		{ 0x1D, NULL, _T("Mini-Centronics Type-14") },
		{ 0x1E, NULL, _T("Mini-Centronics Type-26") },
		{ 0x1F, NULL, _T("Mini-jack (headphones)") },
		{ 0x20, NULL, _T("BNC") },
		{ 0x21, NULL, _T("1394") },
		{ 0x22, NULL, _T("SAS/SATA Plug Receptacle") },
		{ 0xA0, NULL, _T("PC-98") },
		{ 0xA1, NULL, _T("PC-98Hireso") },
		{ 0xA2, NULL, _T("PC-H98") },
		{ 0xA3, NULL, _T("PC-98Note") },
		{ 0xA4, NULL, _T("PC-98Full") }
};

// 7.9.3 Port Types 
LOOKUP_ENTRY PORT_TYPES[] = {
		{ 0x00, NULL, _T("None") },
		{ 0x01, NULL, _T("Parallel Port XT/AT Compatible") },
		{ 0x02, NULL, _T("Parallel Port PS/2") },
		{ 0x03, NULL, _T("Parallel Port ECP") },
		{ 0x04, NULL, _T("Parallel Port EPP") },
		{ 0x05, NULL, _T("Parallel Port ECP/EPP") },
		{ 0x06, NULL, _T("Serial Port XT/AT Compatible") },
		{ 0x07, NULL, _T("Serial Port 16450 Compatible") },
		{ 0x08, NULL, _T("Serial Port 16550 Compatible") },
		{ 0x09, NULL, _T("Serial Port 16550A Compatible") },
		{ 0x0A, NULL, _T("SCSI Port") },
		{ 0x0B, NULL, _T("MIDI Port") },
		{ 0x0C, NULL, _T("Joy Stick Port") },
		{ 0x0D, NULL, _T("Keyboard Port") },
		{ 0x0E, NULL, _T("Mouse Port") },
		{ 0x0F, NULL, _T("SSA SCSI") },
		{ 0x10, NULL, _T("USB") },
		{ 0x11, NULL, _T("FireWire (IEEE P1394)") },
		{ 0x12, NULL, _T("PCMCIA Type I2") },
		{ 0x13, NULL, _T("PCMCIA Type II") },
		{ 0x14, NULL, _T("PCMCIA Type III") },
		{ 0x15, NULL, _T("Cardbus") },
		{ 0x16, NULL, _T("Access Bus Port") },
		{ 0x17, NULL, _T("SCSI II") },
		{ 0x18, NULL, _T("SCSI Wide") },
		{ 0x19, NULL, _T("PC-98") },
		{ 0x1A, NULL, _T("PC-98-Hireso") },
		{ 0x1B, NULL, _T("PC-H98") },
		{ 0x1C, NULL, _T("Video Port") },
		{ 0x1D, NULL, _T("Audio Port") },
		{ 0x1E, NULL, _T("Modem Port") },
		{ 0x1F, NULL, _T("Network Port") },
		{ 0x20, NULL, _T("SATA") },
		{ 0x21, NULL, _T("SAS") },
		{ 0xA0, NULL, _T("8251 Compatible") },
		{ 0xA1, NULL, _T("8251 FIFO Compatible") },
		{ 0xFF, NULL, _T("Other") }
};

PNODE GetPortDetail(PRAW_SMBIOS_DATA smbios, PSMBIOS_STRUCT_HEADER header)
{
	PNODE portNode = node_alloc(_T("Port"), NODE_FLAG_TABLE_ENTRY);
	LPTSTR pszBuffer = NULL;
	TCHAR szBuffer[MAX_PATH + 1];
	DWORD i = 0;
	DWORD dwBuffer = 0;

	// 0x04 Internal Reference Designator
	pszBuffer = GetSmbiosString(header, 0x04);
	node_att_set(portNode, _T("InternalDesignation"), pszBuffer, 0);
	LocalFree(pszBuffer);

	// 0x05 Internal Connector Type
	dwBuffer = BYTE_AT_OFFSET(header, 0x05);
	for (i = 0; i < ARRAYSIZE(PORT_CONN_TYPES); i++) {
		if (PORT_CONN_TYPES[i].Index == dwBuffer) {
			node_att_set(portNode, _T("InternalConnectorType"), PORT_CONN_TYPES[i].Description, 0);
			break;
		}
	}

	// 0x06 External Reference Designator
	pszBuffer = GetSmbiosString(header, 0x06);
	node_att_set(portNode, _T("ExternalDesignation"), pszBuffer, 0);
	LocalFree(pszBuffer);

	// 0x07 External Connector Type
	dwBuffer = BYTE_AT_OFFSET(header, 0x07);
	for (i = 0; i < ARRAYSIZE(PORT_CONN_TYPES); i++) {
		if (PORT_CONN_TYPES[i].Index == dwBuffer) {
			node_att_set(portNode, _T("ExternalConnectorType"), PORT_CONN_TYPES[i].Description, 0);
			break;
		}
	}

	// 0x08 Port Type
	dwBuffer = BYTE_AT_OFFSET(header, 0x08);
	for (i = 0; i < ARRAYSIZE(PORT_TYPES); i++) {
		if (PORT_TYPES[i].Index == dwBuffer) {
			node_att_set(portNode, _T("Type"), PORT_TYPES[i].Description, 0);
			break;
		}
	}

	return portNode;
}