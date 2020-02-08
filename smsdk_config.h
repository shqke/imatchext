#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

#if SOURCE_ENGINE == SE_LEFT4DEAD
#define GAME_FOLDER "left4dead"
#elif SOURCE_ENGINE == SE_LEFT4DEAD2
#define GAME_FOLDER "left4dead2"
#endif

#if defined(_WIN32)
#define MATCHMAKING_DS_LIB "matchmaking_ds.dll"
#elif SOURCE_ENGINE == SE_LEFT4DEAD
#define MATCHMAKING_DS_LIB "matchmaking_ds.so"
#elif SOURCE_ENGINE == SE_LEFT4DEAD2
#define MATCHMAKING_DS_LIB "matchmaking_ds_srv.so"
#endif

#define SMEXT_CONF_NAME			"[L4D/2] Matchmaking Extension Interface"
#define SMEXT_CONF_DESCRIPTION	"Exposes some methods from matchmaking extension interface to SourcePawn"
#define SMEXT_CONF_VERSION		"1.1.0"
#define SMEXT_CONF_AUTHOR		"shqke"
#define SMEXT_CONF_URL			"https://github.com/shqke/imatchext/"
#define SMEXT_CONF_LOGTAG		"IMATCHEXT"
#define SMEXT_CONF_LICENSE		"MIT"
#define SMEXT_CONF_DATESTRING	__DATE__

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
