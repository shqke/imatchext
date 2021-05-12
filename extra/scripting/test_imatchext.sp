#include <sourcemod>
#include <imatchext>

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
		
		if (GetFeatureStatus(FeatureType_Native, "GetGameModeInfo") == FeatureStatus_Available) {
			// Only available in Left 4 Dead 2
			KeyValues kvModeInfo = new KeyValues("");
			if (GetGameModeInfo(kvModeInfo, buffer)) {
				PrintToServer(" -- Gamemode Info -- ");
				
				char base[64];
				kvModeInfo.GetString("base", base, sizeof(base));
				if (strcmp(buffer, base)) {
					PrintToServer("Based on: %s", base);
				}
				
				PrintToServer("Max players: %d, Player zombies: %s", kvModeInfo.GetNum("maxplayers", 4), kvModeInfo.GetNum("playercontrolledzombies") > 0 ? "yes" : "no");
				
				PrintToServer("Single chapter: %s, Has difficulty: %s", !!kvModeInfo.GetNum("singlechapter") ? "yes" : "no", !!kvModeInfo.GetNum("hasdifficulty") ? "yes" : "no");
			}
			
			delete kvModeInfo;
		}
		
		// GetMapInfo first argument requires keys:
		// game/mode
		// game/mission
		// game/chapter
		KeyValues kvMapInfo = new KeyValues(""), kvMissionInfo = new KeyValues("");
		if (GetMapInfo(kv, kvMapInfo, kvMissionInfo)) {
			// kvMissionInfo is not guaranteed to change
			PrintToServer(" -- Mission Info -- ");
			
			kvMissionInfo.GetString("name", buffer, sizeof(buffer));
			PrintToServer("Name: %s (%s)", buffer, kvMissionInfo.GetNum("builtin") ? "Built In" : "Addon");
			
			kvMissionInfo.GetString("version", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Version: %s", buffer);
			}
			
			kvMissionInfo.GetString("displaytitle", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Title: %s", buffer);
			}
			
			kvMissionInfo.GetString("description", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Description: %s", buffer);
			}
			
			kvMissionInfo.GetString("author", buffer, sizeof(buffer));
			if (buffer[0]) {
				PrintToServer("Author: %s", buffer);
			}
			
			PrintToServer(" -- Map Info -- ");
			
			kvMapInfo.GetString("map", buffer, sizeof(buffer));
			PrintToServer("Map: %s", buffer);
			
			kvMapInfo.GetString("displayname", buffer, sizeof(buffer));
			PrintToServer("Name: %s", buffer);
			
			kv.SetNum("game/chapter", kv.GetNum("game/chapter") + 1);
			PrintToServer("Chapter: %d, Is final map: %s", kvMapInfo.GetNum("chapter"), GetMapInfo(kv) ? "no" : "yes");
		}
		
		delete kvMapInfo;
		delete kvMissionInfo;
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
	version = "1.1.0",
	url = "https://github.com/shqke/imatchext/"
};
