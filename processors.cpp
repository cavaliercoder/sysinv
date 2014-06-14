#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include <intrin.h>

#define BUFLEN	128

// 7.5.1 Processor Information — Processor Type 
LPCTSTR PROC_TYPES[] = {
	_T("Invalid"),						// 0x00 Invalid
	_T("Other"),						// 0x01 Other
	_T("Unknown"),						// 0x02 Unknown
	_T("Central Processor"),			// 0x03 Central Processor
	_T("Math Processor"),				// 0x04 Math Processor
	_T("DSP Processor"),				// 0x05 DSP Processor
	_T("Video")							// 0x06 Video Processor
};

// 7.5.2 Processor Information — Processor Family 
PROCESSOR_FAMILY PROCESSOR_FAMILIES[] = {
		{ 0x01, _T("Other") },	// 1
		{ 0x02, _T("Unknown") },	// 2
		{ 0x03, _T("8086") },	// 3
		{ 0x04, _T("80286") },	// 4
		{ 0x05, _T("Intel386™ processor") },	// 5
		{ 0x06, _T("Intel486™ processor") },	// 6
		{ 0x07, _T("8087") },	// 7
		{ 0x08, _T("80287") },	// 8
		{ 0x09, _T("80387") },	// 9
		{ 0x0A, _T("80487") },	// 10
		{ 0x0B, _T("Intel® Pentium® processor") },	// 11
		{ 0x0C, _T("Pentium® Pro processor") },	// 12
		{ 0x0D, _T("Pentium® II processor") },	// 13
		{ 0x0E, _T("Pentium® processor with MMX™ technology") },	// 14
		{ 0x0F, _T("Intel® Celeron® processor") },	// 15
		{ 0x10, _T("Pentium® II Xeon™ processor") },	// 16
		{ 0x11, _T("Pentium® III processor") },	// 17
		{ 0x12, _T("M1 Family") },	// 18
		{ 0x13, _T("M2 Family") },	// 19
		{ 0x14, _T("Intel® Celeron® M processor") },	// 20
		{ 0x15, _T("Intel® Pentium® 4 HT processor") },	// 21
		{ 0x18, _T("AMD Duron™ Processor Family") },	// 24
		{ 0x19, _T("K5 Family") },	// 25
		{ 0x1A, _T("K6 Family") },	// 26
		{ 0x1B, _T("K6-2") },	// 27
		{ 0x1C, _T("K6-3") },	// 28
		{ 0x1D, _T("AMD Athlon™ Processor Family") },	// 29
		{ 0x1E, _T("AMD29000 Family") },	// 30
		{ 0x1F, _T("K6-2+") },	// 31
		{ 0x20, _T("Power PC Family") },	// 32
		{ 0x21, _T("Power PC 601") },	// 33
		{ 0x22, _T("Power PC 603") },	// 34
		{ 0x23, _T("Power PC 603+") },	// 35
		{ 0x24, _T("Power PC 604") },	// 36
		{ 0x25, _T("Power PC 620") },	// 37
		{ 0x26, _T("Power PC x704") },	// 38
		{ 0x27, _T("Power PC") },	// 39750
		{ 0x28, _T("Intel® Core™ Duo processor") },	// 40
		{ 0x29, _T("Intel® Core™ Duo mobile processor") },	// 41
		{ 0x2A, _T("Intel® Core™ Solo mobile processor") },	// 42
		{ 0x2B, _T("Intel® Atom™ processor") },	// 43
		{ 0x30, _T("Alpha Family") },	// 48
		{ 0x31, _T("Alpha 21064") },	// 49
		{ 0x32, _T("Alpha 21066") },	// 50
		{ 0x33, _T("Alpha 21164") },	// 51
		{ 0x34, _T("Alpha 21164PC") },	// 52
		{ 0x35, _T("Alpha 21164a") },	// 53
		{ 0x36, _T("Alpha 21264") },	// 54
		{ 0x37, _T("Alpha 21364") },	// 55
		{ 0x38, _T("AMD Turion™ II Ultra Dual-Core Mobile M Processor Family") },	// 56
		{ 0x39, _T("AMD Turion™ II Dual-Core Mobile M Processor Family") },	// 57
		{ 0x3A, _T("AMD Athlon™ II Dual-Core M Processor Family") },	// 58
		{ 0x3B, _T("AMD Opteron™ 6100 Series Processor") },	// 59
		{ 0x3C, _T("AMD Opteron™ 4100 Series Processor") },	// 60
		{ 0x3D, _T("AMD Opteron™ 6200 Series Processor") },	// 61
		{ 0x3E, _T("AMD Opteron™ 4200 Series Processor") },	// 62
		{ 0x3F, _T("AMD FX™ Series Processor") },	// 63
		{ 0x40, _T("MIPS Family") },	// 64
		{ 0x41, _T("MIPS R4000") },	// 65
		{ 0x42, _T("MIPS R4200") },	// 66
		{ 0x43, _T("MIPS R4400") },	// 67
		{ 0x44, _T("MIPS R4600") },	// 68
		{ 0x45, _T("MIPS R10000") },	// 69
		{ 0x46, _T("AMD C-Series Processor") },	// 70
		{ 0x47, _T("AMD E-Series Processor") },	// 71
		{ 0x48, _T("AMD A-Series Processor") },	// 72
		{ 0x49, _T("AMD G-Series Processor") },	// 73
		{ 0x4A, _T("AMD Z-Series Processor") },	// 74
		{ 0x4B, _T("AMD R-Series Processor") },	// 75
		{ 0x4C, _T("AMD Opteron™ 4300 Series Processor") },	// 76
		{ 0x4D, _T("AMD Opteron™ 6300 Series Processor") },	// 77
		{ 0x4E, _T("AMD Opteron™ 3300 Series Processor") },	// 78
		{ 0x4F, _T("AMD FirePro™ Series Processor") },	// 79
		{ 0x50, _T("SPARC Family") },	// 80
		{ 0x51, _T("SuperSPARC") },	// 81
		{ 0x52, _T("microSPARC II") },	// 82
		{ 0x53, _T("microSPARC IIep") },	// 83
		{ 0x54, _T("UltraSPARC") },	// 84
		{ 0x55, _T("UltraSPARC II") },	// 85
		{ 0x56, _T("UltraSPARC Iii") },	// 86
		{ 0x57, _T("UltraSPARC III") },	// 87
		{ 0x58, _T("UltraSPARC IIIi") },	// 88
		{ 0x60, _T("68040 Family") },	// 96
		{ 0x61, _T("68xxx") },	// 97
		{ 0x62, _T("68000") },	// 98
		{ 0x63, _T("68010") },	// 99
		{ 0x64, _T("68020") },	// 100
		{ 0x65, _T("68030") },	// 101
		{ 0x70, _T("Hobbit Family") },	// 112
		{ 0x78, _T("Crusoe™ TM5000 Family") },	// 120
		{ 0x79, _T("Crusoe™ TM3000 Family") },	// 121
		{ 0x7A, _T("Efficeon™ TM8000 Family") },	// 122
		{ 0x80, _T("Weitek") },	// 128
		{ 0x81, _T("Available for assignment") },	// 129
		{ 0x82, _T("Itanium™ processor") },	// 130
		{ 0x83, _T("AMD Athlon™ 64 Processor Family") },	// 131
		{ 0x84, _T("AMD Opteron™ Processor Family") },	// 132
		{ 0x85, _T("AMD Sempron™ Processor Family") },	// 133
		{ 0x86, _T("AMD Turion™ 64 Mobile Technology") },	// 134
		{ 0x87, _T("Dual-Core AMD Opteron™ Processor Family") },	// 135
		{ 0x88, _T("AMD Athlon™ 64 X2 Dual-Core Processor Family") },	// 136
		{ 0x89, _T("AMD Turion™ 64 X2 Mobile Technology") },	// 137
		{ 0x8A, _T("Quad-Core AMD Opteron™ Processor Family") },	// 138
		{ 0x8B, _T("Third-Generation AMD Opteron™ Processor Family") },	// 139
		{ 0x8C, _T("AMD Phenom™ FX Quad-Core Processor Family") },	// 140
		{ 0x8D, _T("AMD Phenom™ X4 Quad-Core Processor Family") },	// 141
		{ 0x8E, _T("AMD Phenom™ X2 Dual-Core Processor Family") },	// 142
		{ 0x8F, _T("AMD Athlon™ X2 Dual-Core Processor Family") },	// 143
		{ 0x90, _T("PA-RISC Family") },	// 144
		{ 0x91, _T("PA-RISC 8500") },	// 145
		{ 0x92, _T("PA-RISC 8000") },	// 146
		{ 0x93, _T("PA-RISC 7300LC") },	// 147
		{ 0x94, _T("PA-RISC 7200") },	// 148
		{ 0x95, _T("PA-RISC 7100LC") },	// 149
		{ 0x96, _T("PA-RISC 7100") },	// 150
		{ 0xA0, _T("V30 Family") },	// 160
		{ 0xA1, _T("Quad-Core Intel® Xeon® processor 3200 Series") },	// 161
		{ 0xA2, _T("Dual-Core Intel® Xeon® processor 3000 Series") },	// 162
		{ 0xA3, _T("Quad-Core Intel® Xeon® processor 5300 Series") },	// 163
		{ 0xA4, _T("Dual-Core Intel® Xeon® processor 5100 Series") },	// 164
		{ 0xA5, _T("Dual-Core Intel® Xeon® processor 5000 Series") },	// 165
		{ 0xA6, _T("Dual-Core Intel® Xeon® processor LV") },	// 166
		{ 0xA7, _T("Dual-Core Intel® Xeon® processor ULV") },	// 167
		{ 0xA8, _T("Dual-Core Intel® Xeon® processor 7100 Series") },	// 168
		{ 0xA9, _T("Quad-Core Intel® Xeon® processor 5400 Series") },	// 169
		{ 0xAA, _T("Quad-Core Intel® Xeon® processor") },	// 170
		{ 0xAB, _T("Dual-Core Intel® Xeon® processor 5200 Series") },	// 171
		{ 0xAC, _T("Dual-Core Intel® Xeon® processor 7200 Series") },	// 172
		{ 0xAD, _T("Quad-Core Intel® Xeon® processor 7300 Series") },	// 173
		{ 0xAE, _T("Quad-Core Intel® Xeon® processor 7400 Series") },	// 174
		{ 0xAF, _T("Multi-Core Intel® Xeon® processor 7400 Series") },	// 175
		{ 0xB0, _T("Pentium® III Xeon™ processor") },	// 176
		{ 0xB1, _T("Pentium® III Processor with Intel® SpeedStep™ Technology") },	// 177
		{ 0xB2, _T("Pentium® 4 Processor") },	// 178
		{ 0xB3, _T("Intel® Xeon® processor") },	// 179
		{ 0xB4, _T("AS400 Family") },	// 180
		{ 0xB5, _T("Intel® Xeon™ processor MP") },	// 181
		{ 0xB6, _T("AMD Athlon™ XP Processor Family") },	// 182
		{ 0xB7, _T("AMD Athlon™ MP Processor Family") },	// 183
		{ 0xB8, _T("Intel® Itanium® 2 processor") },	// 184
		{ 0xB9, _T("Intel® Pentium® M processor") },	// 185
		{ 0xBA, _T("Intel® Celeron® D processor") },	// 186
		{ 0xBB, _T("Intel® Pentium® D processor") },	// 187
		{ 0xBC, _T("Intel® Pentium® Processor Extreme Edition") },	// 188
		{ 0xBD, _T("Intel® Core™ Solo Processor") },	// 189
		{ 0xBE, _T("Reserved [3]") },	// 190
		{ 0xBF, _T("Intel® Core™ 2 Duo Processor") },	// 191
		{ 0xC0, _T("Intel® Core™ 2 Solo processor") },	// 192
		{ 0xC1, _T("Intel® Core™ 2 Extreme processor") },	// 193
		{ 0xC2, _T("Intel® Core™ 2 Quad processor") },	// 194
		{ 0xC3, _T("Intel® Core™ 2 Extreme mobile processor") },	// 195
		{ 0xC4, _T("Intel® Core™ 2 Duo mobile processor") },	// 196
		{ 0xC5, _T("Intel® Core™ 2 Solo mobile processor") },	// 197
		{ 0xC6, _T("Intel® Core™ i7 processor") },	// 198
		{ 0xC7, _T("Dual-Core Intel® Celeron® processor") },	// 199
		{ 0xC8, _T("IBM390 Family") },	// 200
		{ 0xC9, _T("G4") },	// 201
		{ 0xCA, _T("G5") },	// 202
		{ 0xCB, _T("ESA/390 G6") },	// 203
		{ 0xCC, _T("z/Architecture base") },	// 204
		{ 0xCD, _T("Intel® Core™ i5 processor") },	// 205
		{ 0xCE, _T("Intel® Core™ i3 processor") },	// 206
		{ 0xD2, _T("VIA C7™-M Processor Family") },	// 210
		{ 0xD3, _T("VIA C7™-D Processor Family") },	// 211
		{ 0xD4, _T("VIA C7™ Processor Family") },	// 212
		{ 0xD5, _T("VIA Eden™ Processor Family") },	// 213
		{ 0xD6, _T("Multi-Core Intel® Xeon® processor") },	// 214
		{ 0xD7, _T("Dual-Core Intel® Xeon® processor 3xxx Series") },	// 215
		{ 0xD8, _T("Quad-Core Intel® Xeon® processor 3xxx Series") },	// 216
		{ 0xD9, _T("VIA Nano™ Processor Family") },	// 217
		{ 0xDA, _T("Dual-Core Intel® Xeon® processor 5xxx Series") },	// 218
		{ 0xDB, _T("Quad-Core Intel® Xeon® processor 5xxx Series") },	// 219
		{ 0xDC, _T("Available for assignment") },	// 220
		{ 0xDD, _T("Dual-Core Intel® Xeon® processor 7xxx Series") },	// 221
		{ 0xDE, _T("Quad-Core Intel® Xeon® processor 7xxx Series") },	// 222
		{ 0xDF, _T("Multi-Core Intel® Xeon® processor 7xxx Series") },	// 223
		{ 0xE0, _T("Multi-Core Intel® Xeon® processor 3400 Series") },	// 224
		{ 0xE4, _T("AMD Opteron™ 3000 Series Processor") },	// 228
		{ 0xE5, _T("AMD Sempron™ II Processor") },	// 229
		{ 0xE6, _T("Embedded AMD Opteron™ Quad-Core Processor Family") },	// 230
		{ 0xE7, _T("AMD Phenom™ Triple-Core Processor Family") },	// 231
		{ 0xE8, _T("AMD Turion™ Ultra Dual-Core Mobile Processor Family") },	// 232
		{ 0xE9, _T("AMD Turion™ Dual-Core Mobile Processor Family") },	// 233
		{ 0xEA, _T("AMD Athlon™ Dual-Core Processor Family") },	// 234
		{ 0xEB, _T("AMD Sempron™ SI Processor Family") },	// 235
		{ 0xEC, _T("AMD Phenom™ II Processor Family") },	// 236
		{ 0xED, _T("AMD Athlon™ II Processor Family") },	// 237
		{ 0xEE, _T("Six-Core AMD Opteron™ Processor Family") },	// 238
		{ 0xEF, _T("AMD Sempron™ M Processor Family") },	// 239
		{ 0xFA, _T("i860") },	// 250
		{ 0xFB, _T("i960") },	// 251
		{ 0x104, _T("SH-3") },	// 260
		{ 0x105, _T("SH-4") },	// 261
		{ 0x118, _T("ARM") },	// 280
		{ 0x119, _T("StrongARM") },	// 281
		{ 0x12C, _T("6x86") },	// 300
		{ 0x12D, _T("MediaGX") },	// 301
		{ 0x12E, _T("MII") },	// 302
		{ 0x140, _T("WinChip") },	// 320
		{ 0x15E, _T("DSP") },	// 350
		{ 0x1F4, _T("Video Processor") }	// 500
};

