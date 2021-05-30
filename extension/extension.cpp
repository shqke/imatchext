#include "extension.h"
#include "natives.h"

#include <byteswap.h>
#include <sm_platform.h>
#include <amtl/os/am-shared-library.h>
#include <CDetour/detours.h>

#include <memory>
#include <cstring>

#if !defined(_WIN32) && SOURCE_ENGINE == SE_LEFT4DEAD2
#define MATCHMAKING_LIB_DS_SUFFIX "_ds_srv"
#else
#define MATCHMAKING_LIB_DS_SUFFIX "_ds"
#endif

CExtension g_Extension;
SMEXT_LINK(&g_Extension);

ILanguageFileParser* g_pLanguageFileParser = NULL;
IFileSystem* g_pFullFileSystem = NULL;
IMatchFramework* g_pMatchFramework = NULL;
IMatchExtL4D* g_pMatchExtL4D = NULL;

IForward* g_fwdOnMissionCacheReload = NULL;

void* pfn_CMatchExtL4D_Initialize = NULL;
CDetour* detour_CMatchExtL4D_Initialize = NULL;

HKeySymbol g_iCurrentModeSymbol = INVALID_KEY_SYMBOL;
HKeySymbol g_iCurrentMissionSymbol = INVALID_KEY_SYMBOL;

ConVar* mp_gamemode = NULL;

void Handler_sm_imatchext_disable_modes(IConVar* pVar, const char* pOldValue, float flOldValue)
{
	const char* pszNewValue = static_cast<ConVar*>(pVar)->GetString();
	if (V_stricmp(pszNewValue, pOldValue) == 0) {
		return;
	}

	g_Extension.UpdateDisabledModes(pszNewValue);
}

void Handler_sm_imatchext_disable_missions(IConVar* pVar, const char* pOldValue, float flOldValue)
{
	const char* pszNewValue = static_cast<ConVar*>(pVar)->GetString();
	if (V_stricmp(pszNewValue, pOldValue) == 0) {
		return;
	}

	g_Extension.UpdateDisabledMissions(pszNewValue);
}

ConVar sm_imatchext_disable_modes("sm_imatchext_disable_modes", "", 0, "Comma delimited game modes to mark as disabled", Handler_sm_imatchext_disable_modes);
ConVar sm_imatchext_disable_missions("sm_imatchext_disable_missions", "", 0, "Comma delimited mission names to mark as disabled", Handler_sm_imatchext_disable_missions);

CON_COMMAND(sm_imatchext_show_disabled, "")
{
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	Msg("Disabled modes:\n");

	KeyValues* pkvAllModes = g_pMatchExtL4D->GetAllModes();
	for (KeyValues* pkvMode = pkvAllModes->GetFirstTrueSubKey(); pkvMode != NULL; pkvMode = pkvMode->GetNextTrueSubKey()) {
		if (pkvMode->GetInt("disabled") != 0) {
			Msg("\"%s\"\n", pkvMode->GetName());
		}
	}

	Msg("\n");
#endif

	Msg("Disabled missions:\n");

	KeyValues* pkvAllMissions = g_pMatchExtL4D->GetAllMissions();
	for (KeyValues* pkvMission = pkvAllMissions->GetFirstTrueSubKey(); pkvMission != NULL; pkvMission = pkvMission->GetNextTrueSubKey()) {
		if (pkvMission->GetInt("disabled") != 0) {
			Msg("\"%s\"\n", pkvMission->GetName());
		}
	}

	Msg("\n");
}

CON_COMMAND(sm_imatchext_gen_phrases, "Generate SourceMod translation phrases used in imatchext extension from game files")
{
	g_Extension.Generate();
}

DETOUR_DECL_MEMBER0(Handler_CMatchExtL4D_Initialize, void)
{
	DETOUR_MEMBER_CALL(Handler_CMatchExtL4D_Initialize)();

	g_Extension.NotifyMissionReload(true);
}

size_t UTIL_DecodeHexString(unsigned char* buffer, size_t maxlength, const char* hexstr)
{
	size_t written = 0;
	size_t length = strlen(hexstr);

	for (size_t i = 0; i < length; i++)
	{
		if (written >= maxlength)
			break;
		buffer[written++] = hexstr[i];
		if (hexstr[i] == '\\' && hexstr[i + 1] == 'x')
		{
			if (i + 3 >= length)
				continue;
			/* Get the hex part. */
			char s_byte[3];
			int r_byte;
			s_byte[0] = hexstr[i + 2];
			s_byte[1] = hexstr[i + 3];
			s_byte[2] = '\0';
			/* Read it as an integer */
			sscanf(s_byte, "%x", &r_byte);
			/* Save the value */
			buffer[written - 1] = r_byte;
			/* Adjust index */
			i += 3;
		}
	}

	return written;
}

