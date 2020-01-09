#ifndef _INCLUDE_IL4DMATCHEXT_EXTENSION_PROPER_H_
#define _INCLUDE_IL4DMATCHEXT_EXTENSION_PROPER_H_

#include "smsdk_ext.h"

#include <KeyValues.h>
#include "public/matchmaking/imatchframework.h"
#include "IL4DMatchExtData.h"

#include "public/matchmaking/l4d/imatchext_l4d.h"

extern IMatchFramework *g_pMatchFramework;
extern IMatchExtL4D *g_pMatchExtL4D;

#if defined(KE_WINDOWS)
# define MATCHMAKING_DS_LIB "matchmaking_ds.dll"
#else
# define MATCHMAKING_DS_LIB "matchmaking_ds_srv.so"
#endif

class CMatchExtL4D :
	public SDKExtension,
	public IL4DMatchExtData
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);

public:
	virtual KeyValues *GetAllMissions();
	virtual KeyValues *GetAllModes();
	virtual KeyValues *GetMapInfo(KeyValues *pSettings, KeyValues **ppMissionInfo);
	virtual KeyValues *GetMapInfoByBspName(KeyValues *pSettings, const char *szBspMapName, KeyValues **ppMissionInfo);
	virtual KeyValues *GetGameModeInfo(const char *szGameModeName);
	virtual KeyValues *GetActiveServerDetails(KeyValues *pRequest = NULL);
};

#endif // _INCLUDE_IL4DMATCHEXT_EXTENSION_PROPER_H_
