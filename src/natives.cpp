#include "natives.h"
#include "wrappers.h"

#include <memory>

// Helpers
class KeyValuesPtr :
	public std::unique_ptr<KeyValues, void(*)(KeyValues*)>
{
public:
	KeyValuesPtr(KeyValues* pkv) :
		std::unique_ptr<KeyValues, void(*)(KeyValues*)>(pkv, &DeleterFn)
	{
	}
	
	KeyValuesPtr(const char* setName, const char* firstKey, const char* firstValue) :
		std::unique_ptr<KeyValues, void(*)(KeyValues*)>(new KeyValues(setName, firstKey, firstValue), &DeleterFn)
	{
	}

	KeyValuesPtr() = delete;

private:
	static void DeleterFn(KeyValues* pkv)
	{
		pkv->deleteThis();
	}
};

KeyValuesPtr GetServerGameDetails(KeyValues* pkvRequest = NULL)
{
	KeyValues* pkvDetails = NULL;

	IMatchNetworkMsgController* pMsgController = g_pMatchFramework->GetMatchNetworkMsgController();
	if (pMsgController != NULL) {
		pkvDetails = pMsgController->GetActiveServerGameDetails(pkvRequest);
	}

	return KeyValuesPtr(pkvDetails);
}

HKeySymbol GetCurrentMission()
{
	auto pkvServerGameDetails = GetServerGameDetails();
	if (pkvServerGameDetails != NULL) {
		const char* pszMission = pkvServerGameDetails->GetString("game/campaign");
		if (*pszMission != '\0') {
			return KeyValuesSystem()->GetSymbolForString(pszMission, false);
		}
	}

	return INVALID_KEY_SYMBOL;
}

HKeySymbol GetCurrentMode()
{
	const char* pszMode = mp_gamemode->GetString();
	if (*pszMode == '\0') {
		pszMode = "coop";
	}

	return KeyValuesSystem()->GetSymbolForString(pszMode, false);
}

inline int Param_GetKeyValues(IPluginContext* pContext, cell_t param, KeyValues** ppkvData, bool bOptional)
{
	KeyValues* pkvData = NULL;
	if (param != 0) {
		pkvData = smutils->ReadKeyValuesHandle(param);
	}

	if (pkvData == NULL && (param != 0 || !bOptional)) {
		if (pContext != NULL) {
			pContext->ReportError("Invalid Handle \"%u\"", param);
		}

		return SP_ERROR_PARAM;
	}

	*ppkvData = pkvData;
	return SP_ERROR_NONE;
}

inline int Param_GetStringPtr(IPluginContext* pContext, cell_t param, const char** ppszValue, bool bOptional)
{
	char* pszValue = NULL;
	int error = SP_ERROR_NONE;
	if (bOptional) {
		error = pContext->LocalToStringNULL(param, &pszValue);
	}
	else {
		error = pContext->LocalToString(param, &pszValue);
	}

	if (error != SP_ERROR_NONE) {
		pContext->ReportErrorNumber(error);
		return error;
	}

	*ppszValue = pszValue;
	return SP_ERROR_NONE;
}

inline int Param_SetStringPtr(IPluginContext* pContext, cell_t destAddr, cell_t maxlength, const char* pszValue, size_t* pBytes)
{
	int error = pContext->StringToLocalUTF8(destAddr, maxlength, pszValue, pBytes);
	if (error != SP_ERROR_NONE) {
		pContext->ReportErrorNumber(error);
		return error;
	}

	return SP_ERROR_NONE;
}

inline int Param_GetCellPtr(IPluginContext* pContext, cell_t param, cell_t** ppszAddress)
{
	cell_t* pszAddress = NULL;
	int error = pContext->LocalToPhysAddr(param, &pszAddress);
	if (error != SP_ERROR_NONE) {
		pContext->ReportErrorNumber(error);
		return error;
	}

	*ppszAddress = pszAddress;
	return SP_ERROR_NONE;
}