void CExtension::OnLevelInit(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background)
{
	NotifyMissionReload(false);
}

bool CExtension::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
	mp_gamemode = g_pCVar->FindVar("mp_gamemode");
	if (mp_gamemode == NULL) {
		V_strncpy(error, "Cvar \"mp_gamemode\" not found", maxlength);
		return false;
	}

	if (!SetupFromMatchmakingLibrary(error, maxlength)) {
		return false;
	}

	IGameConfig* gc = NULL;
	if (!gameconfs->LoadGameConfigFile(GAMEDATA_FILE, &gc, error, maxlength)) {
		V_strncpy(error, "Unable to load a gamedata file \"" GAMEDATA_FILE ".txt\"", maxlength);
		return false;
	}

	if (!SetupFromGameConfig(gc, error, maxlength)) {
		gameconfs->CloseGameConfigFile(gc);
		return false;
	}

	gameconfs->CloseGameConfigFile(gc);

	CDetourManager::Init(smutils->GetScriptingEngine(), NULL);

	detour_CMatchExtL4D_Initialize = DETOUR_CREATE_MEMBER(Handler_CMatchExtL4D_Initialize, pfn_CMatchExtL4D_Initialize);
	if (detour_CMatchExtL4D_Initialize == NULL) {
		V_strncpy(error, "Unable to create a detour for \"CMatchExtL4D::Initialize\"", maxlength);
		return false;
	}

	detour_CMatchExtL4D_Initialize->EnableDetour();

	g_fwdOnMissionCacheReload = forwards->CreateForward("OnMissionCacheReload", ET_Ignore, 0, NULL);
	if (g_fwdOnMissionCacheReload == NULL) {
		V_strncpy(error, "Unable to create a forward for \"OnMissionCacheReload\"", maxlength);
		return false;
	}

	IPluginIterator* pIterator = plsys->GetPluginIterator();
	if (pIterator != NULL) {
		do {
			IPlugin* pPlugin = pIterator->GetPlugin();
			if (pPlugin != NULL) {
				CExtension::OnPluginLoaded(pPlugin);
			}

			pIterator->NextPlugin();
		} while (pIterator->MorePlugins());

		pIterator->Release();
	}

	NotifyMissionReload(false);

	plsys->AddPluginsListener(this);
	sharesys->AddNatives(myself, g_Natives);
	sharesys->AddInterface(myself, this);
	sharesys->RegisterLibrary(myself, "imatchext");

	sharesys->AddDependency(myself, "langparser.ext", false, true);
	ConVar_Register(0, this);

	return true;
}

void CExtension::SDK_OnUnload()
{
	CPhrasesGenerator::SDK_OnUnload();

	if (detour_CMatchExtL4D_Initialize != NULL) {
		detour_CMatchExtL4D_Initialize->Destroy();
		detour_CMatchExtL4D_Initialize = NULL;
	}

	if (g_fwdOnMissionCacheReload != NULL) {
		forwards->ReleaseForward(g_fwdOnMissionCacheReload);
		g_fwdOnMissionCacheReload = NULL;
	}

	ConVar_Unregister();

	plsys->RemovePluginsListener(this);

	UpdateDisabledModes("");
	UpdateDisabledMissions("");
}

void CExtension::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(ILANGPARSER, g_pLanguageFileParser);
}

bool CExtension::QueryInterfaceDrop(SMInterface* pInterface)
{
	return true;
}

void CExtension::NotifyInterfaceDrop(SMInterface* pInterface)
{
	if (g_pLanguageFileParser == pInterface) {
		CPhrasesGenerator::SDK_OnUnload();
		g_pLanguageFileParser = NULL;
	}
}

bool CExtension::SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	ismm->AddListener(g_PLAPI, this);

	return true;
}

