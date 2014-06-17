#include "stdafx.h"
#include "common.h"
#include "node.h"

// Hardware
PNODE EnumSmbiosTables();
PNODE EnumPackages();
PNODE EnumProcessors();
PNODE EnumDisks(); 
PNODE EnumNetworkAdapters();

PNODE EnumVolumes();
PNODE EnumClusterServices();

// Software
PNODE GetAgentDetail();
PNODE GetSystemDetail();
PNODE GetOperatingSystemDetail();
PNODE GetVirtualizationDetail();
// Configuration


// SMBIOS functions
PNODE GetSmbiosDetail();
PNODE GetBiosDetail();
PNODE EnumBaseboards();
PNODE EnumChassis();
PNODE EnumMemorySockets();
PNODE EnumProcSockets();
PNODE EnumOemStrings();