inline int Param_GetKeyValuesMode(IPluginContext* pContext, cell_t keySymbol, KeyValues** ppkvMode)
{
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues* pkvModes = g_pMatchExtL4D->GetAllModes();
	if (pkvModes == NULL) {
		if (pContext != NULL) {
			pContext->ReportError("Missing \"GameModes\" key");
		}

		return SP_ERROR_NOT_FOUND;
	}

	KeyValues* pkvMode = pkvModes->FindKey(keySymbol);
	if (pkvMode == NULL) {
		if (pContext != NULL) {
			pContext->ReportError("Invalid ModeSymbol \"%u\"", keySymbol);
		}

		return SP_ERROR_PARAM;
	}

	if (ppkvMode != NULL) {
		*ppkvMode = pkvMode;
	}
#endif
	return SP_ERROR_NONE;
}

inline int Param_GetModeStringPtr(IPluginContext* pContext, cell_t addr, const char** ppszName)
{
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	const char* pszName = NULL;
	int error = Param_GetStringPtr(pContext, addr, &pszName, true);
	if (error != SP_ERROR_NONE) {
		return error;
	}

	if (pszName == NULL || *pszName == '\0') {
		pszName = mp_gamemode->GetString();
		if (*pszName == '\0') {
			pszName = "coop";
		}
	}

	// Validate input - disallow subkeys
	KeyValues* pkvMode = NULL;
	error = Param_GetKeyValuesMode(pContext, KeyValuesSystem()->GetSymbolForString(pszName, false), &pkvMode);
	if (error != SP_ERROR_NONE) {
		return error;
	}

	*ppszName = pszName;
#endif
	return SP_ERROR_NONE;
}

inline int Param_GetKeyValuesMission(IPluginContext* pContext, cell_t keySymbol, KeyValues** ppkvMission)
{
	KeyValues* pkvMissions = g_pMatchExtL4D->GetAllMissions();
	if (pkvMissions == NULL) {
		if (pContext != NULL) {
			pContext->ReportError("Missing \"Missions\" key");
		}

		return SP_ERROR_NOT_FOUND;
	}

	KeyValues* pkvMission = pkvMissions->FindKey(keySymbol);
	if (pkvMission == NULL) {
		if (pContext != NULL) {
			pContext->ReportError("Invalid MissionSymbol \"%u\"", keySymbol);
		}

		return SP_ERROR_PARAM;
	}

	if (ppkvMission != NULL) {
		*ppkvMission = pkvMission;
	}

	return SP_ERROR_NONE;
}

bool IsModeOfType(const char* pszBaseMode, IPluginContext* pContext, cell_t param)
{
	const char* pszName = NULL;
	if (Param_GetModeStringPtr(pContext, param, &pszName) != SP_ERROR_NONE) {
		return 0;
	}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues* pkvMode = g_pMatchExtL4D->GetGameModeInfo(pszName);
	if (pkvMode != NULL) {
		return V_stricmp(pkvMode->GetString("base"), pszBaseMode) == 0;
	}

	return 0;
#else
	return V_stricmp(pszName, pszBaseMode) == 0;
#endif
}

// Natives
static cell_t GetServerGameDetails(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[1], &pkvDest, false) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvRequest = NULL;
	if (Param_GetKeyValues(pContext, params[2], &pkvRequest, true) != SP_ERROR_NONE) {
		return 0;
	}

	auto pkvDetails = GetServerGameDetails(pkvRequest);
	if (pkvDetails != NULL) {
		*pkvDest = *pkvDetails;
		return 1;
	}

	return 0;
}

static cell_t GetReservationCookie(IPluginContext* pContext, const cell_t* params)
{
	cell_t* pxuidReserve = NULL;
	if (Param_GetCellPtr(pContext, params[1], &pxuidReserve) != SP_ERROR_NONE) {
		return 0;
	}

	extern CBaseServer* g_pGameServer;
	if (g_pGameServer != NULL) {
		*(uint64_t*)pxuidReserve = g_pGameServer->GetReservationCookie();
		return 1;
	}

	return 0;
}

static cell_t GetAllMissions(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[1], &pkvDest, false) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMissions = g_pMatchExtL4D->GetAllMissions();
	if (pkvMissions != NULL) {
		*pkvDest = *pkvMissions;
		return 1;
	}

	return 0;
}

