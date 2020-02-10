#if defined _imatchext_included
 #endinput
#endif
#define _imatchext_included

native bool GetServerDetails(Handle out, Handle request = null);
native bool GetAllMissions(Handle out);
// request: (string) "Game/mode"; (string) "Game/campaign"; (int) "Game/chapter"
native bool GetMapInfo(Handle request, Handle out_map = null, Handle out_mission = null);
// request: (string) "Game/mode"
native bool GetMapInfoByBspName(Handle request, const char[] name, Handle out_map = null, Handle out_mission = null);

// Natives below are for Left 4 Dead 2 only
native bool GetAllModes(Handle out);
native bool GetGameModeInfo(Handle out, const char[] name);

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
