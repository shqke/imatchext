#ifndef _INCLUDE_IMATCHEXT_EXT_H_
#define _INCLUDE_IMATCHEXT_EXT_H_

#include <smsdk_ext.h>
#include "../IMatchExtInterface.h"
#include "../ILanguageFileParser.h"
#include "phrasesgen.h"

#include <forward_list>
#include <unordered_map>

extern IMatchFramework* g_pMatchFramework;
extern IMatchExtL4D* g_pMatchExtL4D;
extern ILanguageFileParser* g_pLanguageFileParser;
extern ConVar* mp_gamemode;

class CExtension :
	public CPhrasesGenerator,
	public SDKExtension,
	public IMatchExtInterface,
	public IConCommandBaseAccessor,
	public IPluginsListener,
	public IMetamodListener
{
public:
	void NotifyMissionReload(bool bFireForwards);
	void UpdateDisabledModes(const char* pszSettings);
	void UpdateDisabledMissions(const char* pszSettings);

private:
	bool SetupFromGameConfig(IGameConfig* gc, char* error, int maxlength);
	bool SetupFromMatchmakingLibrary(char* error, int maxlength);

public: // IConCommandBaseAccessor
	// Flags is a combination of FCVAR flags in cvar.h.
	// hOut is filled in with a handle to the variable.
	bool RegisterConCommandBase(ConCommandBase* pVar) override;

private:
	std::forward_list<IMatchExtListener*> m_Listeners;

public: // IMatchExtInterface
	IMatchExtL4D* GetIMatchExtL4D() override;
	IMatchFramework* GetIMatchFrameWork() override;

	void AddListener(IMatchExtListener* pListener) override;
	void RemoveListener(IMatchExtListener* pListener) override;

private:
	struct PubVars_t
	{
		sp_pubvar_t* m_pCurModeVar;
		sp_pubvar_t* m_pCurMissionVar;
	};

	// Pointers to pubvars data per plugin
	std::unordered_map<IPlugin*, PubVars_t> m_PubVars;

public: // IPluginsListener
	// @brief Called when a plugin's required dependencies and natives have
	// been bound. Plugins at this phase may be in any state Failed or
	// lower. This is invoked immediately before OnPluginStart, and sometime
	// after OnPluginCreated.
	void OnPluginLoaded(IPlugin* plugin) override;

	// @brief Called when a plugin is about to be unloaded. This is called for
	// any plugin for which OnPluginLoaded was called, and is invoked
	// immediately after OnPluginEnd(). The plugin may be in any state Failed
	// or lower.
	//
	// This function must not cause the plugin to re-enter script code. If
	// you wish to be notified of when a plugin is unloading, and to forbid
	// future calls on that plugin, use OnPluginWillUnload and use a
	// plugin property to block future calls.
	void OnPluginUnloaded(IPlugin* plugin) override;

public: // IMetamodListener
	/**
	 * @brief Called when the level is loaded (after GameInit, before
	 * ServerActivate).
	 *
	 * To override this, hook IServerGameDLL::LevelInit().
	 *
	 * @param pMapName		Name of the map.
	 * @param pMapEntities	Lump string of the map entities, in KeyValues
	 * 						format.
	 * @param pOldLevel		Unknown.
	 * @param pLandmarkName	Unknown.
	 * @param loadGame		Unknown.
	 * @param background	Unknown.
	 */
	void OnLevelInit(char const* pMapName,
		char const* pMapEntities,
		char const* pOldLevel,
		char const* pLandmarkName,
		bool loadGame,
		bool background) override;

public: // SDKExtension
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlen	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	bool SDK_OnLoad(char* error, size_t maxlen, bool late) override;

	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	void SDK_OnUnload() override;

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	void SDK_OnAllLoaded() override;

	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	bool SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late) override;

public: // IExtensionInterface
	/**
	 * @brief Asks the extension whether it's safe to remove an external
	 * interface it's using.  If it's not safe, return false, and the
	 * extension will be unloaded afterwards.
	 *
	 * NOTE: It is important to also hook NotifyInterfaceDrop() in order to clean
	 * up resources.
	 *
	 * @param pInterface		Pointer to interface being dropped.  This
	 * 							pointer may be opaque, and it should not
	 *							be queried using SMInterface functions unless
	 *							it can be verified to match an existing
	 *							pointer of known type.
	 * @return					True to continue, false to unload this
	 * 							extension afterwards.
	 */
	bool QueryInterfaceDrop(SMInterface* pInterface) override;

	/**
	 * @brief Notifies the extension that an external interface it uses is being removed.
	 *
	 * @param pInterface		Pointer to interface being dropped.  This
	 * 							pointer may be opaque, and it should not
	 *							be queried using SMInterface functions unless
	 *							it can be verified to match an existing
	 */
	void NotifyInterfaceDrop(SMInterface* pInterface) override;
};

#endif // _INCLUDE_IMATCHEXT_EXT_H_
