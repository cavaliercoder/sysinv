#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include "virtualization.h"

// 7.17.1 Memory Array — Location 
LPCTSTR MEM_ARRAY_LOCATION[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other 
	_T("Unknown"),								// 0x02 Unknown 
	_T("System board or motherboard"),			// 0x03 System board or motherboard 
	_T("ISA add-on card"),						// 0x04 ISA add-on card 
	_T("EISA add-on card"),						// 0x05 EISA add-on card 
	_T("PCI add-on card"),						// 0x06 PCI add-on card 
	_T("MCA add-on card"),						// 0x07 MCA add-on card 
	_T("PCMCIA add-on card"),					// 0x08 PCMCIA add-on card 
	_T("Proprietary add-on card"),				// 0x09 Proprietary add-on card 
	_T("NuBus"),								// 0x0A NuBus 
	_T("PC-98/C20 add-on card"),				// 0xA0 PC-98/C20 add-on card 
	_T("PC-98/C24 add-on card"),				// 0xA1 PC-98/C24 add-on card 
	_T("PC-98/E add-on card"),					// 0xA2 PC-98/E add-on card 
	_T("PC-98/Local bus add-on card")			// 0xA3 PC-98/Local bus add-on card 
};

// 7.17.2 Memory Array — Use
LPCTSTR MEM_ARRAY_USE[] {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other 
	_T("Unknown"),								// 0x02 Unknown 
	_T("System memory"),						// 0x03 System memory 
	_T("Video memory"),							// 0x04 Video memory 
	_T("Flash memory"),							// 0x05 Flash memory 
	_T("Non-volatile RAM"),						// 0x06 Non-volatile RAM 
	_T("Cache memory")							// 0x07 Cache memory 
};

// 7.17.3 Memory Array — Error Correction Types
LPCTSTR MEM_ARRAY_EC_TYPE[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other
	_T("Unknown"),								// 0x02 Unknown
	_T("None"),									// 0x03 None
	_T("Parity"),								// 0x04 Parity
	_T("Single - bit ECC"),						// 0x05 Single - bit ECC
	_T("Multi - bit ECC"),						// 0x06 Multi - bit ECC
	_T("CRC")									// 0x07 CRC
};

// 7.18.1 Memory Device — Form Factor
LPCTSTR MEM_FORM_FACTOR[] = {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other
	_T("Unknown"),								// 0x02 Unknown
	_T("SIMM"),									// 0x03 SIMM
	_T("SIP"),									// 0x04 SIP
	_T("Chip"),									// 0x05 Chip
	_T("DIP"),									// 0x06 DIP
	_T("ZIP"),									// 0x07 ZIP
	_T("Proprietary Card"),						// 0x08 Proprietary Card
	_T("DIMM"),									// 0x09 DIMM
	_T("TSOP"),									// 0x0A TSOP
	_T("Row of chips"),							// 0x0B Row of chips
	_T("RIMM"),									// 0x0C RIMM
	_T("SODIMM"),								// 0x0D SODIMM
	_T("SRIMM"),								// 0x0E SRIMM
	_T("FB-DIMM")								// 0x0F FB-DIMM
};

// 7.18.2 Memory Device — Type
LPCTSTR MEM_TYPE[] {
	_T("Invalid"),								// 0x00 Invalid
	_T("Other"),								// 0x01 Other
	_T("Unknown"),								// 0x02 Unknown
	_T("DRAM"),									// 0x03 DRAM
	_T("EDRAM"),								// 0x04 EDRAM
	_T("VRAM"),									// 0x05 VRAM
	_T("SRAM"),									// 0x06 SRAM
	_T("RAM"),									// 0x07 RAM
	_T("ROM"),									// 0x08 ROM
	_T("FLASH"),								// 0x09 FLASH
	_T("EEPROM"),								// 0x0A EEPROM
	_T("FEPROM"),								// 0x0B FEPROM
	_T("EPROM"),								// 0x0C EPROM
	_T("CDRAM"),								// 0x0D CDRAM
	_T("3DRAM"),								// 0x0E 3DRAM
	_T("SDRAM"),								// 0x0F SDRAM
	_T("SGRAM"),								// 0x10 SGRAM
	_T("RDRAM"),								// 0x11 RDRAM
	_T("DDR"),									// 0x12 DDR
	_T("DDR2"),									// 0x13 DDR2
	_T("DDR2 FB-DIMM"),							// 0x14 DDR2 FB-DIMM
	_T("15h - 17h Reserved"),					// 0x15 15h - 17h Reserved
	_T("DDR3"),									// 0x18 DDR3
	_T("FBD2")									// 0x19 FBD2
};

