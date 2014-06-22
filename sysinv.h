#include "stdafx.h"
#include "common.h"
#include "node.h"

// Hardware
PNODE EnumSmbiosTables();
PNODE EnumPackages();
PNODE EnumProcessors();
PNODE EnumDisks();

// Software
PNODE GetAgentDetail();
PNODE GetSystemDetail();
PNODE GetOperatingSystemDetail();
PNODE GetVirtualizationDetail();

// Configuration
PNODE EnumNetworkInterfaces();
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