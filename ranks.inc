#include "csgocolors"

stock LoadRanks() {

	decl String:sPath[PLATFORM_MAX_PATH];
	
	BuildPath(Path_SM, sPath, sizeof(sPath), "configs/timer/rankings.cfg");

	new Handle:hKv = CreateKeyValues("Rankings");
	
	if (!FileToKeyValues(hKv, sPath))
	{
		CloseHandle(hKv);
		return;
	}

	if (!KvGotoFirstSubKey(hKv))
	{
		CloseHandle(hKv);
		return;
	}

  	int iteration = 0;
	do {
		decl String:sSectionName[32];
		KvGetSectionName(hKv, sSectionName, sizeof(sSectionName));
		
		//PrintToChatAll("%i -> %s", iteration, sSectionName);
		
		if (StrEqual(sSectionName, "Points", false)) {
			
			g_PointsGiven[Points_OnFirstFinish] = KvGetNum(hKv, "points_first");
			
			continue;
		}
		
		if (StrEqual(sSectionName, "WorldRecord", false)) {
			new Handle:hKvCopy = CreateKeyValues("WorldRecords");
			KvCopySubkeys(hKv, hKvCopy);
			
			if (!KvGotoFirstSubKey(hKvCopy))
			{
				CloseHandle(hKvCopy);
				return;
			}
			
			int rank = 0;
			
			do {
				decl String:sSectionName2[32];
				KvGetSectionName(hKvCopy, sSectionName2, sizeof(sSectionName2));

				g_WRRank[rank] = StringToInt(sSectionName2);
				
				g_WRAdd[rank] = KvGetNum(hKvCopy, "add_points");
				g_WRRemove[rank] = KvGetNum(hKvCopy, "remove_points");
				g_WRImprove[rank] = KvGetNum(hKvCopy, "improve_points");
				
				rank++;
				
			} while (KvGotoNextKey(hKvCopy));
			
			CloseHandle(hKvCopy);
			
			continue;
			
		}

		KvGetString(hKv, "info", g_RankNames[iteration], 64);
		KvGetString(hKv, "tag", g_ClanTag[iteration], 64);
		KvGetString(hKv, "chat", g_ChatPrefix[iteration], 64);
      
		g_RankPosition[iteration] = KvGetNum(hKv, "ranks");
		
		//PrintToChatAll("%s -> %s", sSectionName, chatStr);
		//CPrintToChatAll("%s", g_ChatPrefix[iteration]);
        
		iteration++;
		
	} while (KvGotoNextKey(hKv));
		
	CloseHandle(hKv);
}

int PointsByWRRank(int rank, int flag = 0) {
	for (int i = 0; i < MAX_RANKS; i++) {
		if (g_WRRank[(i+1)] == 0)
			break;
			
		if ( (g_WRRank[i] <= rank) && (rank < g_WRRank[(i+1)]) ) {
			if (flag == FLAG_WRADD) {
				return g_WRAdd[i];
			} else if (flag == FLAG_WRREMOVE) {
				return g_WRRemove[i];
			} else if (flag == FLAG_WRIMPROVE) {
				return g_WRImprove[i];
			} else {
				return -1;
			}
		}
	}
	return -1;
}

public int RankByIndex(int rank, int client) {
	if (g_PlayersPoints[client] == 0) 
		return 0;
	for (int i = 1; i < 100; i++) {
		if (StrEqual(g_RankNames[i], "")) {
			break;
		}
		if (rank <= g_RankPosition[i]) {
			return i;
		}
	}
	return -1;
}

