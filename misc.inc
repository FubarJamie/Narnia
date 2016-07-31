public Action Command_Skip(int client, int args) {

	for (int slot = 0; slot < 5; slot++) {
	
		int weaponIndex;
		while ((weaponIndex = GetPlayerWeaponSlot(client, slot)) != -1) {
			RemovePlayerItem(client, weaponIndex);
			RemoveEdict(weaponIndex);
			//GetClientAuthId(client, AuthId_SteamID64);
		}
	}
}

public Action Command_Knife(int client, int args) {

	if (!IsValidClient(client, true)) {
		PrintToChat(client, "You are dead");
		return;
	}
	
	char sWeapon[64];
	Client_GetActiveWeaponName(client, sWeapon, 64);
	
	//GetPlayerWeaponSlot(int client, int slot)

	//Client_RemoveAllWeapons(client, "", true);
	
	//Client_HasWeapon(

	if (!Client_HasWeapon(client, "weapon_knife")) {
		GivePlayerItem(client, "weapon_knife", 0);
	}
	
	if (!Client_HasWeapon(client, "weapon_hkp2000")) {
		GivePlayerItem(client, "weapon_hkp2000", 0);
	}

}

public Action Command_Colours(int client, int args) {
	PrintToChatAll("\x011 \x022 \x033 \x044 \x055 \x066 \x077 \x088 \x099 \x0AA \x0BB \x0CC \x0DD \x0EE \x0FF");
}

public Action Command_Spectate(int client, int args) {

	if (args < 1) {
		// Goto spectate
		ChangeClientTeam(client, 1);
		PrintToChatAll("%s %N has joined spectator mode.", PREFIX, client);
		return Plugin_Handled;
	} else {
		char name[64];
		GetCmdArg(1, name, 64);
		
		int target = FindTarget(client, name);
		if (target == -1) {
			PrintToChat(client, "%s Target not found %s", PREFIX, name);
		} else if (client == target) {
			PrintToChat(client, "%s You cannot spectate your self.", PREFIX);
		} else {
			
			//StartFollowingPlayer(client, target);
			
			PrintToChat(client, "%s Spectating %N now", PREFIX, target);
		}
	}
	
	return Plugin_Handled;
}

public Action Command_HideHud(int client, int args) {

	if (g_HideHud[client]) {
		g_HideHud[client] = false;
		PrintToChat(client, "%s Your hud has been enabled", PREFIX);
	} else {
		g_HideHud[client] = true;
		PrintToChat(client, "%s Your hud has been disabled", PREFIX);
	}

	return Plugin_Handled;

}