//  7.18.3 Memory Device — Type Detail 
LPCTSTR MEM_TYPE_DETAIL[] {
	_T("Reserved, set to 0"),					// Bit 0
	_T("Other"),								// Bit 1
	_T("Unknown"),								// Bit 2
	_T("Fast-paged"),							// Bit 3
	_T("Static column"),						// Bit 4
	_T("Pseudo-static"),						// Bit 5
	_T("RAMBUS"),								// Bit 6
	_T("Bit 7 Synchronous"),					// Bit 7
	_T("CMOS"),									// Bit 8
	_T("EDO"),									// Bit 9
	_T("Window DRAM"),							// Bit 10
	_T("Cache DRAM"),							// Bit 11
	_T("Non-volatile"),							// Bit 12
	_T("Registered (Buffered)"),				// Bit 13
	_T("Unbuffered (Unregistered)"),			// Bit 14
	_T("LRDIMM")								// Bit 15
};

// SMBIOS Table Type 16 Physical Memory Array
PNODE EnumMemorySockets()
{
	PNODE parentNode = NULL;
	PNODE node = NULL;
	PNODE devicesNode = NULL;
	PNODE deviceNode = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;
	PSMBIOS_STRUCT_HEADER memHeader = NULL;
	PBYTE cursor = NULL;
	PBYTE memCursor = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];
	DWORD i;

	WORD wbuffer = 0;
	DWORD dwBuffer = 0;
	
	QWORD totalArrayMb = 0;
	QWORD totalSystemMb = 0;

	BOOL isVirt = IsVirtualized();

	// v2.1+ (Use Table Type 5 Memory Controller for < v2.1)
	if (smbios->SMBIOSMajorVersion < 2 || (smbios->SMBIOSMajorVersion == 2 && smbios->SMBIOSMinorVersion < 1))
		return parentNode;

	parentNode = node_alloc(_T("Memory"), NFLG_TABLE);
	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_MEM_ARRAY))) {
		cursor = (PBYTE)header;
		node = node_append_new(parentNode, _T("Array"), NFLG_TABLE_ROW);

		// 0x04 Location
		node_att_set(node, _T("Location"), SAFE_INDEX(MEM_ARRAY_LOCATION, BYTE_AT_OFFSET(header, 0x04)), 0);

		// 0x05 Use
		node_att_set(node, _T("Use"), SAFE_INDEX(MEM_ARRAY_USE, BYTE_AT_OFFSET(header, 0x05)), 0);

		// 0x06 Error Checking
		node_att_set(node, _T("ErrorChecking"), SAFE_INDEX(MEM_ARRAY_EC_TYPE, BYTE_AT_OFFSET(header, 0x06)), 0);

		// 0x07 Maximum Capacity (KB)
		dwBuffer = DWORD_AT_OFFSET(header, 0x07);
		if (dwBuffer != 0x80000000) {
			swprintf(buffer, _T("%u"), dwBuffer);
			node_att_set(node, _T("MaxCapacityKb"), buffer, NAFLG_FMT_KBYTES);

			swprintf(buffer, _T("%u"), dwBuffer / 1024);
			node_att_set(node, _T("MaxCapacityMb"), buffer, NAFLG_FMT_MBYTES);

			swprintf(buffer, _T("%u"), dwBuffer / 1048576);
			node_att_set(node, _T("MaxCapacityGb"), buffer, NAFLG_FMT_GBYTES);
		}

		// 0x0D Number of memory devices
		swprintf(buffer, _T("%u"), BYTE_AT_OFFSET(header, 0x0D));
		node_att_set(node, _T("SocketCount"), buffer, NAFLG_FMT_NUMERIC);

		// 7.18 Memory Devices (Type 17)
		devicesNode = node_append_new(node, _T("Modules"), NFLG_TABLE);
		while (NULL != (memHeader = GetNextStructureOfType(memHeader, SMB_TABLE_MEM_DEVICE))) {
			memCursor = (PBYTE)memHeader;

			// 0x04 Memory Array Handle
			// Ensure array handle matches the parent handle
			if (header->Handle != WORD_AT_OFFSET(memHeader, 0x04))
				continue;

			// Skip empty slots
			if (isVirt && 0 == WORD_AT_OFFSET(memHeader, 0x0C))
				continue;

			deviceNode = node_append_new(devicesNode, _T("Module"), NFLG_TABLE_ROW);

			// 0x08 Total Width
			wbuffer = WORD_AT_OFFSET(memHeader, 0x08);
			if (0xFFFF != wbuffer) {
				swprintf(buffer, _T("%u"), wbuffer);
				node_att_set(deviceNode, _T("TotalWidth"), buffer, NAFLG_FMT_NUMERIC);
			}

			// 0x0A Data Width
			wbuffer = WORD_AT_OFFSET(memHeader, 0x0A);
			if (0xFFFF != wbuffer) {
				swprintf(buffer, _T("%u"), wbuffer);
				node_att_set(deviceNode, _T("DataWidth"), buffer, NAFLG_FMT_NUMERIC);
			}

			// 0x0C Size
			wbuffer = WORD_AT_OFFSET(memHeader, 0x0C);
			if (wbuffer & 0x8000) {
				// Size is in KB
				wbuffer &= ~0x8000;
				swprintf(buffer, _T("%u"), wbuffer);
				node_att_set(deviceNode, _T("Size"), buffer, NAFLG_FMT_KBYTES);
			}
			else
			{
				// Size is in MB. Scale down to bytes
				swprintf(buffer, _T("%llu"), (QWORD)wbuffer * 1048576LL);
				node_att_set(deviceNode, _T("SizeBytes"), buffer, NAFLG_FMT_BYTES);

				swprintf(buffer, _T("%u"), wbuffer);
				node_att_set(deviceNode, _T("SizeMb"), buffer, NAFLG_FMT_MBYTES);
				totalArrayMb += wbuffer;

				if (wbuffer > 1024) {
					swprintf(buffer, _T("%u"), wbuffer / 1024);
					node_att_set(deviceNode, _T("SizeGb"), buffer, NAFLG_FMT_GBYTES);
				}
			}
			// 0x0E Form Factor
			node_att_set(deviceNode, _T("FormFactor"), SAFE_INDEX(MEM_FORM_FACTOR, BYTE_AT_OFFSET(memHeader, 0x0E)), 0);

			// 0x0F Device Set
			wbuffer = BYTE_AT_OFFSET(memHeader, 0x0F);
			if (0xFF != wbuffer && 0x00 != wbuffer) {
				swprintf(buffer, _T("0x%X"), wbuffer);
				node_att_set(deviceNode, _T("DeviceSet"), buffer, NAFLG_FMT_HEX);
			}

			// 0x10 Device Locator
			node_att_set(deviceNode, _T("DeviceLocator"), GetSmbiosString(memHeader, BYTE_AT_OFFSET(memHeader, 0x10)), 0);

			// 0x11 Device Locator
			node_att_set(deviceNode, _T("BankLocator"), GetSmbiosString(memHeader, BYTE_AT_OFFSET(memHeader, 0x11)), 0);

			// 0x12 Memory Type
			node_att_set(deviceNode, _T("Type"), SAFE_INDEX(MEM_TYPE, BYTE_AT_OFFSET(memHeader, 0x012)), 0);

			// 0x13 Type Detail
			unicode = NULL;
			for (i = 0; i < ARRAYSIZE(MEM_TYPE_DETAIL); i++) {
				if (CHECK_BIT(WORD_AT_OFFSET(memHeader, 0x13), i)) {
					AppendMultiString(&unicode, MEM_TYPE_DETAIL[i]);
				}
			}
			node_att_set_multi(deviceNode, _T("TypeDetails"), unicode, 0);
			LocalFree(unicode);

			//v 2.3+
			if (2 < smbios->SMBIOSMajorVersion || (2 == smbios->SMBIOSMajorVersion && 3 <= smbios->SMBIOSMinorVersion)) {
				// 0x15 Speed Mhz
				swprintf(buffer, _T("%u"), WORD_AT_OFFSET(memHeader, 0x15));
				node_att_set(deviceNode, _T("SpeedMhz"), buffer, NAFLG_FMT_NUMERIC);

				// 0x17 Manufacturer
				node_att_set(deviceNode, _T("Manufacturer"), GetSmbiosString(memHeader, BYTE_AT_OFFSET(memHeader, 0x17)), 0);

				// 0x18 Serial Number
				node_att_set(deviceNode, _T("SerialNumber"), GetSmbiosString(memHeader, BYTE_AT_OFFSET(memHeader, 0x18)), 0);

				// 0x19 Asset Tag
				node_att_set(deviceNode, _T("AssetTag"), GetSmbiosString(memHeader, BYTE_AT_OFFSET(memHeader, 0x19)), 0);

				// 0x1A Part Number
				node_att_set(deviceNode, _T("PartNumber"), GetSmbiosString(memHeader, BYTE_AT_OFFSET(memHeader, 0x1A)), 0);
			}
		}
		
		// Total array size
		totalSystemMb += totalArrayMb;
		swprintf(buffer, _T("%llu"), totalArrayMb);
		node_att_set(node, _T("TotalInstalledMb"), buffer, NAFLG_FMT_MBYTES);
	}

	// Total system size
	swprintf(buffer, _T("%llu"), totalSystemMb);
	node_att_set(parentNode, _T("TotalInstalledMb"), buffer, NAFLG_FMT_MBYTES);

	return parentNode;
}