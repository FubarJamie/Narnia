
public Action Command_Profile(int client, int args) {

	Handle menu = CreateMenu(Profile_MenuHandler);
	
	SetMenuTitle(menu, "My Profile");
	
	char displayStr[128];
	
	Format(displayStr, sizeof(displayStr), "Credits: %i\n", Store_GetClientCredits(client) );
	
	AddMenuItem(menu, "-1", displayStr, ITEMDRAW_DISABLED);
	
	int DonateTime = Donate_GetTime(client);
	
	if (DonateTime == -1) {
		Format(displayStr, sizeof(displayStr), "VIP Time: no.\n");
	} else {
		Format(displayStr, sizeof(displayStr), "VIP Time: %i\n", DonateTime);
	}
	
	Format(displayStr, sizeof(displayStr), "VIP Time: %i\n", Donate_GetTime(client));
	
	AddMenuItem(menu, "-1", displayStr, ITEMDRAW_DISABLED);
	
	
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

public int Profile_MenuHandler(Handle menu, MenuAction action, int param1, int param2) {

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
	}

}
