#include <sourcemod>

#define REQUIRE_EXTENSIONS
#include <imatchextl4d>

public Action test_print_details(int args)
{
	KeyValues kv = new KeyValues("");
	if (GetServerDetails(kv)) {
		char buffer[512];
		
		PrintToServer(" -- Server Info -- ");
		
		kv.GetString("server/adronline", buffer, sizeof(buffer));
		PrintToServer("IP Address: %s", buffer);
		
		kv.GetString("server/name", buffer, sizeof(buffer));
		PrintToServer("Server Name: %s", buffer);
		
		kv.GetString("game/mode", buffer, sizeof(buffer));
		PrintToServer("Gamemode: %s", buffer);
		
		KeyValues mode_kv = new KeyValues("");
		if (GetGameModeInfo(mode_kv, buffer)) {
			PrintToServer(" -- Gamemode Info -- ");
			
			char base[64];
			mode_kv.GetString("base", base, sizeof(base));
			if (strcmp(buffer, base)) {
				PrintToServer("Based on: %s", base);
			}
			
			PrintToServer("Max players: %d, Player zombies: %s", mode_kv.GetNum("maxplayers", 4), mode_kv.GetNum("playercontrolledzombies") > 0 ? "yes" : "no");
			
			PrintToServer("Single chapter: %s, Has difficulty: %s", !!mode_kv.GetNum("singlechapter") ? "yes" : "no", !!mode_kv.GetNum("hasdifficulty") ? "yes" : "no");
		}
		
		delete mode_kv;
		
		// GetMapInfo first argument requires keys:
		// game/mode
		// game/mission
		// game/chapter
		KeyValues map_kv = new KeyValues(""), mission_kv = new KeyValues("");
		if (GetMapInfo(kv, map_kv, mission_kv)) {
			// mission_kv is not guaranteed to change
			PrintToServer(" -- Mission Info -- ");
			
			mission_kv.GetString("name", buffer, sizeof(buffer));
			PrintToServer("Name: %s (%s)", buffer, mission_kv.GetNum("builtin") ? "Built In" : "Addon");
			
			mission_kv.GetString("version", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Version: %s", buffer);
			}
			
			mission_kv.GetString("displaytitle", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Title: %s", buffer);
			}
			
			mission_kv.GetString("description", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Description: %s", buffer);
			}
			
			mission_kv.GetString("author", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Author: %s", buffer);
			}
			
			PrintToServer(" -- Map Info -- ");
			
			map_kv.GetString("map", buffer, sizeof(buffer));
			PrintToServer("Map: %s", buffer);
			
			map_kv.GetString("displayname", buffer, sizeof(buffer));
			PrintToServer("Name: %s", buffer);
			
			kv.SetNum("game/chapter", kv.GetNum("game/chapter") + 1);
			PrintToServer("Chapter: %d, Is final map: %s", map_kv.GetNum("chapter"), GetMapInfo(kv) ? "no" : "yes");
		}
		
		delete map_kv;
		delete mission_kv;
	}
	else {
		PrintToServer("GetServerDetails returned false");
	}
	
	delete kv;
	
	return Plugin_Handled;
}

public void OnPluginStart()
{
	RegServerCmd("test_print_details", test_print_details);
}

public Plugin myinfo = 
{
	name = "[L4D/2] Matchmaking Extension Interface Test",
	author = "shqke",
	description = "Matchmaking Extension Interface test",
	version = "1.0.1",
	url = "https://github.com/shqke/imatchextl4d"
};
