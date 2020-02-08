#include "extension.h"
#include "natives.h"

CMatchExtInterface g_MatchExtInterface;
SMEXT_LINK(&g_MatchExtInterface);

IMatchFramework *g_pMatchFramework = NULL;
IMatchExtL4D *g_pMatchExtL4D = NULL;

bool CMatchExtInterface::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if (Q_strcmp(GAME_FOLDER, smutils->GetGameFolderName())) {
		Q_strncpy(error, "Only supports " GAME_FOLDER, maxlength);

		return false;
	}

	char path[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, path, sizeof(path), "bin/" MATCHMAKING_DS_LIB);

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

IMatchExtL4D *CMatchExtInterface::GetIMatchExtL4D()
{
	return g_pMatchExtL4D;
}

IMatchFramework *CMatchExtInterface::GetIMatchFrameWork()
{
	return g_pMatchFramework;
}