// 7.5.5 Processor Information — Processor Upgrade
LPTSTR PROCESSOR_UPGRADE[] = {
	_T("Invalid"),						// 0x00 Invalid
	_T("Other"),						// 0x01
	_T("Unknown"),						// 0x02
	_T("Daughter Board"),				// 0x03
	_T("ZIF Socket"),					// 0x04
	_T("Replaceable Piggy Back"),		// 0x05
	_T("None"),							// 0x06
	_T("LIF Socket"),					// 0x07
	_T("Slot 1"),						// 0x08
	_T("Slot 2"),						// 0x09
	_T("370-pin socket"),				// 0x0A
	_T("Slot A"),						// 0x0B
	_T("Slot"),							// 0x0CM
	_T("Socket 423"),					// 0x0D
	_T("Socket A (Socket 462)"),		// 0x0E
	_T("Socket 478"),					// 0x0F
	_T("Socket 754"),					// 0x10
	_T("Socket 940"),					// 0x11
	_T("Socket 939"),					// 0x12
	_T("Socket mPGA604"),				// 0x13
	_T("Socket LGA771"),				// 0x14
	_T("Socket LGA775"),				// 0x15
	_T("Socket S1"),					// 0x16
	_T("Socket AM2"),					// 0x17
	_T("Socket F (1207)"),				// 0x18
	_T("Socket LGA1366"),				// 0x19
	_T("Socket G34"),					// 0x1A
	_T("Socket AM3"),					// 0x1B
	_T("Socket C32"),					// 0x1C
	_T("Socket LGA1156"),				// 0x1D
	_T("Socket LGA1567"),				// 0x1E
	_T("Socket PGA988A"),				// 0x1F
	_T("Socket BGA1288"),				// 0x20
	_T("Socket rPGA988B"),				// 0x21
	_T("Socket BGA1023"),				// 0x22
	_T("Socket BGA1224"),				// 0x23
	_T("Socket LGA1155"),				// 0x24
	_T("Socket LGA1356"),				// 0x25
	_T("Socket LGA2011"),				// 0x26
	_T("Socket FS1"),					// 0x27
	_T("Socket FS2"),					// 0x28
	_T("Socket FM1"),					// 0x29
	_T("Socket FM2"),					// 0x2A
	_T("Socket LGA2011-3"),				// 0x2B
	_T("Socket LGA1356-3")				// 0x2C
};

