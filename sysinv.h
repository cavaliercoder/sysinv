#include "stdafx.h"
#include "common.h"
#include "node.h"

// Hardware
PNODE GetSystemDetail();
PNODE EnumSmbiosTables();
PNODE GetVirtualizationDetail();
PNODE EnumProcessors();
PNODE EnumDisks();

// Software
PNODE GetAgentDetail();
PNODE GetOperatingSystemDetail();
PNODE EnumPackages();
PNODE EnumHotfixes();

// Configuration
PNODE EnumNetworkInterfaces();
PNODE EnumNetworkRoutes();
PNODE EnumVolumes();
PNODE EnumClusterServices();

// SMBIOS functions
PNODE GetSmbiosDetail();
PNODE GetBiosDetail();
PNODE EnumBaseboards();
PNODE EnumChassis();
PNODE EnumMemorySockets();
PNODE EnumProcSockets();
PNODE EnumOemStrings();