bool CExtension::SetupFromGameConfig(IGameConfig* gc, char* error, int maxlength)
{
	const char* pszKey = "CMatchExtL4D::Initialize";
	if (engine->IsDedicatedServer()) {
		pszKey = "CMatchExtL4D::Initialize (DS)";
	}

	const char* pszPattern = gc->GetKeyValue(pszKey);
	if (pszPattern == NULL) {
		V_snprintf(error, maxlength, "Unable to find signature for \"%s\" from game config (file: \"" GAMEDATA_FILE ".txt\")", pszKey);
		return false;
	}

	if (*pszPattern == '@') {
#if defined PLATFORM_WINDOWS
		MEMORY_BASIC_INFORMATION mem;
		if (VirtualQuery(g_pMatchExtL4D, &mem, sizeof(mem)) != 0) {
			pfn_CMatchExtL4D_Initialize = memutils->ResolveSymbol(mem.AllocationBase, pszPattern + 1);
		}
#elif defined PLATFORM_POSIX
		Dl_info info;
		if (dladdr(g_pMatchExtL4D, &info) != 0) {
			void* handle = dlopen(info.dli_fname, RTLD_NOW);
			if (handle) {
				pfn_CMatchExtL4D_Initialize = memutils->ResolveSymbol(handle, pszPattern + 1);
				dlclose(handle);
			}
		}
#endif
	}
	else {
		byte bytes[512];
		size_t written = UTIL_DecodeHexString(bytes, sizeof(bytes), pszPattern);
		pfn_CMatchExtL4D_Initialize = memutils->FindPattern(g_pMatchExtL4D, reinterpret_cast<char*>(bytes), written);
	}

	if (pfn_CMatchExtL4D_Initialize == NULL) {
		V_snprintf(error, maxlength, "Sigscan for \"%s\" failed (game config file: \"" GAMEDATA_FILE ".txt\")", pszKey);
		return false;
	}

	return true;
}

bool CExtension::SetupFromMatchmakingLibrary(char* error, int maxlength)
{
	char path[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, path, sizeof(path), "bin/matchmaking%s." PLATFORM_LIB_EXT, engine->IsDedicatedServer() ? MATCHMAKING_LIB_DS_SUFFIX : "");

	char libErr[256];
	ke::RefPtr<ke::SharedLib> mms_lib = ke::SharedLib::Open(path, libErr, sizeof(libErr));
	if (!mms_lib) {
		V_snprintf(error, maxlength, "Unable to load matchmaking library \"%s\" (error: \"%s\")", path, libErr);
		return false;
	}

	CreateInterfaceFn matchmakingFactory = mms_lib->get<CreateInterfaceFn>(CREATEINTERFACE_PROCNAME);
	if (matchmakingFactory == NULL) {
		V_snprintf(error, maxlength, "Unable to retrieve matchmaking factory (file: \"%s\")", path);
		return false;
	}

	g_pMatchFramework = static_cast<IMatchFramework*>(matchmakingFactory(IMATCHFRAMEWORK_VERSION_STRING, NULL));
	if (g_pMatchFramework == NULL) {
		V_snprintf(error, maxlength, "Unable to find interface \"" IMATCHFRAMEWORK_VERSION_STRING "\" (path: \"%s\")", path);
		return false;
	}

	g_pMatchExtL4D = static_cast<IMatchExtL4D*>(matchmakingFactory(IMATCHEXT_L4D_INTERFACE, NULL));
	if (g_pMatchExtL4D == NULL) {
		V_snprintf(error, maxlength, "Unable to find interface \"" IMATCHEXT_L4D_INTERFACE "\" (path: \"%s\")", path);
		return false;
	}

	return true;
}

void CExtension::NotifyMissionReload(bool bFireForwards)
{
	g_iCurrentModeSymbol = GetCurrentMode();
	g_iCurrentMissionSymbol = GetCurrentMission();

	for ([[maybe_unused]] auto& [plugin, pubVars] : m_PubVars) {
		if (pubVars.m_pCurModeVar != NULL) {
			*pubVars.m_pCurModeVar->offs = g_iCurrentModeSymbol;
		}

		if (pubVars.m_pCurMissionVar != NULL) {
			*pubVars.m_pCurMissionVar->offs = g_iCurrentMissionSymbol;
		}
	}

	UpdateDisabledModes(sm_imatchext_disable_modes.GetString());
	UpdateDisabledMissions(sm_imatchext_disable_missions.GetString());

	if (bFireForwards) {
		for (const auto& pIface : m_Listeners) {
			pIface->NotifyMissionReload();
		}

		g_fwdOnMissionCacheReload->Execute();
	}
}