// 7.5.9 Processor Characteristics 
LPCTSTR PROCESSOR_CHARACTERISTICS[] = {
	_T("Reserved"),						// Bit 0
	_T("Unknown"),						// Bit 1
	_T("64-bit Capable"),				// Bit 2
	_T("Multi-Core"),					// Bit 3
	_T("Hardware Thread"),				// Bit 4
	_T("Execute Protection"),			// Bit 5
	_T("Enhanced Virtualization"),		// Bit 6
	_T("Power/Performance Control")		// Bit 7
};

// Intel Table 3-20
PROCESSOR_FEATURE PROC_FEATURES_ECX[] = {
		{ 0, _T("SSE3"), _T("Streaming SIMD Extensions 3 ") },
		{ 1, _T("PCLMULQDQ"), _T("PCLMULQDQ instruction") },
		{ 2, _T("DTES64 "), _T("64-bit DS Area") },
		{ 3, _T("MONITOR "), _T("MONITOR/MWAIT") },
		{ 4, _T("DS-CPL"), _T("CPL Qualified Debug Store") },
		{ 5, _T("VMX"), _T("Virtual Machine Extensions") },
		{ 6, _T("SMX"), _T("Safer Mode Extensions") },
		{ 7, _T("EIST"), _T("Enhanced Intel SpeedStep® technology") },
		{ 8, _T("TM2"), _T("Thermal Monitor 2") },
		{ 9, _T("SSSE3"), _T("Supplemental Streaming SIMD Extensions 3") },
		{ 10, _T("CNXT-ID"), _T("L1 Context ID") },
		{ 11, _T("SDBG"), _T("IA32_DEBUG_INTERFACE MSR for silicon debug") },
		{ 12, _T("FMA"), _T("FMA extensions using YMM state") },
		{ 13, _T("CMPXCHG16B"), _T("CMPXCHG16B Available") },
		{ 14, _T("xTPR"), _T("xTPR Update Control") },
		{ 15, _T("PDCM"), _T("Perfmon and Debug Capability") },
		{ 17, _T("PCID"), _T("Process-context identifiers") },
		{ 18, _T("DCA"), _T("Prefetch data from a memory mapped device") },
		{ 19, _T("SSE4.1"), _T("SSE4.1") },
		{ 20, _T("SSE4.2"), _T("SSE4.2") },
		{ 21, _T("x2APIC"), _T("x2APIC feature") },
		{ 22, _T("MOVBE"), _T("MOVBE instruction") },
		{ 23, _T("POPCNT"), _T("POPCNT instruction") },
		{ 24, _T("TSC-Deadline"), _T("APIC timer supports one-shot operation using a TSC deadline value") },
		{ 25, _T("AESNI"), _T("AESNI instruction extensions") },
		{ 26, _T("XSAVE"), _T("XSAVE/XRSTOR processor extended states feature, XSETBV/XGETBV instructions, and XCR0") },
		{ 27, _T("OSXSAVE"), _T("OS has set CR4.OSXSAVE[bit 18] to enable the XSAVE feature set") },
		{ 28, _T("AVX"), _T("AVX instruction extensions") },
		{ 29, _T("F16C"), _T("16-bit floating-point conversion instructions") },
		{ 30, _T("RDRAND"), _T("RDRAND instruction") }
};