static cell_t GetMapInfo(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvRequest = NULL;
	if (Param_GetKeyValues(pContext, params[1], &pkvRequest, false) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMapDest = NULL;
	if (Param_GetKeyValues(pContext, params[2], &pkvMapDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMissionDest = NULL;
	if (Param_GetKeyValues(pContext, params[3], &pkvMissionDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMission = NULL;
	KeyValues* pkvMap = g_pMatchExtL4D->GetMapInfo(pkvRequest, &pkvMission);
	if (pkvMap != NULL) {
		if (pkvMapDest != NULL) {
			*pkvMapDest = *pkvMap;
		}

		if (pkvMissionDest != NULL && pkvMission != NULL) {
			*pkvMissionDest = *pkvMission;
		}

		return 1;
	}

	return 0;
}

static cell_t GetMapInfoByBspName(IPluginContext* pContext, const cell_t* params)
{
	const char* pszBSPName = NULL;
	if (Param_GetStringPtr(pContext, params[1], &pszBSPName, false) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszMode = NULL;
	if (Param_GetModeStringPtr(pContext, params[2], &pszMode) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMapDest = NULL;
	if (Param_GetKeyValues(pContext, params[3], &pkvMapDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMissionDest = NULL;
	if (Param_GetKeyValues(pContext, params[4], &pkvMissionDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	auto pkvRequest = GetServerGameDetails();
	if (pkvRequest != NULL) {
		pkvRequest->SetString("game/mode", pszMode);

		KeyValues* pkvMission = NULL;
		KeyValues* pkvMap = g_pMatchExtL4D->GetMapInfoByBspName(pkvRequest.get(), pszBSPName, &pkvMission);
		if (pkvMap != NULL) {
			if (pkvMapDest != NULL) {
				*pkvMapDest = *pkvMap;
			}

			if (pkvMissionDest != NULL && pkvMission != NULL) {
				*pkvMissionDest = *pkvMission;
			}

			return 1;
		}
	}

	return 0;
}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
static cell_t GetAllModes(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[1], &pkvDest, false) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvModes = g_pMatchExtL4D->GetAllModes();
	if (pkvModes != NULL) {
		*pkvDest = *pkvModes;
		return 1;
	}

	return 0;
}

static cell_t GetGameModeInfo(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[1], &pkvDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszName = NULL;
	if (Param_GetModeStringPtr(pContext, params[2], &pszName) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMode = g_pMatchExtL4D->GetGameModeInfo(pszName);
	if (pkvMode == NULL) {
		return 0;
	}

	if (pkvDest != NULL) {
		*pkvDest = *pkvMode;
	}

	return 1;
}
#endif

static cell_t IsSingleChapterMode(IPluginContext* pContext, const cell_t* params)
{
	const char* pszName = NULL;
	if (Param_GetModeStringPtr(pContext, params[1], &pszName) != SP_ERROR_NONE) {
		return 0;
	}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues* pkvMode = g_pMatchExtL4D->GetGameModeInfo(pszName);
	if (pkvMode != NULL) {
		KeyValues* pkvBaseMode = g_pMatchExtL4D->GetGameModeInfo(pkvMode->GetString("base", pszName));
		if (pkvBaseMode != NULL) {
			return pkvMode->GetInt("singlechapter", pkvBaseMode->GetInt("singlechapter")) != 0;
		}
	}

	return 0;
#else
	return V_stricmp(pszName, "survival") == 0;
#endif
}

static cell_t IsCoopMode(IPluginContext* pContext, const cell_t* params)
{
	return IsModeOfType("coop", pContext, params[1]);
}

static cell_t IsRealismMode(IPluginContext* pContext, const cell_t* params)
{
	return IsModeOfType("realism", pContext, params[1]);
}

static cell_t IsVersusMode(IPluginContext* pContext, const cell_t* params)
{
	return IsModeOfType("versus", pContext, params[1]);
}

static cell_t IsSurvivalMode(IPluginContext* pContext, const cell_t* params)
{
	return IsModeOfType("survival", pContext, params[1]);
}

static cell_t IsScavengeMode(IPluginContext* pContext, const cell_t* params)
{
	return IsModeOfType("scavenge", pContext, params[1]);
}

static cell_t HasPlayerControlledZombies(IPluginContext* pContext, const cell_t* params)
{
	const char* pszName = NULL;
	if (Param_GetModeStringPtr(pContext, params[1], &pszName) != SP_ERROR_NONE) {
		return 0;
	}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues* pkvMode = g_pMatchExtL4D->GetGameModeInfo(pszName);
	if (pkvMode != NULL) {
		return pkvMode->GetInt("playercontrolledzombies") > 0;
	}

	return 0;
#else
	return V_stricmp(pszName, "versus") == 0 || V_stricmp(pszName, "teamversus") == 0;
#endif
}

static cell_t HasConfigurableDifficultySetting(IPluginContext* pContext, const cell_t* params)
{
	const char* pszName = NULL;
	if (Param_GetModeStringPtr(pContext, params[1], &pszName) != SP_ERROR_NONE) {
		return 0;
	}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues* pkvMode = g_pMatchExtL4D->GetGameModeInfo(pszName);
	if (pkvMode != NULL) {
		KeyValues* pkvBaseMode = g_pMatchExtL4D->GetGameModeInfo(pkvMode->GetString("base", pszName));
		if (pkvBaseMode != NULL) {
			return pkvMode->GetInt("hasdifficulty", pkvBaseMode->GetInt("hasdifficulty")) != 0;
		}
	}

	return 0;
#else
	return V_stricmp(pszName, "coop") == 0;
#endif
}

static cell_t IsFirstMapInScenario(IPluginContext* pContext, const cell_t* params)
{
	auto pkvDetails = GetServerGameDetails();
	if (pkvDetails == NULL) {
		return 0;
	}

	KeyValues* pkvMap = g_pMatchExtL4D->GetMapInfo(pkvDetails.get());
	if (pkvMap == NULL) {
		return 0;
	}

	pkvDetails->SetInt("game/chapter", 1);
	if (g_pMatchExtL4D->GetMapInfo(pkvDetails.get()) == pkvMap) {
		return 1;
	}

	return 0;
}

static cell_t IsMissionFinalMap(IPluginContext* pContext, const cell_t* params)
{
	auto pkvDetails = GetServerGameDetails();
	if (pkvDetails == NULL) {
		return 0;
	}

	KeyValues* pkvMap = g_pMatchExtL4D->GetMapInfo(pkvDetails.get());
	if (pkvMap == NULL) {
		return 0;
	}

	pkvDetails->SetInt("game/chapter", pkvDetails->GetInt("game/chapter") + 1);
	if (g_pMatchExtL4D->GetMapInfo(pkvDetails.get()) == NULL) {
		return 1;
	}

	return 0;
}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
static cell_t ModeSymbol_IsValid(IPluginContext* pContext, const cell_t* params)
{
	return Param_GetKeyValuesMode(NULL, params[1], NULL) == SP_ERROR_NONE;
}

static cell_t ModeSymbol_GetName(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	size_t written = 0;
	if (Param_SetStringPtr(pContext, params[2], params[3], pkvMode->GetName(), &written) != SP_ERROR_NONE) {
		return 0;
	}

	return written;
}

static cell_t ModeSymbol_GetString(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszKey = NULL;
	if (Param_GetStringPtr(pContext, params[2], &pszKey, true) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszDefaultValue = NULL;
	if (Param_GetStringPtr(pContext, params[5], &pszDefaultValue, false) != SP_ERROR_NONE) {
		return 0;
	}

	size_t written = 0;
	if (Param_SetStringPtr(pContext, params[3], params[4], pkvMode->GetString(pszKey, pszDefaultValue), &written) != SP_ERROR_NONE) {
		return 0;
	}

	return written;
}

static cell_t ModeSymbol_Export(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[2], &pkvDest, false) != SP_ERROR_NONE) {
		return 0;
	}

	*pkvDest = *pkvMode;
	return 0;
}

static cell_t ModeSymbol_IsDisabled_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMode->GetInt("disabled") != 0;
}

static cell_t ModeSymbol_Base_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvBaseMode = g_pMatchExtL4D->GetGameModeInfo(pkvMode->GetString("base"));
	if (pkvBaseMode != NULL) {
		return pkvBaseMode->GetNameSymbol();
	}
	
	return 0;
}

static cell_t ModeSymbol_IsAddon_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMode->GetInt("addon") != 0;
}

static cell_t ModeSymbol_MaxPlayers_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMode->GetInt("maxplayers");
}

static cell_t ModeSymbol_HasPlayerControlledZombies_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMode->GetInt("playercontrolledzombies") > 0;
}

static cell_t ModeSymbol_HasConfigurableDifficultySetting_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvBaseMode = g_pMatchExtL4D->GetGameModeInfo(pkvMode->GetString("base", pkvMode->GetName()));
	if (pkvBaseMode != NULL) {
		return pkvMode->GetInt("hasdifficulty", pkvBaseMode->GetInt("hasdifficulty")) != 0;
	}

	return 0;
}

static cell_t ModeSymbol_IsSingleChapterMode_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvBaseMode = g_pMatchExtL4D->GetGameModeInfo(pkvMode->GetString("base", pkvMode->GetName()));
	if (pkvBaseMode != NULL) {
		return pkvMode->GetInt("singlechapter", pkvBaseMode->GetInt("singlechapter")) != 0;
	}

	return 0;
}

static cell_t ModeSymbol_GetNumChapters(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[2], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMissionModes = pkvMission->FindKey("modes", true);
	if (pkvMissionModes == NULL) {
		return 0;
	}

	KeyValues* pkvMissionMode = pkvMissionModes->FindKey(params[1]);
	if (pkvMissionMode == NULL) {
		return 0;
	}

	return pkvMissionMode->GetInt("chapters");
}

static cell_t ModeSymbol_GetChapter(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[4], &pkvDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[2], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvMissionModes = pkvMission->FindKey("modes", true);
	if (pkvMissionModes == NULL) {
		return 0;
	}

	KeyValues* pkvMissionMode = pkvMissionModes->FindKey(params[1]);
	if (pkvMissionMode == NULL) {
		return 0;
	}

	char szChapter[16];
	V_snprintf(szChapter, sizeof(szChapter), "%d", params[3]);
	KeyValues* pkvChapter = pkvMissionMode->FindKey(szChapter);
	if (pkvChapter == NULL) {
		return 0;
	}

	if (pkvDest != NULL) {
		*pkvDest = *pkvChapter;
	}

	return 1;
}

static cell_t ModeSymbol_GetChapterByFile(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszBSPName = NULL;
	if (Param_GetStringPtr(pContext, params[2], &pszBSPName, false) != SP_ERROR_NONE) {
		return 0;
	}

	cell_t* pnMissionId = NULL;
	if (Param_GetCellPtr(pContext, params[3], &pnMissionId) != SP_ERROR_NONE) {
		return 0;
	}

	cell_t* piChapter = NULL;
	if (Param_GetCellPtr(pContext, params[4], &piChapter) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[5], &pkvDest, true) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValuesPtr pkvRequest("Request", "game/mode", pkvMode->GetName());

	KeyValues* pkvMission = NULL;
	KeyValues* pkvMap = g_pMatchExtL4D->GetMapInfoByBspName(pkvRequest.get(), pszBSPName, &pkvMission);
	if (pkvMap != NULL) {
		if (pkvDest != NULL) {
			*pkvDest = *pkvMap;
		}

		if (pkvMission != NULL && pnMissionId != NULL) {
			*pnMissionId = pkvMission->GetNameSymbol();
		}

		if (piChapter != NULL) {
			*piChapter = pkvMap->GetInt("chapter");
		}

		return 1;
	}

	return 0;
}

static cell_t ModeSymbol_First(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvModes = g_pMatchExtL4D->GetAllModes();
	if (pkvModes == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	KeyValues* pkvMode = pkvModes->GetFirstTrueSubKey();
	if (pkvMode == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	return pkvMode->GetNameSymbol();
}

static cell_t ModeSymbol_Next(IPluginContext* pContext, const cell_t* params)
{
	if (params[1] == INVALID_KEY_SYMBOL) {
		return ModeSymbol_First(pContext, params);
	}

	KeyValues* pkvMode = NULL;
	if (Param_GetKeyValuesMode(pContext, params[1], &pkvMode) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvNext = pkvMode->GetNextTrueSubKey();
	if (pkvNext == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	return pkvNext->GetNameSymbol();
}

static cell_t GetModeSymbol(IPluginContext* pContext, const cell_t* params)
{
	const char* pszName = NULL;
	if (Param_GetStringPtr(pContext, params[1], &pszName, true) != SP_ERROR_NONE) {
		return 0;
	}

	if (pszName == NULL || *pszName == '\0') {
		pszName = mp_gamemode->GetString();
		if (*pszName == '\0') {
			pszName = "coop";
		}
	}

	KeyValues* pkvMode = g_pMatchExtL4D->GetGameModeInfo(pszName);
	if (pkvMode == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	return pkvMode->GetNameSymbol();
}
#endif

static cell_t MissionSymbol_IsValid(IPluginContext* pContext, const cell_t* params)
{
	return Param_GetKeyValuesMission(NULL, params[1], NULL) == SP_ERROR_NONE;
}

static cell_t MissionSymbol_GetName(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	size_t written = 0;
	if (Param_SetStringPtr(pContext, params[2], params[3], pkvMission->GetName(), &written) != SP_ERROR_NONE) {
		return 0;
	}

	return written;
}

static cell_t MissionSymbol_GetString(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszKey = NULL;
	if (Param_GetStringPtr(pContext, params[2], &pszKey, true) != SP_ERROR_NONE) {
		return 0;
	}

	const char* pszDefaultValue = NULL;
	if (Param_GetStringPtr(pContext, params[5], &pszDefaultValue, false) != SP_ERROR_NONE) {
		return 0;
	}

	size_t written = 0;
	if (Param_SetStringPtr(pContext, params[3], params[4], pkvMission->GetString(pszKey, pszDefaultValue), &written) != SP_ERROR_NONE) {
		return 0;
	}

	return written;
}

static cell_t MissionSymbol_Export(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvDest = NULL;
	if (Param_GetKeyValues(pContext, params[2], &pkvDest, false) != SP_ERROR_NONE) {
		return 0;
	}

	*pkvDest = *pkvMission;
	return 0;
}

static cell_t MissionSymbol_IsDisabled_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMission->GetInt("disabled") != 0;
}

static cell_t MissionSymbol_IsAddon_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMission->GetInt("addon") != 0;
}

static cell_t MissionSymbol_AllowCola_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMission->GetInt("allow_cola") != 0;
}

static cell_t MissionSymbol_NoFemaleBoomers_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMission->GetInt("no_female_boomers") != 0;
}

static cell_t MissionSymbol_NoCSWeapons_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMission->GetInt("no_cs_weapons") != 0;
}

static cell_t MissionSymbol_SurvivorSet_get(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	return pkvMission->GetInt("survivor_set") == 1 ? 1 : 2;
}

static cell_t MissionSymbol_First(IPluginContext* pContext, const cell_t* params)
{
	KeyValues* pkvMissions = g_pMatchExtL4D->GetAllMissions();
	if (pkvMissions == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	KeyValues* pkvMission = pkvMissions->GetFirstTrueSubKey();
	if (pkvMission == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	return pkvMission->GetNameSymbol();
}

static cell_t MissionSymbol_Next(IPluginContext* pContext, const cell_t* params)
{
	if (params[1] == INVALID_KEY_SYMBOL) {
		return MissionSymbol_First(pContext, params);
	}

	KeyValues* pkvMission = NULL;
	if (Param_GetKeyValuesMission(pContext, params[1], &pkvMission) != SP_ERROR_NONE) {
		return 0;
	}

	KeyValues* pkvNext = pkvMission->GetNextTrueSubKey();
	if (pkvNext == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	return pkvNext->GetNameSymbol();
}

static cell_t GetMissionSymbol(IPluginContext* pContext, const cell_t* params)
{
	const char* pszName = NULL;
	if (Param_GetStringPtr(pContext, params[1], &pszName, false) != SP_ERROR_NONE) {
		return 0;
	}

	if (*pszName == '\0') {
		return INVALID_KEY_SYMBOL;
	}

	KeyValues* pkvMissions = g_pMatchExtL4D->GetAllMissions();
	if (pkvMissions == NULL) {
		return INVALID_KEY_SYMBOL;
	}

	KeyValues* pkvMission = pkvMissions->FindKey(KeyValuesSystem()->GetSymbolForString(pszName, false));
	if (pkvMission != NULL) {
		return pkvMission->GetNameSymbol();
	}

	return INVALID_KEY_SYMBOL;
}

const sp_nativeinfo_t g_Natives[] =
{
	{ "GetServerGameDetails", GetServerGameDetails },
	{ "GetReservationCookie", GetReservationCookie },
	{ "GetAllMissions", GetAllMissions },
	{ "GetMapInfo", GetMapInfo },
	{ "GetMapInfoByBspName", GetMapInfoByBspName },
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	{ "GetAllModes", GetAllModes },
	{ "GetGameModeInfo", GetGameModeInfo },
#endif

	{ "IsSingleChapterMode", IsSingleChapterMode },
	{ "IsCoopMode", IsCoopMode },
	{ "IsRealismMode", IsRealismMode },
	{ "IsVersusMode", IsVersusMode },
	{ "IsSurvivalMode", IsSurvivalMode },
	{ "IsScavengeMode", IsScavengeMode },
	{ "HasPlayerControlledZombies", HasPlayerControlledZombies },
	{ "HasConfigurableDifficultySetting", HasConfigurableDifficultySetting },
	{ "IsFirstMapInScenario", IsFirstMapInScenario },
	{ "IsMissionFinalMap", IsMissionFinalMap },

	// Helpers
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	{ "ModeSymbol.GetName", ModeSymbol_GetName },
	{ "ModeSymbol.GetString", ModeSymbol_GetString },
	{ "ModeSymbol.Export", ModeSymbol_Export },
	{ "ModeSymbol.IsDisabled.get", ModeSymbol_IsDisabled_get },
	{ "ModeSymbol.Base.get", ModeSymbol_Base_get },
	{ "ModeSymbol.IsAddon.get", ModeSymbol_IsAddon_get },
	{ "ModeSymbol.MaxPlayers.get", ModeSymbol_MaxPlayers_get },
	{ "ModeSymbol.HasPlayerControlledZombies.get", ModeSymbol_HasPlayerControlledZombies_get },
	{ "ModeSymbol.HasConfigurableDifficultySetting.get", ModeSymbol_HasConfigurableDifficultySetting_get },
	{ "ModeSymbol.IsSingleChapterMode.get", ModeSymbol_IsSingleChapterMode_get },
	{ "ModeSymbol.GetNumChapters", ModeSymbol_GetNumChapters },
	{ "ModeSymbol.ExportChapter", ModeSymbol_GetChapter },
	{ "ModeSymbol.ExportChapterByFile", ModeSymbol_GetChapterByFile },
	{ "ModeSymbol.First", ModeSymbol_First },
	{ "ModeSymbol.Next", ModeSymbol_Next },
	{ "ModeSymbol.IsValid", ModeSymbol_IsValid },
	{ "GetModeSymbol", GetModeSymbol },
#endif

	{ "MissionSymbol.GetName", MissionSymbol_GetName },
	{ "MissionSymbol.GetString", MissionSymbol_GetString },
	{ "MissionSymbol.Export", MissionSymbol_Export },
	{ "MissionSymbol.IsDisabled.get", MissionSymbol_IsDisabled_get },
	{ "MissionSymbol.IsAddon.get", MissionSymbol_IsAddon_get },
	{ "MissionSymbol.AllowCola.get", MissionSymbol_AllowCola_get },
	{ "MissionSymbol.NoFemaleBoomers.get", MissionSymbol_NoFemaleBoomers_get },
	{ "MissionSymbol.NoCSWeapons.get", MissionSymbol_NoCSWeapons_get },
	{ "MissionSymbol.SurvivorSet.get", MissionSymbol_SurvivorSet_get },
	{ "MissionSymbol.First", MissionSymbol_First },
	{ "MissionSymbol.Next", MissionSymbol_Next },
	{ "MissionSymbol.IsValid", MissionSymbol_IsValid },
	{ "GetMissionSymbol", GetMissionSymbol },

	{ NULL, NULL },
};