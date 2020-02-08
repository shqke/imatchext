#include "natives.h"

static cell_t GetServerDetails(IPluginContext *pContext, const cell_t *params)
{
	KeyValues *pDest = g_pSM->ReadKeyValuesHandle(params[1]);
	if (!pDest) {
		pContext->ReportError("Invalid Handle");

		return 0;
	}

	KeyValues *pRequest = g_pSM->ReadKeyValuesHandle(params[2]);

	KeyValues *pResult = g_pMatchFramework->GetMatchNetworkMsgController()->GetActiveServerGameDetails(pRequest);
	if (pResult) {
		*pDest = *pResult;

		// Clean up
		pResult->deleteThis();
	}

	return !!pResult;
}

static cell_t GetAllMissions(IPluginContext *pContext, const cell_t *params)
{
	KeyValues *pDest = g_pSM->ReadKeyValuesHandle(params[1]);
	if (!pDest) {
		pContext->ReportError("Invalid Handle");

		return 0;
	}

	KeyValues *pResult = g_pMatchExtL4D->GetAllMissions();
	if (pResult) {
		*pDest = *pResult;
	}

	return !!pResult;
}

static cell_t GetMapInfo(IPluginContext *pContext, const cell_t *params)
{
	KeyValues *pRequest = g_pSM->ReadKeyValuesHandle(params[1]);
	if (!pRequest) {
		pContext->ReportError("Invalid Handle");

		return 0;
	}

	KeyValues *pKvMapOut = g_pSM->ReadKeyValuesHandle(params[2]);
	KeyValues *pKvMissionOut = g_pSM->ReadKeyValuesHandle(params[3]);

	KeyValues *pKvMission = NULL;
	KeyValues *pKvMap = g_pMatchExtL4D->GetMapInfo(pRequest, &pKvMission);
	if (pKvMap) {
		if (pKvMapOut) {
			*pKvMapOut = *pKvMap;
		}

		if (pKvMissionOut) {
			*pKvMissionOut = *pKvMission;
		}
	}

	return !!pKvMap;
}

static cell_t GetMapInfoByBspName(IPluginContext *pContext, const cell_t *params)
{
	KeyValues *pRequest = g_pSM->ReadKeyValuesHandle(params[1]);
	if (!pRequest) {
		pContext->ReportError("Invalid Handle");

		return 0;
	}

	char *bsp_name = NULL;
	pContext->LocalToString(params[2], &bsp_name);

	KeyValues *pKvMapOut = g_pSM->ReadKeyValuesHandle(params[3]);
	KeyValues *pKvMissionOut = g_pSM->ReadKeyValuesHandle(params[4]);

	KeyValues *pKvMission = NULL;
	KeyValues *pKvMap = g_pMatchExtL4D->GetMapInfoByBspName(pRequest, bsp_name, &pKvMission);
	if (pKvMap) {
		if (pKvMapOut) {
			*pKvMapOut = *pKvMap;
		}

		if (pKvMissionOut) {
			*pKvMissionOut = *pKvMission;
		}
	}

	return !!pKvMap;
}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
static cell_t GetAllModes(IPluginContext *pContext, const cell_t *params)
{
	KeyValues *pDest = g_pSM->ReadKeyValuesHandle(params[1]);
	if (!pDest) {
		pContext->ReportError("Invalid Handle");

		return 0;
	}

	KeyValues *pResult = g_pMatchExtL4D->GetAllModes();
	if (pResult) {
		*pDest = *pResult;
	}

	return !!pResult;
}

static cell_t GetGameModeInfo(IPluginContext *pContext, const cell_t *params)
{
	KeyValues *pDest = g_pSM->ReadKeyValuesHandle(params[1]);
	if (!pDest) {
		pContext->ReportError("Invalid Handle");

		return 0;
	}

	char *name;
	pContext->LocalToStringNULL(params[2], &name);

	KeyValues *pResult = g_pMatchExtL4D->GetGameModeInfo(name);
	if (pResult) {
		*pDest = *pResult;
	}

	return !!pResult;
}
#endif

const sp_nativeinfo_t MyNatives[] =
{
	{ "GetServerDetails", GetServerDetails },
	{ "GetAllMissions", GetAllMissions },
	{ "GetMapInfo", GetMapInfo },
	{ "GetMapInfoByBspName", GetMapInfoByBspName },
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	{ "GetAllModes", GetAllModes },
	{ "GetGameModeInfo", GetGameModeInfo },
#endif

	{ NULL, NULL },
};