#if defined _imatchextl4d_included
 #endinput
#endif
#define _imatchextl4d_included

native bool GetAllMissions(Handle out);
native bool GetAllModes(Handle out);
// request: (string) "Game/mode"; (string) "Game/campaign"; (int) "Game/chapter"
native bool GetMapInfo(Handle request, Handle out_map = null, Handle out_mission = null);
// request: (string) "Game/mode"
native bool GetMapInfoByBspName(Handle request, const char[] name, Handle out_map = null, Handle out_mission = null);
native bool GetGameModeInfo(Handle out, const char[] name);
native bool GetServerDetails(Handle out, Handle request = null);

public Extension __ext_imatchextl4d =
{
	name = "[L4D/2] Matchmaking Extension Interface",
	file = "imatchextl4d.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public __ext_imatchextl4d_SetNTVOptional()
{
	MarkNativeAsOptional("GetAllMissions");
	MarkNativeAsOptional("GetAllModes");
	MarkNativeAsOptional("GetMapInfo");
	MarkNativeAsOptional("GetMapInfoByBspName");
	MarkNativeAsOptional("GetGameModeInfo");
	MarkNativeAsOptional("GetServerDetails");
}
#endif
