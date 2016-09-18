
public Action Command_Profile(int client, int args) {

	Handle menu = CreateMenu(Profile_MenuHandler);
	
	char title[256];
	
	char displayStr[128];
	
	Format(displayStr, sizeof(displayStr), "Credits: %i\n", Store_GetClientCredits(client) );
	
	char DonateTime[128];
	Format(DonateTime, sizeof(DonateTime), "%s", Donate_GetTime(client));
	
	if (StrContains(DonateTime, "-1")) {
		Format(DonateTime, sizeof(DonateTime), "VIP Time: Not VIP\n");
	} else {
		Format(DonateTime, sizeof(DonateTime), "VIP Time: %i\n", DonateTime);
	}
	
	char PlayTime[128];
	
	int timePlayed = curPlayTime[client] + (GetTime() - epoch[client]);
	
	float played = float(timePlayed);
	
	Format(PlayTime, sizeof(PlayTime), "Play Time: %.4f Hours\n",  ((played / 60) / 60) );
	
	Format(title, sizeof(title), "My Profile\n \n%s%s%s \n", displayStr, DonateTime, PlayTime);
	
	SetMenuTitle(menu, title);

	AddMenuItem(menu, "0", "Timer Settings");
	
	DisplayMenu(menu, client, 120);

}

public int Profile_MenuHandler(Handle menu, MenuAction action, int param1, int param2) {

	if (action == MenuAction_Select) {
		
		char info[4];
		GetMenuItem(menu, param2, info, sizeof(info));
		
		if (StrEqual(info, "0")) {
			Command_Timer(param1, 0);
		}
	}

}

public Action Command_Timer(int client, int args) {
	Handle menu = CreateMenu(Timer_DropDown_MenuHandler);
	
	SetMenuTitle(menu, "My Profile");
	
	if (g_ShowChatRank[client]) {
		AddMenuItem(menu, "0", "Hide my chat rank");
	} else {
		AddMenuItem(menu, "1", "Show my chat rank");
	}
	
	if (g_HideHud[client]) {
		AddMenuItem(menu, "2", "Enable Hud");
	} else {
		AddMenuItem(menu, "3", "Disable Hud");
	}
	
	DisplayMenu(menu, client, 120);
	
}

public int Timer_DropDown_MenuHandler(Handle menu, MenuAction action, int param1, int param2) {
	if (action == MenuAction_Select) {
		
		char info[4];
		GetMenuItem(menu, param2, info, sizeof(info));
		
		if (StrEqual(info, "0")) {
			g_ShowChatRank[param1] = false;
			PrintToChat(param1, "%s Your chat rank will now be hidden", PREFIX);
		} else if (StrEqual(info, "1")) {
			g_ShowChatRank[param1] = true;
			PrintToChat(param1, "%s Your chat rank will now be shown", PREFIX);
		} else if (StrEqual(info, "2")) {
			g_HideHud[param1] = false;
			PrintToChat(param1, "%s Your hud has been enabled", PREFIX);
		} else if (StrEqual(info, "3")) {
			g_HideHud[param1] = true;
			PrintToChat(param1, "%s Your hud has been disabled", PREFIX);
		}
		
	} else if (action == MenuAction_Cancel) {
		Command_Profile(param1, 0);
	}
}

public int Client_GetTime(int client) {

	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);

	char sQuery[256];
	Format(sQuery, 256, "SELECT `playtime` FROM users WHERE auth = '%s'", auth);
	
	SQL_TQuery(g_SQLhandle, SQL_GetTimeStore_Callback, sQuery, GetClientSerial(client));

}

public void SQL_GetTimeStore_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;
		
	int client = GetClientFromSerial(data);

	if (SQL_FetchRow(handle)) {
		
		curPlayTime[client] = SQL_FetchInt(handle, 0);
	}

}

public int Client_SaveTime(int client) {

	int playTime = curPlayTime[client] + (GetTime() - epoch[client]);

	char sQuery[512];
	Format(sQuery, sizeof(sQuery), "UPDATE users SET `playtime` = '%i' WHERE `auth` = '%s'", playTime, g_PlayerAuth[client]);
	
	SQL_TQuery(g_SQLhandle, SQL_UpdateTime_Callback, sQuery);

}

public void SQL_UpdateTime_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;
}