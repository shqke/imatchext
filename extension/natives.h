#ifndef _INCLUDE_NATIVES_H_
#define _INCLUDE_NATIVES_H_

#include "extension.h"

#include <vstdlib/IKeyValuesSystem.h>
#include <sp_vm_api.h>

extern const sp_nativeinfo_t g_Natives[];

extern HKeySymbol GetCurrentMode();
extern HKeySymbol GetCurrentMission();

#endif // _INCLUDE_NATIVES_H_
