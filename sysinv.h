#include "stdafx.h"
#include "common.h"
#include "node.h"
#include "smbios.h"

PNODE EnumSmbiosTables();
PNODE EnumPackages();
PNODE EnumProcessors();
PNODE EnumDisks();
PNODE EnumVolumes();
PNODE EnumClusterServices();

PNODE GetAgentDetail();
PNODE GetSystemDetail();
PNODE GetOperatingSystemDetail();

// SMBIOS functions
PNODE GetSmbiosDetail();
PNODE GetBiosDetail();
PNODE GetBiosSystemDetail();
PNODE EnumBaseboards();
PNODE EnumSlots();
PNODE EnumChassis();
PNODE EnumProcSockets();
PNODE EnumOemStrings();