#include "stdafx.h"
#include "common.h"
#include "node.h"

PNODE EnumSmbiosTables();
PNODE EnumPackages();
PNODE EnumProcessors();
PNODE EnumDisks();
PNODE EnumVolumes();
PNODE EnumClusterServices();

PNODE GetAgentDetail();
PNODE GetSystemDetail();
PNODE GetOperatingSystemDetail();