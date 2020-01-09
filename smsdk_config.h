#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

#define SMEXT_CONF_NAME			"[L4D/2] Matchmaking Extension Interface"
#define SMEXT_CONF_DESCRIPTION	"Exposes some methods from L4D matchmaking extension interface to SourcePawn"
#define SMEXT_CONF_VERSION		"1.0.1"
#define SMEXT_CONF_AUTHOR		"shqke"
#define SMEXT_CONF_URL			"https://github.com/shqke/imatchextl4d"
#define SMEXT_CONF_LOGTAG		"IMATCHEXTL4D"
#define SMEXT_CONF_LICENSE		"GPL"
#define SMEXT_CONF_DATESTRING	__DATE__

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
