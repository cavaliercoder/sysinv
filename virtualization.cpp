#include "stdafx.h"
#include "sysinv.h"
#include "smbios.h"


#define VIRT_PLATFORM_NONE			0
#define VIRT_PLATFORM_VMWARE		1
#define VIRT_PLATFORM_VBOX			2
#define VIRT_PLATFORM_QEMU			3
#define VIRT_PLATFORM_KVM			4
#define VIRT_PLATFORM_MSVPC			5
#define VIRT_PLATFORM_HYPERV		6
#define VIRT_PLATFORM_XEN			7
#define VIRT_PLATFORM_VTZO			8
#define VIRT_PLATFORM_PARA			9

LOOKUP_ENTRY VIRT_VENDORS[] = {
		{ VIRT_PLATFORM_VMWARE, _T("vmware"), _T("VMWare") },
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

PNODE GetVirtualizationDetail()
{
	DWORD i = 0;
	DWORD dwVirtPlatform = VIRT_PLATFORM_NONE;
	PNODE virtNode = node_alloc(_T("Virtualization"), 0);
	PSMBIOS_STRUCT_HEADER sysTable = GetNextStructureOfType(NULL, SMB_TABLE_SYSTEM);
	LPTSTR szManufacturer = NULL;
	LPTSTR szProduct = NULL;


	if (NULL == sysTable) {
		SetError(ERR_CRIT, 0, _T("Unable to determine Virtualization status from SBMIOS System Table (Type %u)"), SMB_TABLE_SYSTEM);
		node_att_set(virtNode, _T("Status"), _T("Unknown"), 0);
		return virtNode;
	}

	szManufacturer = GetSmbiosString(sysTable, BYTE_AT_OFFSET(sysTable, 0x04));
	szProduct = GetSmbiosString(sysTable, BYTE_AT_OFFSET(sysTable, 0x05));

	for (i = 0; i < ARRAYSIZE(VIRT_VENDORS); i++)
	{
		if (NULL != wcsstr(szManufacturer, VIRT_VENDORS[i].Code) || NULL != wcsstr(szProduct, VIRT_VENDORS[i].Code)) {
			dwVirtPlatform = VIRT_VENDORS[i].Index;
			node_att_set(virtNode, _T("Platform"), VIRT_VENDORS[i].Description, 0);
			break;
		}
	}

	LocalFree(szManufacturer);
	LocalFree(szProduct);

	if (VIRT_PLATFORM_NONE == dwVirtPlatform)
		node_att_set(virtNode, _T("Status"), _T("Physical"), 0);
	else
		node_att_set(virtNode, _T("Status"), _T("Virtual"), 0);

	return virtNode;
}