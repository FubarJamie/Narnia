

public void OnPlayerConnect(int client, bool connectMessage) {

	if (!IsValidClient(client))
		return;
	GetPlayersPoints(client, connectMessage);
	
}


public Action Player_Spawned(Event event, const char[] name, bool dontBroadcast) {
	int client = GetClientOfUserId(event.GetInt("userid"));
	
	g_LastZone[client] = Zone_Count;
	g_ExtraLife[client] = true;

	ResumeTimer(client);
	StopTimer(client);

	float vPos[3];
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", vPos);
	
	for (int i = 1; i < 7; i++) {
		//TE_SetupBeamPoints(vPos, vPos2, g_BeamSprite, 0, 0, 0, 0.1, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, {255,255,255,255}, 0);
		
		int colours[4] =  { 0, 0, 0, 255 };
		
		for (int x = 0; x < 3; x++) {
			colours[x] = Math_GetRandomInt(0, 255);
		}
		
		TE_SetupBeamRingPoint(vPos, 60.0, 60.1, g_BeamSprite, g_HaloSprite, 0, 0, 2.5, 1.5, 0.5, colours, 0, FBEAM_FADEOUT);
		TE_SendToAll();
		vPos[2] += 12.5;
	}
	
	/*if (g_HideSave[client]) {
		g_Hide[client] = true;
		g_HideSave[client] = false;
		PrintToChat(client, " [\x03SourceRuns\x01] - Players are now [\x09HIDDEN\x01]");
	}*/
	
}

public Action Player_Disconnect(Event event, const char[] name, bool dontBroadcast) {

	/*int client = GetClientOfUserId(GetEventInt(event, "userid"));
	
	char reason[64], Gname[32], networkId[22];
	
	GetEventString(event, "reason", reason, 64);
	GetEventString(event, "name", Gname, 32);
	GetEventString(event, "networkid", networkId, 22);
	
	CPrintToChatAll("{YELLOW}[{RED}Disconnect{YELLOW}] {DEFAULT}- {BLUE}%N{DEFAULT} has left the server with {BLUE}%i{DEFAULT} points! Reason: {PURPLE}%s", client, g_PlayersPoints[client], reason);
	*/
	
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	
	char reason[22];
	GetEventString(event, "reason", reason, sizeof(reason));
	
	CPrintToChatAll("{YELLOW}[{RED}%s{YELLOW}] {DEFAULT}- {BLUE}%N{DEFAULT} has left the server with {BLUE}%i{DEFAULT} points!", reason, client, g_PlayersPoints[client]);

	return Plugin_Handled;
}

public void OnClientDisconnect(int client) {
	
	// Timer
	StopTimer(client);
	// zones
	Reset(client);
	
	g_AutoBhop[client] = true;
	g_Jumping[client] = false;
	g_JumpIteration[client] = 0;
	g_CurrentButtons[client] = 0;
	gBS_LastWR[client] = Style_Auto;
	
	for (int i = 0; i < MAX_STYLES; i++) {
		g_PlayerRecord[client][i] = 0.0;
		g_CurrPosWR[client][i] = 0;
		g_RealTimePosWR[client][i] = 0;
	}
	
	g_CurrentStyle[client] = Style_Auto;
	g_LastZone[client] = Zone_Start;
	g_Jumps[client] = 0;
	
	g_PlayersPoints[client] = 0;
	g_PlayersRank[client] = 0;
	
	//CPrintToChatAll("{YELLOW}[{RED}Disconnect{YELLOW}] {DEFAULT}- {BLUE}%N{DEFAULT} has left the server with {BLUE}%i{DEFAULT} points!", client, g_PlayersPoints[client]);
	
}

public void OnClientPostAdminCheck(int client) {

	//SDKHook_WeaponDropPost

	//SDKHook(client, SDKHook_WeaponDropPost, Hook_WeaponDrop);
	//SDKHook(client, SDKHook_WeaponCanUse, OnWeaponCanUse);

}

public void OnClientPutInServer(int client) {

	g_Hide[client] = false; 
	SDKHook(client, SDKHook_SetTransmit, Hook_SetTransmit); 

	OnClientInServer(client);
	
	CreatePlayer(client);
	
	LoadClientRecords(client);
	GetPlayersRank(client);
	
	UpdateClientCache(client);
	
	g_ShowChatRank[client] = true;
	
	float fTime = CalculateTime(client);
	GetRLPosWR(client, fTime, view_as<int>(g_CurrentStyle[client]));
	GetPosWR(client, view_as<int>(g_CurrentStyle[client]));
	Timer_GetMaxWR(view_as<int>(g_CurrentStyle[client]));

}

public Action Player_Connect(Event event, const char[] name, bool dontBroadcast) {
	return Plugin_Handled;
}

public Action Player_Team_Change(Event event, const char[] name, bool dontBroadcast) {
	return Plugin_Handled;
}

public Action Player_Say(Event event, const char[] name, bool dontBroadcast) {
	
	GetEventInt(event, "userid");
	
	char text[256];
	
	GetEventString(event, "text", text, 256);
	
	if (StrEqual(text, "", false)) return Plugin_Handled;
	
	CPrintToChatAll("%s", text);
	
	return Plugin_Handled;
}

