#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"

LPTSTR SLOT_TYPE_STRINGS[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other "),								// 0x01 Other 
	_T("Unknown "),								// 0x02 Unknown 
	_T("ISA "),									// 0x03 ISA 
	_T("MCA "),									// 0x04 MCA 
	_T("EISA "),								// 0x05 EISA 
	_T("PCI "),									// 0x06 PCI 
	_T("PC Card (PCMCIA) "),					// 0x07 PC Card (PCMCIA) 
	_T("VL-VESA "),								// 0x08 VL-VESA 
	_T("Proprietary "),							// 0x09 Proprietary 
	_T("Processor Card Slot"),					// 0x0A Processor Card Slot
	_T("Proprietary Memory Card Slot "),		// 0x0B Proprietary Memory Card Slot 
	_T("I/O Riser Card Slot "),					// 0x0C I/O Riser Card Slot 
	_T("NuBus "),								// 0x0D NuBus 
	_T("PCI – 66MHz Capable "),					// 0x0E PCI – 66MHz Capable 
	_T("AGP "),									// 0x0F AGP 
	_T("AGP 2X "),								// 0x10 AGP 2X 
	_T("AGP 4X "),								// 0x11 AGP 4X 
	_T("PCI-X "),								// 0x12 PCI-X 
	_T("AGP 8X ")								// 0x13 AGP 8X 
	_T("PC-98/C20 "),							// 0xA0 PC-98/C20 
	_T("PC-98/C24 "),							// 0xA1 PC-98/C24 
	_T("PC-98/E "),								// 0xA2 PC-98/E 
	_T("PC-98/Local Bus "),						// 0xA3 PC-98/Local Bus 
	_T("PC-98/Card "),							// 0xA4 PC-98/Card 
	_T("PCI Express (see below) "),				// 0xA5 PCI Express (see below) 
	_T("PCI Express x1 "),						// 0xA6 PCI Express x1 
	_T("PCI Express x2 "),						// 0xA7 PCI Express x2 
	_T("PCI Express x4 "),						// 0xA8 PCI Express x4 
	_T("PCI Express x8 "),						// 0xA9 PCI Express x8 
	_T("PCI Express x16 "),						// 0xAA PCI Express x16 
	_T("PCI Express Gen 2 (see below) "),		// 0xAB PCI Express Gen 2 (see below) 
	_T("PCI Express Gen 2 x1 "),				// 0xAC PCI Express Gen 2 x1 
	_T("PCI Express Gen 2 x2 "),				// 0xAD PCI Express Gen 2 x2 
	_T("PCI Express Gen 2 x4 "),				// 0xAE PCI Express Gen 2 x4 
	_T("PCI Express Gen 2 x8 "),				// 0xAF PCI Express Gen 2 x8 
	_T("PCI Express Gen 2 x16 "),				// 0xB0 PCI Express Gen 2 x16 
	_T("PCI Express Gen 3 (see below) "),		// 0xB1 PCI Express Gen 3 (see below) 
	_T("PCI Express Gen 3 x1 "),				// 0xB2 PCI Express Gen 3 x1 
	_T("PCI Express Gen 3 x2 "),				// 0xB3 PCI Express Gen 3 x2 
	_T("PCI Express Gen 3 x4 "),				// 0xB4 PCI Express Gen 3 x4 
	_T("PCI Express Gen 3 x8 "),				// 0xB5 PCI Express Gen 3 x8 
	_T("PCI Express Gen 3 x16")					// 0xB6 PCI Express Gen 3 x16
};

LPTSTR SLOT_DATA_BUS_WIDTH[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other "),								// 0x01 Other 
	_T("Unknown "),								// 0x02 Unknown 
	_T("8 bit "),								// 0x03 8 bit 
	_T("16 bit "),								// 0x04 16 bit 
	_T("32 bit "),								// 0x05 32 bit 
	_T("64 bit "),								// 0x06 64 bit 
	_T("128 bit "),								// 0x07 128 bit 
	_T("1x or x1 "),							// 0x08 1x or x1 
	_T("2x or x2 "),							// 0x09 2x or x2 
	_T("4x or x4 "),							// 0x0A 4x or x4 
	_T("8x or x8 "),							// 0x0B 8x or x8 
	_T("12x or x12 "),							// 0x0C 12x or x12 
	_T("16x or x16 "),							// 0x0D 16x or x16 
	_T("32x or x32 ")							// 0x0E 32x or x32 
};

LPTSTR SLOT_CURRENT_USAGE[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other "),								// 0x01 Other 
	_T("Unknown "),								// 0x02 Unknown 
	_T("Available "),							// 0x03 Available 
	_T("In use")								// 0x04 In use
};

LPTSTR SLOT_LENGTH[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other "),								// 0x01 Other 
	_T("Unknown "),								// 0x02 Unknown 
	_T("Short Length"),							// 0x03 Short Length
	_T("Long Length")							// 0x04 Long Length
};

// SMBIOS Table Type 4
PNODE EnumSlots()
{
	PNODE slotsNode = node_alloc(_T("Slots"), NODE_FLAG_TABLE);
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PBYTE cursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_SLOTS))) {
		cursor = (PBYTE)header;
		node = node_append_new(slotsNode, _T("Slot"), NODE_FLAG_TABLE_ENTRY);

		// v2.0+
		if (2 <= smbios->SMBIOSMajorVersion) {
			// 0x04 Designation
			unicode = GetSmbiosString(header, *(cursor + 0x04));
			node_att_set(node, _T("Designation"), unicode, 0);
			LocalFree(unicode);

			// 0x05 Slot Type
			node_att_set(node, _T("Type"), SAFE_INDEX(SLOT_TYPE_STRINGS, *(cursor + 0x05)), 0);

			// 0x06 Slot Data Bus Width
			node_att_set(node, _T("DataBusWidth"), SAFE_INDEX(SLOT_DATA_BUS_WIDTH, *(cursor + 0x06)), 0);

			//0x07 Curent Usage
			node_att_set(node, _T("Usage"), SAFE_INDEX(SLOT_CURRENT_USAGE, *(cursor + 0x07)), 0);

			//0x08 Slot Length
			node_att_set(node, _T("Length"), SAFE_INDEX(SLOT_LENGTH, *(cursor + 0x08)), 0);
		}
	}

	return slotsNode;
}