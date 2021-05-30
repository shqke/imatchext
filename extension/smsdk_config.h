#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

#define SMEXT_CONF_NAME			"[L4D/2] Matchmaking Extension Interface"
#define SMEXT_CONF_DESCRIPTION	"Exposes some methods from matchmaking extension interface to SourcePawn"
#define SMEXT_CONF_VERSION		"1.3.0"
#define SMEXT_CONF_AUTHOR		"Evgeniy \"shqke\" Kazakov"
#define SMEXT_CONF_URL			"https://github.com/shqke/imatchext/"
#define SMEXT_CONF_LOGTAG		"imatchext"
#define SMEXT_CONF_LICENSE		"GPL"
#define SMEXT_CONF_DATESTRING	__DATE__

#define GAMEDATA_FILE			"imatchext"

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#define SMEXT_CONF_METAMOD
#define SMEXT_ENABLE_GAMECONF
#define SMEXT_ENABLE_MEMUTILS
#define SMEXT_ENABLE_FORWARDSYS
#define SMEXT_ENABLE_TRANSLATOR
#define SMEXT_ENABLE_THREADER
#define SMEXT_ENABLE_PLUGINSYS

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