public Action Command_SayChat(int client, int args) {
	
	if (args >= 1) {
		char text[256];
		
		GetCmdArg(1, text, 256);
		
		if (StrEqual(text, "", false)) return Plugin_Handled;
		
		if (!IsValidClient(client)) {
			CPrintToChatAll("%N: %s", client, text);
			return Plugin_Handled;
		}
		
		char auth[64];

		GetClientAuthId(client, AuthId_SteamID64, auth, 64);
		
		if (StrEqual(auth, "76561198220151661")) {
			
			CPrintToChatAll("[{DARKRED}D{PURPLE}E{GREEN}V{DEFAULT}] %N: %s", client, text);
			PrintToServer("[DEV] %N: %s", client, text);
			return Plugin_Handled;
		}
		
		if (g_ShowChatRank[client]) {
			CPrintToChatAll("%s%N: %s", g_ChatPrefix[RankByIndex(g_PlayersRank[client], client)], client, text);
		} else {
			CPrintToChatAll("%N: %s", client, text);
		}
		
		PrintToServer("%N: %s", client, text);
		
		return Plugin_Handled;
	}
	
	return Plugin_Continue;
}

public void OnMapStart() {
	
	// Zones
	GetCurrentMap(g_Map, 128);
	UnloadZones(0);
	g_BeamSprite = PrecacheModel("materials/sprites/laserbeam.vmt");
	g_HaloSprite = PrecacheModel("materials/sprites/halo01.vmt");
	RefreshZones();
	
	UpdateWRCache();
	GetCurrentRecords();
	
	ServerCommand("mp_freezetime 0");
	ServerCommand("sm_cvar mp_warmuptime 0");
	ServerCommand("mp_do_warmup_period 0");
	ServerCommand("sm_cvar sv_enablebunnyhopping 1");
	//ServerCommand("sm_cvar mp_weapons_allow_map_placed 2");
	
	for (int i = 1; i < MaxClients; i++) {
		
		if (!IsValidClient(i)) {
			continue;
		}
		
		UpdateClientCache(i);
	}
	
	SQL_TQuery(g_SQLhandle, SQL_UpdateMaxRank_Callback, "SELECT COUNT(*) FROM ranks");
	
	#if defined SQL_DEBUG
		LogMessage("SQL_UpdateMaxRank_Callback");
	#endif
	
	for (int i = 1; i < MaxClients; i++) {
		if (!IsValidClient(i))
			continue;
		
		float fTime = CalculateTime(i);
		GetRLPosWR(i, fTime, view_as<int>(g_CurrentStyle[i]));
		GetPosWR(i, view_as<int>(g_CurrentStyle[i]));
		Timer_GetMaxWR(view_as<int>(g_CurrentStyle[i]));
	}
}

public Action CS_OnCSWeaponDrop(int client, int weaponIndex) {
	RemovePlayerItem(client, weaponIndex);
	return Plugin_Handled;
}


public void Player_Jump(Handle event, const char[] name, bool dontBroadcast) {
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	if(g_TimerEnabled[client])
		g_Jumps[client]++;
	
}

public void Player_Death(Handle event, const char[] name, bool dontBroadcast) {
	int client = GetClientOfUserId(GetEventInt(event, "userid"));

	ResumeTimer(client);
	StopTimer(client);
	
	g_TimerEnabled[client] = false;
	
	/*if (g_Hide[client]) {
		g_Hide[client] = false;
		g_HideSave[client] = true;
		PrintToChat(client, " [\x03SourceRuns\x01] - Players are now [\x09VISIBLE\x01]");
	}*/
	
}

public void OnClientInServer(int client) {
	
	// zones
	Reset(client);
	
	// Timer 
	
	StopTimer(client);

	g_CurrentStyle[client] = Style_Auto;

	if(!Client_IsValid(client) || !Client_IsIngame(client) || g_SQLhandle == null)
		return;
		
	if (IsFakeClient(client))
		return;
	
	char sAuthID3[64];
	GetClientAuthId(client, AuthId_SteamID64, sAuthID3, 64);

	char sName[MAX_NAME_LENGTH];
	GetClientName(client, sName, MAX_NAME_LENGTH);

	int iLength = ((strlen(sName) * 2) + 1);
	char[] sEscapedName = new char[iLength];
	SQL_EscapeString(g_SQLhandle, sName, sEscapedName, iLength);

	char sIP[32];
	GetClientIP(client, sIP, 32);

	char sCountry[45];
	GeoipCountry(sIP, sCountry, 45);

	if(StrEqual(sCountry, ""))
		FormatEx(sCountry, 45, "Local Area Network");

	char sQuery[512];
	FormatEx(sQuery, 512, "REPLACE INTO users (auth, name, country, ip) VALUES ('%s', '%s', '%s', '%s');", sAuthID3, sEscapedName, sCountry, sIP);

	SQL_TQuery(g_SQLhandle, SQL_InsertUser_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_InsertUser_Callback");
	#endif
}