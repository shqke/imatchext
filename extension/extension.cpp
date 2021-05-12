#include "extension.h"
#include "natives.h"

CMatchExtInterface g_MatchExtInterface;
SMEXT_LINK(&g_MatchExtInterface);

#include <sm_platform.h>
#include <amtl/os/am-shared-library.h>

#if !defined(_WIN32) && SOURCE_ENGINE == SE_LEFT4DEAD2
#define MATCHMAKING_LIB_DS_SUFFIX "_ds_srv"
#else
#define MATCHMAKING_LIB_DS_SUFFIX "_ds"
#endif

IMatchFramework* g_pMatchFramework = NULL;
IMatchExtL4D* g_pMatchExtL4D = NULL;

bool CMatchExtInterface::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
	char path[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, path, sizeof(path), "bin/matchmaking%s." PLATFORM_LIB_EXT, engine->IsDedicatedServer() ? MATCHMAKING_LIB_DS_SUFFIX : "");

	char libErr[256];
	ke::RefPtr<ke::SharedLib> lib = ke::SharedLib::Open(path, libErr, sizeof(libErr));
	if (!lib) {
		V_snprintf(error, maxlength, "Unable to load \"GAME/%s\" (error: \"%s\")", path, libErr);

		return false;
	}

	CreateInterfaceFn matchmakingFactory = lib->get<CreateInterfaceFn>(CREATEINTERFACE_PROCNAME);
	if (matchmakingFactory == NULL) {
		ke::SafeStrcpy(error, maxlength, "Unable to retrieve matchmaking factory \"" CREATEINTERFACE_PROCNAME "\"");

		return false;
	}

	g_pMatchFramework = static_cast<IMatchFramework*>(matchmakingFactory(IMATCHFRAMEWORK_VERSION_STRING, NULL));
	if (g_pMatchFramework == NULL) {
		ke::SafeStrcpy(error, maxlength, "Unable to retrieve matchmaking framework interface \"" IMATCHFRAMEWORK_VERSION_STRING "\"");

		return false;
	}

	g_pMatchExtL4D = static_cast<IMatchExtL4D*>(matchmakingFactory(IMATCHEXT_L4D_INTERFACE, NULL));
	if (g_pMatchExtL4D == NULL) {
		ke::SafeStrcpy(error, maxlength, "Unable to retrieve matchmaking extension interface \"" IMATCHEXT_L4D_INTERFACE "\"");

		return false;
	}

	sharesys->AddNatives(myself, MyNatives);
	sharesys->AddInterface(myself, this);

	return true;
}

IMatchExtL4D* CMatchExtInterface::GetIMatchExtL4D()
{
	return g_pMatchExtL4D;
}

IMatchFramework* CMatchExtInterface::GetIMatchFrameWork()
{
	return g_pMatchFramework;
}