public void CreatePlayer(int client) {
	
	if (!IsValidClient(client))
		return;
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	char sQuery[256];
	Format(sQuery, 256, "SELECT COUNT(*) FROM ranks WHERE auth = '%s'", auth);
	
	SQL_TQuery(g_SQLhandle, SQL_CheckPlayer_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_CheckPlayer_Callback");
	#endif
}

public void SQL_CheckPlayer_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) 
		return;

	int client = GetClientFromSerial(data);
	
	if (!IsValidClient(client)) 
		return;
		
	if (SQL_FetchRow(handle)) {
		if (SQL_FetchInt(handle, 0) == 0) {
			
			CPrintToChatAll("Welcome {BLUE}%N{DEFAULT} to the server!", client);
			CPrintToChat(client, "Feel free to join our steam group at fsociety.community!");

			char auth[64];
			GetClientAuthId(client, AuthId_SteamID64, auth, 64);
			
			char clientName[64];
			GetClientName(client, clientName, sizeof(clientName));
			
			int iLength = ((strlen(clientName) * 2) + 1);
			char[] sEscapedName = new char[iLength];
			SQL_EscapeString(g_SQLhandle, clientName, sEscapedName, iLength);
			
			char sQuery[256];
			Format(sQuery, 256, "INSERT INTO `ranks` (`id`, `points`, `lostpoints`, `lastname`, `auth`, `epoch`) VALUES (NULL, 0, 0, '%s', '%s', UNIX_TIMESTAMP(NOW()));", sEscapedName, auth);
			
			PrintToServer("%s", sQuery);
			
			SQL_TQuery(g_SQLhandle, SQL_AddUser_Callback, sQuery, GetClientSerial(client));
			#if defined SQL_DEBUG
				LogMessage("SQL_AddUser_Callback");
			#endif
		} else {
			OnPlayerConnect(client, true);
		}
	}
}

public void SQL_AddUser_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null) 
		return;

	int client = GetClientFromSerial(data);
	
	if (!IsValidClient(client)) {
		GetPlayersRank(client);
		return;
	}
}

public void AddPoints(int client, int inc) {
	
	if (!IsValidClient(client))
		return;
	
	if (inc <= 0) {
		PrintToServer("Invalid number entered: %i", inc);
		return;
	}
	
	g_PlayersPoints[client] += inc;
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	char sQuery[256];
	Format(sQuery, 256, "UPDATE `ranks` SET `points` = %i,`epoch` = UNIX_TIMESTAMP(NOW()) WHERE auth = '%s'", g_PlayersPoints[client], auth);
	
	SQL_TQuery(g_SQLhandle, SQL_UpdatePoints_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_UpdatePoints_Callback");
	#endif
}

public void RemovePoints(int client, int dec) {
	
	if (!IsValidClient(client))
		return;
	
	if (dec <= 0) {
		PrintToServer("Invalid number entered: %i", dec);
		return;
	}
	
	g_PlayersPoints[client] -= dec;
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);

	char sQuery[256];
	Format(sQuery, 256, "UPDATE `ranks` SET `points` = %i,`epoch` = UNIX_TIMESTAMP(NOW()) WHERE auth = '%s'", g_PlayersPoints[client], auth);
	
	SQL_TQuery(g_SQLhandle, SQL_UpdatePoints_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_UpdatePoints_Callback");
	#endif
}

public void SQL_UpdatePoints_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;
		
	int client = GetClientFromSerial(data);
	
	if (!IsValidClient(client))
		return;
		
	//GetPlayersRank(client);
}

public void RemovePointsOffline(const char[] auth, int dec) {

	if (dec <= 0) {
		PrintToServer("Invalid number entered: %i", dec);
		return;
	}
	
	DataPack data = CreateDataPack();
	
	WritePackString(data, auth);
	WritePackCell(data, dec);
	
	char sQuery[512];
	Format(sQuery, 512, "SELECT `points`, `lostpoints` from ranks WHERE auth = '%s'", auth);
	
	SQL_TQuery(g_SQLhandle, SQL_RemovePointsOffline_Callback, sQuery, data);
	#if defined SQL_DEBUG
		LogMessage("SQL_RemovePointsOffline_Callback");
	#endif
}

public void SQL_RemovePointsOffline_Callback(Handle owner, Handle handle, const char[] error, any data) {
	
	if (handle == null)
		return;
	
	ResetPack(data);
	
	char auth[64];
	
	ReadPackString(data, auth, 64);
	int dec = ReadPackCell(data);
	
	CloseHandle(data);
	
	if (SQL_FetchRow(handle)) {
		int points = SQL_FetchInt(handle, 0);
		int lostpoints = SQL_FetchInt(handle, 1);
		
		points -= dec;
		lostpoints += dec;
		
		char sQuery[512];
		Format(sQuery, 512, "UPDATE `ranks` SET `points` = %i,`lostpoints` = %i WHERE auth = '%s'", points, lostpoints, auth);
		
		SQL_TQuery(g_SQLhandle, SQL_UpdatePoints2_Callback, sQuery);
		#if defined SQL_DEBUG
			LogMessage("SQL_UpdatePoints2_Callback");
		#endif
	}
}

public void SQL_UpdatePoints2_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null)
		return;
}

