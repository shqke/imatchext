#include <sourcemod>

// #undef REQUIRE_EXTENSIONS
#include <imatchext>

public void OnMissionCacheReload()
{
    PrintToServer("Mission cache reloaded");
}

public Action test_imatchext(int args)
{
    char buffer[32];
    
    PrintToServer("Game Modes:");
    
    ModeSymbol mode = ModeSymbol.First();
    for (int n = 1; mode != ModeSymbol_Invalid; mode = mode.Next(), n++) {
        mode.GetName(buffer, sizeof(buffer));
        PrintToServer("mode #%u: \"%s\" (%d)", n, buffer, mode);
        PrintToServer("=========================");
        
        ModeSymbol base = mode.Base;
        if (base != ModeSymbol_Invalid) {
            base.GetName(buffer, sizeof(buffer));
            PrintToServer("base mode: \"%s\"", buffer);
        }
        
        PrintToServer("soft disabled: %s", mode.IsDisabled ? "yes" : "no");
        PrintToServer("addon: %s", mode.IsAddon ? "yes" : "no");
        PrintToServer("max players: %d", mode.MaxPlayers);
        PrintToServer("player zombies: %s", mode.HasPlayerControlledZombies ? "yes" : "no");
        PrintToServer("has difficulty: %s", mode.HasConfigurableDifficultySetting ? "yes" : "no");
        PrintToServer("single chapter: %s", mode.IsSingleChapterMode ? "yes" : "no");
        PrintToServer("");
    }
    
    mode = GetModeSymbol("coop");
    PrintToServer("\"coop\": #%d %s", mode, ModeSymbol.IsValid(mode) ? "yes" : "no");
    
    mode = GetModeSymbol("invalid_game_mode");
    PrintToServer("\"invalid_game_mode\": #%d %s", mode, ModeSymbol.IsValid(mode) ? "yes" : "no");

    PrintToServer("");
    PrintToServer("Missions:");
    
    MissionSymbol mission = MissionSymbol.First();
    for (int n = 1; mission != MissionSymbol_Invalid; mission = mission.Next(), n++) {
        mission.GetName(buffer, sizeof(buffer));
        PrintToServer("mission #%u: \"%s\" (%d)", n, buffer, mission);
        PrintToServer("=========================");
        
        PrintToServer("soft disabled: %s", mission.IsDisabled ? "yes" : "no");
        PrintToServer("addon: %s", mission.IsAddon ? "yes" : "no");
        PrintToServer("allow cola: %s", mission.AllowCola ? "yes" : "no");
        PrintToServer("disallow female boomers: %s", mission.NoFemaleBoomers ? "yes" : "no");
        PrintToServer("disallow cs weapons: %s", mission.NoCSWeapons ? "yes" : "no");
        PrintToServer("survivor set: %s", mission.SurvivorSet == SurvivorSet_L4D2 ? "l4d2" : "l4d1");
        PrintToServer("");
    }
    
    mission = GetMissionSymbol("l4d2c1");
    PrintToServer("\"l4d2c1\": #%d %s", mission, MissionSymbol.IsValid(mission) ? "yes" : "no");
    
    mission = GetMissionSymbol("invalid_mission");
    PrintToServer("\"invalid_mission\": #%d %s", mission, MissionSymbol.IsValid(mission) ? "yes" : "no");
    
    PrintToServer("[imatchext]\n"
        ... "l4d360ui_mode_verbose_mutation1: '\n"
        ... "%T\n"
        ... "'", "l4d360ui_mode_verbose_mutation1", LANG_SERVER);
    
    if (CurrentMode != ModeSymbol_Invalid) {
        char szMode[64];
        CurrentMode.GetName(szMode, sizeof(szMode));
        PrintToServer("Current Mode: %s (#%d)", szMode, CurrentMode);
    }
    
    if (CurrentMission != MissionSymbol_Invalid) {
        char szMission[64];
        CurrentMission.GetName(szMission, sizeof(szMission));
        PrintToServer("Current Mission: %s (#%d)", szMission, CurrentMission);
    }
    
    return Plugin_Handled;
}

public Action test_imatchext_flat(int args)
{
    char buffer[512];

    KeyValues kv = new KeyValues("");
    if (GetServerGameDetails(kv)) {
        PrintToServer(" -- Server Info -- ");
        
        int cookie[2];
        if (GetReservationCookie(cookie)) {
            if (cookie[1] != 0) {
                PrintToServer("Reserved: 0x%08X%08X", cookie[1], cookie[0]);
            }
            else {
                PrintToServer("Not reserved");
            }
        }
        
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
                if (strcmp(buffer, base) != 0) {
                    PrintToServer("Base: %s", base);
                }
                
                PrintToServer("Max Players: %d", kvModeInfo.GetNum("maxplayers", 4));
                PrintToServer("Player Zombies: %s", kvModeInfo.GetNum("playercontrolledzombies") != 0 ? "yes" : "no");
                PrintToServer("Single Chapter: %s", kvModeInfo.GetNum("singlechapter") != 0 ? "yes" : "no");
                PrintToServer("Difficulty: %s", kvModeInfo.GetNum("hasdifficulty") != 0 ? "yes" : "no");
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
            PrintToServer("Chapter #: %d", kvMapInfo.GetNum("chapter"));
            PrintToServer("Is final map: %s", GetMapInfo(kv) ? "no" : "yes");
        }
        
        delete kvMapInfo;
        delete kvMissionInfo;
    }
    else {
        PrintToServer("GetServerDetails returned false");
    }

    delete kv;

    PrintToServer("");

    return Plugin_Handled;
}

public void OnPluginStart()
{
    LoadTranslations("imatchext.phrases");
    
    RegServerCmd("test_imatchext", test_imatchext);
    RegServerCmd("test_imatchext_flat", test_imatchext_flat);
}

public Plugin myinfo = 
{
    name = "[L4D/2] Matchmaking Extension Interface Test",
    author = "shqke",
    description = "Matchmaking Extension Interface test",
    version = "1.2.0",
    url = "https://github.com/shqke/imatchext/"
};
