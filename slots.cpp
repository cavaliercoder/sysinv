#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include "baseboard.h"

static LPCTSTR SLOT_TYPE_STRINGS[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other 
	_T("Unknown"),								// 0x02 Unknown 
	_T("ISA"),									// 0x03 ISA 
	_T("MCA"),									// 0x04 MCA 
	_T("EISA"),									// 0x05 EISA 
	_T("PCI"),									// 0x06 PCI 
	_T("PC Card (PCMCIA)"),						// 0x07 PC Card (PCMCIA) 
	_T("VL-VESA"),								// 0x08 VL-VESA 
	_T("Proprietary"),							// 0x09 Proprietary 
	_T("Processor Card Slot"),					// 0x0A Processor Card Slot
	_T("Proprietary Memory Card Slot"),			// 0x0B Proprietary Memory Card Slot 
	_T("I/O Riser Card Slot"),					// 0x0C I/O Riser Card Slot 
	_T("NuBus"),								// 0x0D NuBus 
	_T("PCI – 66MHz Capable"),					// 0x0E PCI – 66MHz Capable 
	_T("AGP"),									// 0x0F AGP 
	_T("AGP 2X"),								// 0x10 AGP 2X 
	_T("AGP 4X"),								// 0x11 AGP 4X 
	_T("PCI-X"),								// 0x12 PCI-X 
	_T("AGP 8X")								// 0x13 AGP 8X 
	_T("PC-98/C20"),							// 0xA0 PC-98/C20 
	_T("PC-98/C24"),							// 0xA1 PC-98/C24 
	_T("PC-98/E"),								// 0xA2 PC-98/E 
	_T("PC-98/Local Bus"),						// 0xA3 PC-98/Local Bus 
	_T("PC-98/Card"),							// 0xA4 PC-98/Card 
	_T("PCI Express (see below)"),				// 0xA5 PCI Express (see below) 
	_T("PCI Express x1"),						// 0xA6 PCI Express x1 
	_T("PCI Express x2"),						// 0xA7 PCI Express x2 
	_T("PCI Express x4"),						// 0xA8 PCI Express x4 
	_T("PCI Express x8"),						// 0xA9 PCI Express x8 
	_T("PCI Express x16"),						// 0xAA PCI Express x16 
	_T("PCI Express Gen 2 (see below)"),		// 0xAB PCI Express Gen 2 (see below) 
	_T("PCI Express Gen 2 x1"),					// 0xAC PCI Express Gen 2 x1 
	_T("PCI Express Gen 2 x2"),					// 0xAD PCI Express Gen 2 x2 
	_T("PCI Express Gen 2 x4"),					// 0xAE PCI Express Gen 2 x4 
	_T("PCI Express Gen 2 x8"),					// 0xAF PCI Express Gen 2 x8 
	_T("PCI Express Gen 2 x16"),				// 0xB0 PCI Express Gen 2 x16 
	_T("PCI Express Gen 3 (see below)"),		// 0xB1 PCI Express Gen 3 (see below) 
	_T("PCI Express Gen 3 x1"),					// 0xB2 PCI Express Gen 3 x1 
	_T("PCI Express Gen 3 x2"),					// 0xB3 PCI Express Gen 3 x2 
	_T("PCI Express Gen 3 x4"),					// 0xB4 PCI Express Gen 3 x4 
	_T("PCI Express Gen 3 x8"),					// 0xB5 PCI Express Gen 3 x8 
	_T("PCI Express Gen 3 x16")					// 0xB6 PCI Express Gen 3 x16
};

static LPCTSTR SLOT_DATA_BUS_WIDTH[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other 
	_T("Unknown"),								// 0x02 Unknown 
	_T("8 bit"),								// 0x03 8 bit 
	_T("16 bit"),								// 0x04 16 bit 
	_T("32 bit"),								// 0x05 32 bit 
	_T("64 bit"),								// 0x06 64 bit 
	_T("128 bit"),								// 0x07 128 bit 
	_T("1x or x1"),								// 0x08 1x or x1 
	_T("2x or x2"),								// 0x09 2x or x2 
	_T("4x or x4"),								// 0x0A 4x or x4 
	_T("8x or x8"),								// 0x0B 8x or x8 
	_T("12x or x12"),							// 0x0C 12x or x12 
	_T("16x or x16"),							// 0x0D 16x or x16 
	_T("32x or x32")							// 0x0E 32x or x32 
};

static LPCTSTR SLOT_CURRENT_USAGE[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other 
	_T("Unknown"),								// 0x02 Unknown 
	_T("Available"),							// 0x03 Available 
	_T("In use")								// 0x04 In use
};

static LPCTSTR SLOT_LENGTH[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other 
	_T("Unknown"),								// 0x02 Unknown 
	_T("Short Length"),							// 0x03 Short Length
	_T("Long Length")							// 0x04 Long Length
};