public void StolenWorldRecord(int client, float time, int style) {
	
	if (!IsValidClient(client))
		return;
	
	DataPack data = CreateDataPack();
	
	WritePackCell(data, GetClientSerial(client));
	WritePackCell(data, style);
	
	char sQuery[512];
	Format(sQuery, 512, "SELECT auth FROM playertimes WHERE time > %.3f AND map = '%s' AND style = %i ORDER BY `time` DESC LIMIT 1", time, g_Map, style);
	
	SQL_TQuery(g_SQLhandle, SQL_GetAuth_Callback, sQuery, data);
	#if defined SQL_DEBUG 
		LogMessage("SQL_GetAuth_Callback");
	#endif
}

public void SQL_GetAuth_Callback(Handle owner, Handle handle, const char[] error, any data) {
	
	if (handle == null)
		return;
		
	ResetPack(data);
	
	int client = GetClientFromSerial(ReadPackCell(data));
	int style = ReadPackCell(data);
	
	CloseHandle(data);
	
	if (SQL_FetchRow(handle)) {
		
		DataPack storage = CreateDataPack();
		
		char auth[64];
		SQL_FetchString(handle, 0, auth, 64);
		
		PrintToServer("%s", auth);
		
		WritePackCell(storage, GetClientSerial(client));
		WritePackString(storage, auth);
		
		char sQuery[512];
		Format(sQuery, 512, "SELECT COUNT(*) FROM playertimes WHERE auth = '%s' AND map = '%s' AND style = %i ORDER BY `time` ASC LIMIT 1", auth, g_Map, style);
		
		SQL_TQuery(g_SQLhandle, SQL_GetRank_Callback, sQuery, storage);
		#if defined SQL_DEBUG
			LogMessage("SQL_GetRank_Callback");
		#endif
	}
}

public void SQL_GetRank_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;
		
	ResetPack(data);
	
	char auth[64];
	
	int client = GetClientFromSerial(ReadPackCell(data));
	ReadPackString(data, auth, 64);
	
	char authCompare[64];
	GetClientAuthId(client, AuthId_SteamID64, authCompare, 64);
	
	if (StrEqual(auth, authCompare)) {
		return;
	}
	
	CloseHandle(data);
	
	//
		
	if (SQL_FetchRow(handle)) {
		int rank = SQL_FetchInt(handle, 0);
		
		int clientBeaten = Client_FindBySteamId(auth);
		
		int points = PointsByWRRank(rank, FLAG_WRREMOVE);
		
		if (clientBeaten == -1) {
			RemovePointsOffline(auth, points);
		} else {
			AddPoints(client, PointsByWRRank(rank));
			PrintToChat(client, "%s You have stolen %i points from %N", PREFIX, points, clientBeaten);
			
			RemovePoints(clientBeaten, points);
			PrintToChat(clientBeaten, "%s %N has stolen %i points from you!", PREFIX, client, points);
		}
	}
}

public void IfLostPointsDisplay(int client) {
	
	if (!IsValidClient(client))
		return;
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	DataPack data = CreateDataPack();
	
	WritePackCell(data, GetClientSerial(client));
	WritePackString(data, auth);
	
	char sQuery[512];
	Format(sQuery, 512, "SELECT lostpoints FROM ranks WHERE auth = '%s'", auth);
	
	SQL_TQuery(g_SQLhandle, SQL_LostPointsDisplay_Callback, sQuery, data);
	#if defined SQL_DEBUG
		LogMessage("SQL_LostPointsDisplay_Callback");
	#endif
}

public void SQL_LostPointsDisplay_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null)
		return;
	char auth[64];
	
	ResetPack(data);
	
	int client = GetClientFromSerial(ReadPackCell(data));
	ReadPackString(data, auth, 64);
	
	CloseHandle(data);
	
	if (SQL_FetchRow(handle)) {
		int lostpoints = SQL_FetchInt(handle, 0);
		
		if (lostpoints == 0) {
			// Weclome player back
		} else {
			PrintToChat(client, "%s While you were gone you lost %i points!", PREFIX, lostpoints);
			char sQuery[512];
			Format(sQuery, 512, "UPDATE `ranks` SET `lostpoints` = 0 WHERE auth = '%s'", auth);
			
			SQL_TQuery(g_SQLhandle, SQL_UpdateLostPoints_Callback, sQuery);
			#if defined SQL_DEBUG
				LogMessage("SQL_UpdateLostPoints_Callback");
			#endif
		}
	}
}