// Intel Table 3-21
PROCESSOR_FEATURE PROC_FEATURES_EDX[] = {
		{ 0, _T("FPU"), _T("x87 FPU on Chip") },
		{ 1, _T("VME"), _T("Virtual-8086 Mode Enhancement") },
		{ 2, _T("DE"), _T("Debugging Extensions") },
		{ 3, _T("PSE"), _T("Page Size Extensions") },
		{ 4, _T("TSC"), _T("Time Stamp Counter") },
		{ 5, _T("MSR"), _T("RDMSR and WRMSR Support") },
		{ 6, _T("PAE"), _T("Physical Address Extensions") },
		{ 7, _T("MCE"), _T("Machine Check Exception") },
		{ 8, _T("CX8"), _T("CMPXCHG8B Inst.") },
		{ 9, _T("APIC"), _T("APIC on Chip") },
		{ 11, _T("SEP"), _T("SYSENTER and SYSEXIT") },
		{ 12, _T("MTRR"), _T("Memory Type Range Registers") },
		{ 13, _T("PGE"), _T("PTE Global Bit") },
		{ 14, _T("MCA"), _T("Machine Check Architecture") },
		{ 15, _T("CMOV"), _T("Conditional Move/Compare Instruction") },
		{ 16, _T("PAT"), _T("Page Attribute Table") },
		{ 17, _T("PSE"), _T("Page Size Extension") },
		{ 18, _T("PSN"), _T("Processor Serial Number") },
		{ 19, _T("CLFSH"), _T("CFLUSH Instruction") },
		{ 21, _T("DS"), _T("Debug Store") },
		{ 22, _T("ACPI"), _T("Thermal Monitor and Clock Ctrl") },
		{ 23, _T("MMX"), _T("MMX Technology") },
		{ 24, _T("FXSR"), _T("FXSAVE/FXRSTOR") },
		{ 25, _T("SSE"), _T("SSE Extensions") },
		{ 26, _T("SSE2"), _T("SSE2 Extensions") },
		{ 27, _T("SS"), _T("Self Snoop") },
		{ 28, _T("HTT"), _T("Hyper-threading technology") },
		{ 29, _T("TM"), _T("Thermal Monitor") },
		{ 31, _T("PBE"), _T("Pend. Brk. En.") }
};

