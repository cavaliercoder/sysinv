#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

// 7.1.1 BIOS Characteristics 
static LPCTSTR BIOS_CHARACTERISTICS[] = {
	_T("Reserved"),													// Bit 0
	_T("Reserved"),													// Bit 1
	_T("Unknown"),													// Bit 2
	_T("BIOS Characteristics are not supported"),					// Bit 3
	_T("ISA is supported"),											// Bit 4
	_T("MCA is supported"),											// Bit 5
	_T("EISA is supported"),										// Bit 6
	_T("PCI is supported"),											// Bit 7
	_T("PC card (PCMCIA) is supported"),							// Bit 8
	_T("Plug and Play is supported"),								// Bit 9
	_T("APM is supported"),											// Bit 10
	_T("BIOS is upgradeable (Flash)"),								// Bit 11
	_T("BIOS shadowing is allowed"),								// Bit 12
	_T("VL-VESA is supported"),										// Bit 13
	_T("ESCD support is available"),								// Bit 14
	_T("Boot from CD is supported"),								// Bit 15
	_T("Selectable boot is supported"),								// Bit 16
	_T("BIOS ROM is socketed"),										// Bit 17
	_T("Boot from PC card (PCMCIA) is supported"),					// Bit 18
	_T("EDD specification is supported"),							// Bit 19
	_T("Int 13h — Japanese floppy for NEC 9800 1.2 MB (3.5”, 1K bytes/sector, 360 RPM) is supported"),	// Bit 20
	_T("Int 13h — Japanese floppy for Toshiba 1.2 MB (3.5”, 360 RPM) is supported"),	// Bit 21
	_T("Int 13h — 5.25” / 360 KB floppy services are supported"),	// Bit 22
	_T("Int 13h — 5.25” /1.2 MB floppy services are supported"),	// Bit 23
	_T("Int 13h — 3.5” / 720 KB floppy services are supported"),	// Bit 24
	_T("Int 13h — 3.5” / 2.88 MB floppy services are supported"),	// Bit 25
	_T("Int 5h, print screen Service is supported"),				// Bit 26
	_T("Int 9h, 8042 keyboard services are supported"),				// Bit 27
	_T("Int 14h, serial services are supported"),					// Bit 28
	_T("Int 17h, printer services are supported"),					// Bit 29
	_T("Int 10h, CGA/Mono Video Services are supported"),			// Bit 30
	_T("NEC PC-98.")												// Bit 31
	// Bits32:47 Reserved for BIOS vendor.
	// Bits 48 : 63 Reserved for system vendor.
};

// 7.1.2.1 BIOS Characteristics Extension Byte 1
static LPCTSTR BIOS_CHARACTERISTICS_EX1[] = {
	_T("ACPI is supported"),										// Bit 0
	_T("USB Legacy is supported"),									// Bit 1
	_T("AGP is supported"),											// Bit 2
	_T("I2O boot is supported")										// Bit 3
};

// 7.1.2.2 BIOS Characteristics Extension Byte 2 
static LPCTSTR BIOS_CHARACTERISTICS_EX2[] = {
	_T("BIOS Boot Specification is supported"),						// Bit 0
	_T("Function key-initiated network service boot is supported"),	// Bit 1
	_T("Enable targeted content distribution"),						// Bit 2
	_T("UEFI Specification is supported"),							// Bit 3
	_T("SMBIOS table describes a virtual machine")					// Bit 4
	// Bits 5:7 Reserved for future assignment by this specification.
};

// BIOS Info table (7.1)
PNODE GetBiosDetail()
{
	PNODE node = NULL;
	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];
	DWORD i = 0;
	DWORD dwBuffer = 0;

	header = GetNextStructureOfType(NULL, SMB_TABLE_BIOS);
	if (NULL == header)
		return node;

	// v2.0+
	if (header->Length < 0x12) goto parsed;
	node = node_alloc(_T("Bios"), 0);

	// 0x04 Vendor
	unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x04));
	node_att_set(node, _T("Vendor"), unicode, 0);
	LocalFree(unicode);

	// 0x05 BIOS Version
	unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x05));
	node_att_set(node, _T("Version"), unicode, 0);
	LocalFree(unicode);

	// 0x06 Starting Address Segment
	swprintf(buffer, _T("0x%X"), WORD_AT_OFFSET(header, 0x06));
	node_att_set(node, _T("Address"), buffer, 0);

	// 0x06 Runtime Size
	swprintf(buffer, _T("%u"), 16 * (0x10000 - WORD_AT_OFFSET(header, 0x06)));
	node_att_set(node, _T("RuntimeSizeBytes"), buffer, 0);

	// 0x08 BIOS Release Date
	unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x08));
	node_att_set(node, _T("Date"), unicode, NAFLG_FMT_DATETIME);
	LocalFree(unicode);

	// 0x09 BIOS ROM Size
	swprintf(buffer, _T("%llu"), 64 * (1 + BYTE_AT_OFFSET(header, 0x09)));
	node_att_set(node, _T("RomSizeKb"), buffer, NAFLG_FMT_KBYTES);

	// 0x0A Characteristics
	unicode = NULL;
	for (i = 0; i < ARRAYSIZE(BIOS_CHARACTERISTICS); i++) {
		if (CHECK_BIT(QWORD_AT_OFFSET(header, 0x0A), i)) {
			AppendMultiString(&unicode, BIOS_CHARACTERISTICS[i]);
		}
	}

	// v2.4 +
	// 0x12 Characterist extensions byte 1
	if (header->Length < 0x13) goto got_chars;
	dwBuffer = BYTE_AT_OFFSET(header, 0x12);
	for (i = 0; i < ARRAYSIZE(BIOS_CHARACTERISTICS_EX1); i++) {
		if (CHECK_BIT(dwBuffer, i)) {
			AppendMultiString(&unicode, BIOS_CHARACTERISTICS_EX1[i]);
		}
	}

	// 0x12 Characterist extensions byte 2
	if (header->Length < 0x14) goto got_chars;
	dwBuffer = BYTE_AT_OFFSET(header, 0x13);
	for (i = 0; i < ARRAYSIZE(BIOS_CHARACTERISTICS_EX2); i++) {
		if (CHECK_BIT(dwBuffer, i)) {
			AppendMultiString(&unicode, BIOS_CHARACTERISTICS_EX2[i]);
		}
	}

got_chars:
	node_att_set_multi(node, _T("Characteristics"), unicode, 0);
	LocalFree(unicode);

	// 0x14, 0x15 System BIOS Release
	if (header->Length < 0x16) goto parsed;
	swprintf(buffer, _T("%u.%u"), BYTE_AT_OFFSET(header, 0x14), BYTE_AT_OFFSET(header, 0x15));
	node_att_set(node, _T("Revision"), buffer, 0);

	// 0x16, 0x17 Embedded Controller Firmware Release
	if (header->Length < 0x18) goto parsed;
	swprintf(buffer, _T("%u.%u"), BYTE_AT_OFFSET(header, 0x16), BYTE_AT_OFFSET(header, 0x17));
	node_att_set(node, _T("FirmwareRevision"), buffer, 0);

parsed:

	return node;
}