public void SQL_UpdateLostPoints_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null)
		return;
}

public void GetPlayersRank(int client) {
	
	if (!IsValidClient(client)) 
		return;
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	char sQuery[512];
	//76561197965915595
	//SELECT COUNT(*) FROM ranks WHERE points >= (SELECT `points` FROM ranks WHERE auth = '76561197965915595')
	Format(sQuery, 512, "SELECT COUNT(*), MIN(points) FROM ranks WHERE points >= (SELECT `points` FROM ranks WHERE auth = '%s')", auth);
	SQL_TQuery(g_SQLhandle, SQL_GetClientRank_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_GetClientRank_Callback");
	#endif
}

public void SQL_GetClientRank_Callback(Handle owner, Handle handle, const char[] error, any data) {
	
	if (handle == null) 
		return;

	int client = GetClientFromSerial(data);
	
	if (!IsValidClient(client)) 
		return;
	
	if (SQL_FetchRow(handle)) {
 		g_PlayersRank[client] = SQL_FetchInt(handle, 0);
 	}
}

public void GetPlayersPoints(int client, bool connectMessage) {

	if (!IsValidClient(client)) 
		return;
	
	char sQuery[256];
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	DataPack pack = CreateDataPack();
	
	WritePackCell(pack, GetClientSerial(client));
	WritePackCell(pack, connectMessage);
	
	if (connectMessage) 
		Format(sQuery, 256, "SELECT `points`, `lostpoints` FROM ranks WHERE auth = '%s'", auth);
	else
		Format(sQuery, 256, "SELECT `points` FROM ranks WHERE auth = '%s'", auth);
	
	SQL_TQuery(g_SQLhandle, SQL_GetClientPoints_Callback, sQuery, pack);
	#if defined SQL_DEBUG 
		LogMessage("SQL_GetClientPoints_Callback");
	#endif

}

public void SQL_GetClientPoints_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) 
		return;

	ResetPack(data);
	
	int client = GetClientFromSerial(ReadPackCell(data));
	bool displayMessage = ReadPackCell(data);
	
	CloseHandle(data);
	
	if (!IsValidClient(client)) 
		return;
		
	if (SQL_FetchRow(handle)) {
		g_PlayersPoints[client] = SQL_FetchInt(handle, 0);
		if (displayMessage) {
			int lostpoints = SQL_FetchInt(handle, 1);
			if (lostpoints > 0) {
				// Tell user they lost points
				PrintToChat(client, "%s While you were gone you lost %i points!", PREFIX, lostpoints);
				ResetLostPoints(client);
			}
		}
	}
	
	//g_ChatPrefix[ RankByIndex(g_PlayersRank[client]) ]
	
	if (displayMessage) {
		CPrintToChatAll(" %s{BLUE}%N{DEFAULT} has joined the server with {BLUE}%i{DEFAULT} points!", g_ChatPrefix[RankByIndex(g_PlayersRank[client], client)], client, g_PlayersPoints[client]);
	}
}

public void ResetLostPoints(int client) {
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);

	char sQuery[512];
	Format(sQuery, 512, "UPDATE `ranks` SET `lostpoints` = 0 WHERE auth = '%s'", auth);
	
	SQL_TQuery(g_SQLhandle, SQL_ResetLostPoints_Callback, sQuery);
	#if defined SQL_DEBUG 
		LogMessage("SQL_ResetLostPoints_Callback");
	#endif
}

public void SQL_ResetLostPoints_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null)
		return;
}

public void SQL_UpdateMaxRank_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null)
		return;
		
	if (SQL_FetchRow(handle)) 
		g_RanksMaxRecord = SQL_FetchInt(handle, 0);
}

public Action Command_GivePoints(int client, int args) {

	if (args < 2) {
		PrintToChatAll("%s Usage: givepoints <name> <points>", PREFIX);
		return Plugin_Handled;
	} else {
		char name[64];
		GetCmdArg(1, name, 64);
		
		int target = FindTarget(client, name);
		if (target == -1) {
			PrintToChat(client, "%s Target not found %s", PREFIX, name);
			return Plugin_Handled;
		} else {
			
			char points[64];
			
			GetCmdArg(2, points, 64);
			
			int added = StringToInt(points);
			
			AddPoints(target, added);
			
			PrintToChat(client, "%s You have given %N, %i points", PREFIX, target, added);
			PrintToChat(target, "%s You have given %i points by %N", PREFIX, added, client);
		}
	}

	return Plugin_Handled;
}