void CExtension::UpdateDisabledModes(const char* pszSettings)
{
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues* pkvAllModes = g_pMatchExtL4D->GetAllModes();

	// Clear up
	for (KeyValues* pkvMode = pkvAllModes->GetFirstTrueSubKey(); pkvMode != NULL; pkvMode = pkvMode->GetNextTrueSubKey()) {
		KeyValues* pkvKey = pkvMode->FindKey("disabled");
		if (pkvKey != NULL) {
			pkvKey->SetInt(NULL, 0);
		}
	}

	if (*pszSettings == '\0') {
		return;
	}

	const char* pszEnd = NULL;
	const char* pszToken = pszSettings;
	while (1) {
		pszEnd = strchr(pszToken, ';');

		char token[64];
		if (pszEnd == NULL) {
			V_strncpy(token, pszToken, sizeof(token));
		}
		else {
			V_strncpy(token, pszToken, MIN(pszEnd - pszToken + 1, sizeof(token)));
		}

		if (*token != '\0') {
			KeyValues* pkvMode = pkvAllModes->FindKey(token);
			if (pkvMode != NULL) {
				pkvMode->SetInt("disabled", 1);
			}
		}

		if (pszEnd == NULL) {
			break;
		}

		pszToken = pszEnd + 1;
	}
#endif
}

void CExtension::UpdateDisabledMissions(const char* pszSettings)
{
	KeyValues* pkvAllMissions = g_pMatchExtL4D->GetAllMissions();

	// Clear up
	for (KeyValues* pkvMission = pkvAllMissions->GetFirstTrueSubKey(); pkvMission != NULL; pkvMission = pkvMission->GetNextTrueSubKey()) {
		KeyValues* pkvKey = pkvMission->FindKey("disabled");
		if (pkvKey != NULL) {
			pkvKey->SetInt(NULL, 0);
		}
	}

	if (*pszSettings == '\0') {
		return;
	}

	const char* pszEnd = NULL;
	const char* pszToken = pszSettings;
	while (1) {
		pszEnd = strchr(pszToken, ';');

		char token[64];
		if (pszEnd == NULL) {
			V_strncpy(token, pszToken, sizeof(token));
		}
		else {
			V_strncpy(token, pszToken, MIN(pszEnd - pszToken + 1, sizeof(token)));
		}

		if (*token != '\0') {
			KeyValues* pkvMission = pkvAllMissions->FindKey(token);
			if (pkvMission != NULL) {
				pkvMission->SetInt("disabled", 1);
			}
		}

		if (pszEnd == NULL) {
			break;
		}

		pszToken = pszEnd + 1;
	}
}

bool CExtension::RegisterConCommandBase(ConCommandBase* pVar)
{
	// Notify metamod of ownership
	return META_REGCVAR(pVar);
}

IMatchExtL4D* CExtension::GetIMatchExtL4D()
{
	return g_pMatchExtL4D;
}

IMatchFramework* CExtension::GetIMatchFrameWork()
{
	return g_pMatchFramework;
}

void CExtension::AddListener(IMatchExtListener* listener)
{
	m_Listeners.push_front(listener);
}

void CExtension::RemoveListener(IMatchExtListener* listener)
{
	auto prev = m_Listeners.before_begin();
	for (auto cur = m_Listeners.cbegin(); cur != m_Listeners.cend(); cur = prev++) {
		if (*cur == listener) {
			m_Listeners.erase_after(prev);
			break;
		}
	}
}

void CExtension::OnPluginLoaded(IPlugin* plugin)
{
	IPluginRuntime* pPluginRuntime = plugin->GetRuntime();
	if (pPluginRuntime == NULL) {
		return;
	}

	PubVars_t pubVars = { NULL };

	uint32_t index = 0;
	if (pPluginRuntime->FindPubvarByName("CurrentMode", &index) == SP_ERROR_NONE) {
		pPluginRuntime->GetPubvarByIndex(index, &pubVars.m_pCurModeVar);
	}

	if (pPluginRuntime->FindPubvarByName("CurrentMission", &index) == SP_ERROR_NONE) {
		pPluginRuntime->GetPubvarByIndex(index, &pubVars.m_pCurMissionVar);
	}

	if (pubVars.m_pCurModeVar != NULL || pubVars.m_pCurMissionVar != NULL) {
		m_PubVars.try_emplace(plugin, pubVars);
	}
}

void CExtension::OnPluginUnloaded(IPlugin* plugin)
{
	m_PubVars.erase(plugin);
}
