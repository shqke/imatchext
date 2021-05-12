#if defined _imatchext_included
 #endinput
#endif
#define _imatchext_included

native bool GetServerDetails(Handle kvResult, Handle kvRequest = null);
native bool GetAllMissions(Handle kvResult);
// request: (string) "Game/mode"; (string) "Game/campaign"; (int) "Game/chapter"
native bool GetMapInfo(Handle kvRequest, Handle kvMapInfo = null, Handle kvMissionInfo = null);
// request: (string) "Game/mode"
native bool GetMapInfoByBspName(Handle kvRequest, const char[] name, Handle kvMapInfo = null, Handle kvMissionInfo = null);

// Natives below are for Left 4 Dead 2 only
native bool GetAllModes(Handle kvResult);
native bool GetGameModeInfo(Handle kvResult, const char[] name);

public Extension __ext_imatchext =
{
	name = "[L4D/2] Matchmaking Extension Interface",
	file = "imatchext.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
	required = 0,
};

public __ext_imatchext_SetNTVOptional()
{
#if !defined REQUIRE_EXTENSIONS
	MarkNativeAsOptional("GetServerDetails");
	MarkNativeAsOptional("GetAllMissions");
	MarkNativeAsOptional("GetMapInfo");
	MarkNativeAsOptional("GetMapInfoByBspName");
#endif
	MarkNativeAsOptional("GetAllModes");
	MarkNativeAsOptional("GetGameModeInfo");
}
