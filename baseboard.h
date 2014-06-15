#include "stdafx.h"
#include "sysinv.h"
#include "common.h"
#include "smbios.h"

PNODE EnumSlots(WORD parentHandle);
PNODE GetSlotDetail(PRAW_SMBIOS_DATA smbios, PSMBIOS_STRUCT_HEADER header);
PNODE GetPortDetail(PRAW_SMBIOS_DATA smbios, PSMBIOS_STRUCT_HEADER header);