public Action Command_ShowPoints(int client, int args) {
	PrintToChatAll("%s \x0B%N\x01 is ranked \x0B%i\x01 of \x0B%i\x01 with \x0B%i\x01 points!", PREFIX, client, g_PlayersRank[client], g_RanksMaxRecord, g_PlayersPoints[client]);
}

public Action Command_ShowRank(int client, int args) {
	DisplayRanks(client, 0);
}

DisplayRanks(int client, int item) {
	Handle menu = CreateMenu(Select_Rank_MenuHandler);
	
	int currentRanks;
	for (int i = 0; i < 100; i++) {
		if (StrEqual(g_RankNames[i], "")) {
			currentRanks = i + 1;
			break;
		}
	}
	
	SetMenuTitle(menu, "Chatranks - %i ranks!", currentRanks);

	char temp[3];
	for (int i = 0; i < 100; i++) {
		if (StrEqual(g_RankNames[i], "")) 
			break;
		if (g_RankPosition[i] == -1) 
			continue;
		
		Format(temp, sizeof(temp), "%i", i);
		AddMenuItem(menu, temp, g_RankNames[i]);
	}

	SetMenuExitButton(menu, true);
	DisplayMenuAtItem(menu, client, item, 60);
}

public int Select_Rank_MenuHandler(Handle menu, MenuAction action, int param1, int param2) {
	
	if (action == MenuAction_Select) {
		
		char info[8];
		GetMenuItem(menu, param2, info, 8);
		
		int index = StringToInt(info);
		
		CPrintToChat(param1, "%s%N: Rank \x07%i\x01/\x07%i", g_ChatPrefix[RankByIndex(g_RankPosition[index], param1)], param1, g_RankPosition[index], g_RankPosition[index+1]);

		DisplayRanks(param1, GetMenuSelectionPosition());
	}
}

public void UpdateEpoch(int client) {
	
	char sQuery[1536];
	
	Format(sQuery, sizeof(sQuery), "UPDATE `ranks` SET `epoch`= UNIX_TIMESTAMP(NOW()) WHERE");
	
	int collected = 0;
	
	for (int i = 0; i < MaxClients; i++) {
		if (!IsValidClient(i))
			continue;
		
		char auth[20];
		GetClientAuthId(client, AuthId_SteamID64, auth, 20);
		
		if (collected == 0) 
			Format(sQuery, sizeof(sQuery), "%s auth = '%s'", sQuery, auth);
		else 
			Format(sQuery, sizeof(sQuery), "%s OR auth = '%s'", sQuery, auth);
			
		collected++;
	}
	
	//UPDATE `ranks` SET `epoch`= UNIX_TIMESTAMP(NOW()) WHERE auth = '76561197965915595' OR auth = '76561198220151661'
	
	SQL_TQuery(g_SQLhandle, SQL_UpdateEpoch_Callback, sQuery);

}

public void SQL_UpdateEpoch_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;

}

public Action Command_Top100(int client, int args) {
	
	char sQuery[512];
	Format(sQuery, sizeof(sQuery), "SELECT `points`, `lastname` FROM ranks WHERE points >= 0 ORDER BY points DESC LIMIT 100");
	
	SQL_TQuery(g_SQLhandle, SQL_DisplayRanks_Callback, sQuery, GetClientSerial(client));
	
	#if defined SQL_DEBUG
		LogMessage("SQL_DisplayRanks_Callback");
	#endif

}

public void SQL_DisplayRanks_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;
		
	int client = GetClientFromSerial(data);

	Handle menu = CreateMenu(Select_DisplayRank_MenuHandler);
	
	int rank = 1;
	
	while (SQL_FetchRow(handle)) {
		
		char menuDisplay[128];
		
		char name[64];
		
		int points = SQL_FetchInt(handle, 0);
		
		SQL_FetchString(handle, 1, name, sizeof(name));
		
		Format(menuDisplay, sizeof(menuDisplay), "%s - %i points", name, points);
		
		AddMenuItem(menu, "", menuDisplay);
		
		rank++;
	}
	
	SetMenuTitle(menu, "Top %i players", (rank - 1) );

	SetMenuExitButton(menu, true);
	DisplayMenu(menu, client, 60);

}

public int Select_DisplayRank_MenuHandler(Handle menu, MenuAction action, int param1, int param2) {
	
	if (action == MenuAction_Select) {
		Command_Top100(param1, 0);
	}
}