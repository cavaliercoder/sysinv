#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"
#include "virtualization.h"

PLOOKUP_ENTRY VIRT_PLATFORM = NULL;

LOOKUP_ENTRY VIRT_PHYSICAL = { VIRT_PLATFORM_NONE, NULL, _T("Physical") };

LOOKUP_ENTRY VIRT_UNKNOWN = { VIRT_PLATFORM_NONE, NULL, _T("Unknown") };

LOOKUP_ENTRY VIRT_VENDORS[] = {
		{ VIRT_PLATFORM_VMWARE, _T("vmware"), _T("VMware") },
		{ VIRT_PLATFORM_VBOX, _T("virtualbox"), _T("VirtualBox") },
		{ VIRT_PLATFORM_KVM, _T("kvm"), _T("KVM") },
		{ VIRT_PLATFORM_QEMU, _T("bochs"), _T("Qemu (Emulated") },
		{ VIRT_PLATFORM_MSVPC, _T("virtual machine"), _T("Microsoft Virtual PC") },
		{ VIRT_PLATFORM_HYPERV, _T("hyper"), _T("Microsoft Hyper-V") },
		{ VIRT_PLATFORM_XEN, _T("hvm domu"), _T("Citrix Xen") },
		{ VIRT_PLATFORM_XEN, _T("xen"), _T("Citrix Xen") },
		{ VIRT_PLATFORM_PARA, _T("parallels"), _T("Parallels") },
		{ VIRT_PLATFORM_VTZO, _T("virtuozzo"), _T("Virtuozzo") }
};

BOOL IsVirtualized()
{
	PLOOKUP_ENTRY platform = GetVirtualizationPlatform();
	return platform->Index != VIRT_PLATFORM_NONE;
}

PLOOKUP_ENTRY GetVirtualizationPlatform() {
	DWORD i = 0;
	DWORD dwVirtPlatform = VIRT_PLATFORM_NONE;
	PSMBIOS_STRUCT_HEADER sysTable = GetNextStructureOfType(NULL, SMB_TABLE_SYSTEM);
	LPTSTR szManufacturer = NULL;
	LPTSTR szProduct = NULL;

	// Return cached result
	if (NULL != VIRT_PLATFORM)
		return VIRT_PLATFORM;

	// Validate SMBIOS data 
	if (NULL == sysTable) {
		SetError(ERR_CRIT, 0, _T("Unable to determine Virtualization status from SBMIOS System Table (Type %u)"), SMB_TABLE_SYSTEM);
		VIRT_PLATFORM = &VIRT_UNKNOWN;
		return VIRT_PLATFORM;
	}

	// Default to physical
	VIRT_PLATFORM = &VIRT_PHYSICAL;

	// Search for platform names in SMBIOS System Manufacturer and Product strings
	szManufacturer = GetSmbiosString(sysTable, BYTE_AT_OFFSET(sysTable, 0x04));
	szProduct = GetSmbiosString(sysTable, BYTE_AT_OFFSET(sysTable, 0x05));
	for (i = 0; i < ARRAYSIZE(VIRT_VENDORS); i++)
	{
		if (NULL != wcsistr(szManufacturer, VIRT_VENDORS[i].Code) || NULL != wcsstr(szProduct, VIRT_VENDORS[i].Code)) {
			VIRT_PLATFORM = &VIRT_VENDORS[i];
			break;
		}
	}
	LocalFree(szManufacturer);
	LocalFree(szProduct);

	return VIRT_PLATFORM;
}

PNODE GetVirtualizationDetail()
{
	PLOOKUP_ENTRY platform = GetVirtualizationPlatform();
	PNODE virtNode = node_alloc(_T("Virtualization"), 0);
	if (&VIRT_UNKNOWN == VIRT_PLATFORM) {
		node_att_set(virtNode, _T("Status"), _T("Unknown"), 0);
	}
	else {
		if (VIRT_PLATFORM_NONE == platform->Index) {
			node_att_set(virtNode, _T("Status"), _T("Physical"), 0);
		}

		else {
			node_att_set(virtNode, _T("Status"), _T("Virtual"), 0);
			node_att_set(virtNode, _T("Platform"), platform->Description, 0);
		}
	}
	return virtNode;
}