static LPCTSTR SLOT_CHARACTERISTICS1[] = {
	_T("Characteristics unknown."),					// Bit 0
	_T("Provides 5.0 volts."),						// Bit 1
	_T("Provides 3.3 volts."),						// Bit 2
	_T("Slot’s opening is shared with another slot(for example, PCI / EISA shared slot)."),	// Bit 3
	_T("PC Card slot supports PC Card - 16."),		// Bit 4
	_T("PC Card slot supports CardBus."),			// Bit 5
	_T("PC Card slot supports Zoom Video."),		// Bit 6
	_T("PC Card slot supports Modem Ring Resume."),	// Bit 7
};

static LPCTSTR SLOT_CHARACTERISTICS2[] = {
	_T("PCI slot supports Power Management Event(PME#) signal."),	// Bit 0
	_T("Slot supports hot - plug devices."),	// Bit 1
	_T("PCI slot supports SMBus signal.")		// Bit 2
};

// SMBIOS Table Type 4 (appended to a Baseboard Type 2)
PNODE GetSlotDetail(PRAW_SMBIOS_DATA smbios, PSMBIOS_STRUCT_HEADER header)
{
	PNODE node = NULL;
	LPTSTR pszBuffer = NULL;
	TCHAR szBuffer[MAX_PATH + 1];
	DWORD slotType = 0;
	DWORD i = 0;

	node = node_alloc(_T("Slot"), NFLG_TABLE_ROW);

	// 0x04 Designation
	pszBuffer = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x04));
	node_att_set(node, _T("Designation"), pszBuffer, 0);
	LocalFree(pszBuffer);

	// 0x05 Slot Type
	slotType = BYTE_AT_OFFSET(header, 0x05);
	node_att_set(node, _T("Type"), SAFE_INDEX(SLOT_TYPE_STRINGS, slotType), 0);

	// 0x06 Slot Data Bus Width
	node_att_set(node, _T("DataBusWidth"), SAFE_INDEX(SLOT_DATA_BUS_WIDTH, BYTE_AT_OFFSET(header, 0x06)), 0);

	// 0x07 Curent Usage
	node_att_set(node, _T("Usage"), SAFE_INDEX(SLOT_CURRENT_USAGE, BYTE_AT_OFFSET(header, 0x07)), 0);

	// 0x08 Slot Length
	node_att_set(node, _T("Length"), SAFE_INDEX(SLOT_LENGTH, BYTE_AT_OFFSET(header, 0x08)), 0);

	// 0x09 Slot ID
	switch (slotType) {
	case 0x04:	// MCA
	case 0x05:	// EISA
		swprintf(szBuffer, _T("%u"), BYTE_AT_OFFSET(header, 0x09) + 1);
		node_att_set(node, _T("SlotNumber"), szBuffer, NAFLG_FMT_NUMERIC);
		break;

	case 0x07:	// PC Card (PCMCIA)
		swprintf(szBuffer, _T("%u"), BYTE_AT_OFFSET(header, 0x09));
		node_att_set(node, _T("AdapterNumber"), szBuffer, NAFLG_FMT_NUMERIC);

		swprintf(szBuffer, _T("%u"), BYTE_AT_OFFSET(header, 0x0A));
		node_att_set(node, _T("SocketNumber"), szBuffer, NAFLG_FMT_NUMERIC);
		break;

	default:	// Other
		// PCI, AGP, PCI-X, and PCI Express variants
		if (
			0x06 == slotType
			|| (0x0E <= slotType && 0x13 >= slotType)
			|| (0xA5 <= slotType && 0xB6 >= slotType)
			) {
			swprintf(szBuffer, _T("%u"), BYTE_AT_OFFSET(header, 0x09));
			node_att_set(node, _T("SlotNumber"), szBuffer, NAFLG_FMT_NUMERIC);
		}
	}

	// 0x0B Characteristics 1
	pszBuffer = NULL;
	for (i = 0; i < ARRAYSIZE(SLOT_CHARACTERISTICS1); i++) {
		if (CHECK_BIT(BYTE_AT_OFFSET(header, 0x0B), i)) {
			AppendMultiString(&pszBuffer, SLOT_CHARACTERISTICS1[i]);
		}
	}

	// v2.1+
	if (2 < smbios->SMBIOSMajorVersion || (2 == smbios->SMBIOSMajorVersion && 1 <= smbios->SMBIOSMinorVersion)) {
		//0x0C Characteristics 2
		for (i = 0; i < ARRAYSIZE(SLOT_CHARACTERISTICS2); i++) {
			if (CHECK_BIT(BYTE_AT_OFFSET(header, 0x0C), i)) {
				AppendMultiString(&pszBuffer, SLOT_CHARACTERISTICS2[i]);
			}
		}
	}

	node_att_set_multi(node, _T("Characteristics"), pszBuffer, 0);
	LocalFree(pszBuffer);

	return node;
}