/*
 * Microsoft Specification:
 * http://msdn.microsoft.com/en-us/library/hskdteyh(VS.80).aspx
 *
 * Intel specification (Chapter 3-157, Table 3-17):
 * http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf
 * https://software.intel.com/en-us/articles/intel-architecture-and-processor-identification-with-cpuid-model-and-family-numbers
 *
 * AMD Specification:
 * http://developer.amd.com/wordpress/media/2012/10/254811.pdf
 */
PNODE EnumProcessors()
{
	int cpuinfo[4];
	int i;
	int nExIds;
	char buffer[BUFLEN];
	TCHAR strBuffer[BUFLEN];
	LPTSTR unicode = NULL;
	PNODE cpuNode = node_alloc(L"Processors", NODE_FLAG_TABLE);
	PNODE featuresNode = node_append_new(cpuNode, _T("Features"), NODE_FLAG_TABLE);
	PNODE node = NULL;

	DWORD dwBuffer = NULL;
	DWORD dwBuffer2 = NULL;

	// Get cpu manufacturor (EAX = 0)
	__cpuid(cpuinfo, 0x00);
	memset(buffer, 0, BUFLEN);
	memcpy(&buffer[0], &cpuinfo[EBX], 4);
	memcpy(&buffer[4], &cpuinfo[EDX], 4);
	memcpy(&buffer[8], &cpuinfo[ECX], 4);
	MultiByteToWideChar(CP_UTF8, 0, buffer, -1, strBuffer, BUFLEN);
	node_att_set(cpuNode, L"Manufacturer", strBuffer, 0);

	__cpuid(cpuinfo, 0x01);
	
	// EAX (Intel figure 3-5)
	swprintf(strBuffer, L"%u", (cpuinfo[EAX] & 0xF));
	node_att_set(cpuNode, L"Stepping", strBuffer, 0);

	swprintf(strBuffer, L"%u", (cpuinfo[EAX]  >> 12) & 0xF);
	node_att_set(cpuNode, L"Type", strBuffer, 0);

	dwBuffer = (cpuinfo[EAX] >> 8) & 0xF; // Family
	dwBuffer2 = (cpuinfo[EAX] >> 20) & 0xFF; // Extended Family
	if (0x0F == dwBuffer)
		dwBuffer += dwBuffer2;

	swprintf(strBuffer, _T("0x%02X"), dwBuffer);
	node_att_set(cpuNode, L"Family", strBuffer, 0);

	if (0x0F == dwBuffer || 0x06 == dwBuffer)
		dwBuffer = (((cpuinfo[EAX] >> 16) & 0xF) << 4) + ((cpuinfo[EAX] >> 4) & 0xF);
	else
		dwBuffer = (cpuinfo[EAX] >> 4) & 0xF;

	swprintf(strBuffer, L"0x%02X", dwBuffer);
	node_att_set(cpuNode, L"Model", strBuffer, 0);

	// EBX
	swprintf(strBuffer, _T("%u"), 8 * ((cpuinfo[EBX] >> 8) & 0x7F));
	node_att_set(cpuNode, _T("CacheLineSize"), strBuffer, 0);

	swprintf(strBuffer, _T("%u"), (cpuinfo[EBX] >> 24) & 0x7F);
	node_att_set(cpuNode, _T("ApicPhysicalId"), strBuffer, 0);

	// ECX Feature Info (Intel table 3-21)
	for (i = 0; i < ARRAYSIZE(PROC_FEATURES_ECX); i++) {
		if (CHECK_BIT(cpuinfo[ECX], PROC_FEATURES_ECX[i].BitNumber)) {
			node = node_append_new(featuresNode, _T("Feature"), NODE_FLAG_TABLE_ENTRY);
			node_att_set(node, _T("Feature"), PROC_FEATURES_ECX[i].Code, NODE_ATT_FLAG_KEY);
			node_att_set(node, _T("Description"), PROC_FEATURES_ECX[i].Description, 0);
		}
	}

	// EDX Extended Feature info (Intel table 3-21)
	for (i = 0; i < ARRAYSIZE(PROC_FEATURES_EDX); i++) {
		if (CHECK_BIT(cpuinfo[EDX], PROC_FEATURES_EDX[i].BitNumber)) {
			node = node_append_new(featuresNode, _T("Feature"), NODE_FLAG_TABLE_ENTRY);
			node_att_set(node, _T("Feature"), PROC_FEATURES_EDX[i].Code, NODE_ATT_FLAG_KEY);
			node_att_set(node, _T("Description"), PROC_FEATURES_EDX[i].Description, 0);
		}	
	}

	// Get CPU Brand String
	memset(buffer, 0, BUFLEN);
	__cpuid(cpuinfo, 0x80000000);
	nExIds = cpuinfo[EAX];
	for(i = 0x80000000; i < nExIds; i++) {
		__cpuid(cpuinfo, i);
		switch(i) {
		case 0x80000002:
			memcpy(&buffer[0], cpuinfo, sizeof(cpuinfo));
			break;

		case 0x80000003:
			memcpy(&buffer[16], cpuinfo, sizeof(cpuinfo));
			break;

		case 0x80000004:
			memcpy(&buffer[32], cpuinfo, sizeof(cpuinfo));
			break;
		}
	}
	MultiByteToWideChar(CP_UTF8, 0, buffer, -1, strBuffer, BUFLEN);
	node_att_set(cpuNode, L"BrandString", strBuffer, 0);

	return cpuNode;
}

