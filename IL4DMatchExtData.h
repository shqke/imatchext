#ifndef _INCLUDE_IMATCHEXTL4D_H_
#define _INCLUDE_IMATCHEXTL4D_H_

#include <IShareSys.h>
#include <KeyValues.h>

#define SMINTERFACE_MATCHMAKINGDATA_NAME		"IL4DMatchExtData"
#define SMINTERFACE_MATCHMAKINGDATA_VERSION		10

namespace SourceMod
{
	class IL4DMatchExtData : public SMInterface
	{
	public:
		virtual const char *GetInterfaceName() {
			return SMINTERFACE_MATCHMAKINGDATA_NAME;
		}
		
		virtual unsigned int GetInterfaceVersion() {
			return SMINTERFACE_MATCHMAKINGDATA_VERSION;
		}

	public:
		virtual KeyValues *GetAllMissions() = 0;
		virtual KeyValues *GetAllModes() = 0;
		virtual KeyValues *GetMapInfo(KeyValues *pSettings, KeyValues **ppMissionInfo) = 0;
		virtual KeyValues *GetMapInfoByBspName(KeyValues *pSettings, const char *szBspMapName, KeyValues **ppMissionInfo) = 0;
		virtual KeyValues *GetGameModeInfo(const char *szGameModeName) = 0;
		virtual KeyValues *GetActiveServerDetails(KeyValues *pRequest = NULL) = 0;
	};
};

#endif // _INCLUDE_IMATCHEXTL4D_H_
