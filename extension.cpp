#include "extension.h"
#include "natives.h"

CMatchExtL4D g_MatchExtL4D;
SMEXT_LINK(&g_MatchExtL4D);

IMatchFramework *g_pMatchFramework = NULL;
IMatchExtL4D *g_pMatchExtL4D = NULL;

bool CMatchExtL4D::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if (strncmp(g_pSM->GetGameFolderName(), "left4dead", 9)) {
		strncpy(error, "Only supports Left 4 Dead and Left 4 Dead 2", maxlength);

		return false;
	}

	char path[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_Game, path, sizeof(path), "bin/" MATCHMAKING_DS_LIB);

	CDllDemandLoader dll(path);

	CreateInterfaceFn matchmakingFactory = dll.GetFactory();
	if (!matchmakingFactory) {
		strncpy(error, "Unable to retrieve matchmaking factory", maxlength);

		return false;
	}

	g_pMatchFramework = static_cast<IMatchFramework *>(matchmakingFactory(IMATCHFRAMEWORK_VERSION_STRING, NULL));
	if (!g_pMatchFramework) {
		strncpy(error, "Unable to retrieve matchmaking framework interface", maxlength);

		return false;
	}

	g_pMatchExtL4D = static_cast<IMatchExtL4D *>(matchmakingFactory(IMATCHEXT_L4D_INTERFACE, NULL));
	if (!g_pMatchExtL4D) {
		strncpy(error, "Unable to retrieve matchmaking extension interface", maxlength);

		return false;
	}

	sharesys->AddNatives(myself, MyNatives);
	sharesys->AddInterface(myself, this);

	return true;
}

KeyValues *CMatchExtL4D::GetAllMissions()
{
	return g_pMatchExtL4D->GetAllMissions();
}

KeyValues *CMatchExtL4D::GetAllModes()
{
	return g_pMatchExtL4D->GetAllModes();
}

KeyValues *CMatchExtL4D::GetMapInfo(KeyValues *pSettings, KeyValues **ppMissionInfo)
{
	return g_pMatchExtL4D->GetMapInfo(pSettings, ppMissionInfo);
}

KeyValues *CMatchExtL4D::GetMapInfoByBspName(KeyValues *pSettings, const char *szBspMapName, KeyValues **ppMissionInfo)
{
	return g_pMatchExtL4D->GetMapInfoByBspName(pSettings, szBspMapName, ppMissionInfo);
}

KeyValues *CMatchExtL4D::GetGameModeInfo(const char *szGameModeName)
{
	return g_pMatchExtL4D->GetGameModeInfo(szGameModeName);
}

KeyValues *CMatchExtL4D::GetActiveServerDetails(KeyValues *pRequest)
{
	return g_pMatchFramework->GetMatchNetworkMsgController()->GetActiveServerGameDetails(pRequest);
}