// SMBIOS Table Type 4
PNODE EnumProcSockets()
{
	PNODE procSocketsNode = node_alloc(_T("ProcessorSockets"), 0);
	PNODE node = NULL;

	PRAW_SMBIOS_DATA smbios = GetSmbiosData();
	PSMBIOS_STRUCT_HEADER header = NULL;

	LPTSTR unicode = NULL;
	TCHAR buffer[MAX_PATH + 1];
	DWORD i = 0;
	DWORD dwBuffer = 0;

	while (NULL != (header = GetNextStructureOfType(header, SMB_TABLE_PROCESSOR))) {
		// v2.0+
		if (2 <= smbios->SMBIOSMajorVersion) {
			// Ignore unpopulated sockets
			// Bit 6 = CPU Sock Populated
			if (!(BYTE_AT_OFFSET(header, 0x18) >> 6))
				return node;

			node = node_append_new(procSocketsNode, _T("ProcessorSockets"), 0);

			// 0x04 Designation
			unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x04));
			node_att_set(node, _T("Designation"), unicode, 0);
			LocalFree(unicode);

			// 0x05 Processor Type
			node_att_set(node, _T("Type"), SAFE_INDEX(PROC_TYPES, BYTE_AT_OFFSET(header, 0x05)), 0);

			// 0x06 Processor Family
			for (i = 0; i < ARRAYSIZE(PROCESSOR_FAMILIES); i++) {
				if (BYTE_AT_OFFSET(header, 0x06) == PROCESSOR_FAMILIES[i].Index)
					node_att_set(node, _T("Family"), PROCESSOR_FAMILIES[i].Name, 0);
			}

			// 0x07 Manufacturer
			unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x07));
			node_att_set(node, _T("Manufacturer"), unicode, 0);
			LocalFree(unicode);

			// 0x10 Processor Version String
			unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x10));
			node_att_set(node, _T("Version"), unicode, 0);
			LocalFree(unicode);

			// 0x11 Voltage
			dwBuffer = BYTE_AT_OFFSET(header, 0x11);
			if (CHECK_BIT(dwBuffer, 7)) {
				// Remaining bits are voltage * 10
				dwBuffer &= (1 << 7);
				swprintf(buffer, _T("%f"), (float)dwBuffer / 10.0);
				node_att_set(node, _T("Voltage"), buffer, 0);
			}

			else {
				// Legacy mode
				unicode = NULL;
				if (CHECK_BIT(dwBuffer, 0))
					AppendMultiString(&unicode, _T("5"));
				if (CHECK_BIT(dwBuffer, 1))
					AppendMultiString(&unicode, _T("3.3"));
				if (CHECK_BIT(dwBuffer, 2))
					AppendMultiString(&unicode, _T("2.9"));

				node_att_set_multi(node, _T("SupportedVoltages"), unicode, 0);
				LocalFree(unicode);
			}

			// 0x12 External Clock Speed Mhz
			if (0 != WORD_AT_OFFSET(header, 0x12)) {
				swprintf(buffer, _T("%u"), WORD_AT_OFFSET(header, 0x12));
				node_att_set(node, _T("ExternalClockMhz"), buffer, 0);
			}

			// 0x14 Max Speed
			if (0 != WORD_AT_OFFSET(header, 0x14)) {
				swprintf(buffer, _T("%u"), WORD_AT_OFFSET(header, 0x14));
				node_att_set(node, _T("MaxSpeedMhz"), buffer, 0);
			}

			// 0x16 Current Speed
			if (0 != WORD_AT_OFFSET(header, 0x16)) {
				swprintf(buffer, _T("%u"), WORD_AT_OFFSET(header, 0x16));
				node_att_set(node, _T("CurrentSpeedMhz"), buffer, 0);
			}

			// 0x18 Status
			dwBuffer = BYTE_AT_OFFSET(header, 0x18) & 0x07;
			if (0x01 == (0x01 & dwBuffer))
				node_att_set(node, _T("Status"), _T("CPU Enabled"), 0);
			else if (0x02 == (0x02 & dwBuffer))
				node_att_set(node, _T("Status"), _T("CPU Disabled by User through BIOS setup"), 0);
			else if (0x03 == (0x03 & dwBuffer))
				node_att_set(node, _T("Status"), _T("CPU Disabled By BIOS (POST Error)"), 0);

			else if (0x04 == (0x04 & dwBuffer))
				node_att_set(node, _T("Status"), _T("CPU is Idle, waiting to be enabled"), 0);

			// 0x19 Processor Upgrade
			node_att_set(node, _T("SocketType"), SAFE_INDEX(PROCESSOR_UPGRADE, BYTE_AT_OFFSET(header, 0x19)), 0);

			// v2.3+
			if (2 < smbios->SMBIOSMajorVersion || (2 == smbios->SMBIOSMajorVersion && 3 <= smbios->SMBIOSMinorVersion)) {
				// 0x20 Serial Number
				unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x20));
				node_att_set(node, _T("SerialNumber"), unicode, 0);
				LocalFree(unicode);

				// 0x21 Asset Tag
				unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x21));
				node_att_set(node, _T("AssetTag"), unicode, 0);
				LocalFree(unicode);

				// 0x22 Part Number
				unicode = GetSmbiosString(header, BYTE_AT_OFFSET(header, 0x22));
				node_att_set(node, _T("PartNumber"), unicode, 0);
				LocalFree(unicode);
			}

			// v2.5+
			if (2 < smbios->SMBIOSMajorVersion || (2 == smbios->SMBIOSMajorVersion && 5 <= smbios->SMBIOSMinorVersion)) {
				// 0x23 Core Count
				swprintf(buffer, _T("%u"), BYTE_AT_OFFSET(header, 0x23));
				node_att_set(node, _T("CoreCount"), buffer, 0);

				// 0x24 Enabled Count
				swprintf(buffer, _T("%u"), BYTE_AT_OFFSET(header, 0x24));
				node_att_set(node, _T("EnabledCoreCount"), buffer, 0);

				// 0x25 Thread Count
				swprintf(buffer, _T("%u"), BYTE_AT_OFFSET(header, 0x25));
				node_att_set(node, _T("ThreadsPerCore"), buffer, 0);

				// 0x26 Characteristics
				unicode = NULL;
				dwBuffer = BYTE_AT_OFFSET(header, 0x26);
				for (i = 0; i < ARRAYSIZE(PROCESSOR_CHARACTERISTICS); i++) {
					if (CHECK_BIT(dwBuffer, i))
						AppendMultiString(&unicode, PROCESSOR_CHARACTERISTICS[i]);
				}
				node_att_set_multi(node, _T("Characteristics"), unicode, 0);
				LocalFree(unicode);
			}
		}
	}

	return procSocketsNode;
}