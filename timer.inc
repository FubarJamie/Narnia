/**
 *	Calculates time spent on course except when paused
 *
 *  @param client                Player's index
 *	@return float                Player's time
 */
public float CalculateTime(int client) {
	if(!g_TimerPaused[client])
		return GetEngineTime() - g_StartTime[client] - g_PauseTotalTime[client];
	else
		return g_PauseStartTime[client] - g_StartTime[client] - g_PauseTotalTime[client];
}

public Action Command_StartTimer(int client, int args) {
	
	if (GetClientTeam(client) == 1) {
		return Plugin_Handled;
	}
	
	if(!IsValidClient(client))
		return Plugin_Handled;

	int count = -1;
	for (int i = 0; i < MAX_ZONES; i++) {
		if (!EmptyZone(g_MapZones[i][Zone_Start][0]) && !EmptyZone(g_MapZones[i][Zone_Start][1])) {
			count++;
		}
	}
	
	int startZone = Math_GetRandomInt(0, count);

	float vCenter[3];
	
	GetCenterPos(g_MapZones[startZone][Zone_Start][0], g_MapZones[startZone][Zone_Start][1], vCenter);
	
	float pVelo[3] = { 0.0, 0.0, 0.0 };
	
	TeleportEntity(client, vCenter, NULL_VECTOR, pVelo);

	StartTimer(client);

	return Plugin_Handled;
}

public Action Command_StopTimer(int client, int args) {
	
	if (GetClientTeam(client) == 1) {
		return Plugin_Handled;
	}
	
	if (!IsValidClient(client)) 
		return Plugin_Handled;
	
	StopTimer(client);
	
	return Plugin_Handled;
}

public Action Command_TogglePause(int client, int args) {
	
	if (GetClientTeam(client) == 1) {
		return Plugin_Handled;
	}
	
	if (!IsValidClient(client)) 
		return Plugin_Handled;
	
	if(g_TimerPaused[client])
		ResumeTimer(client);
	else 
		PauseTimer(client);
	
	return Plugin_Handled;
}

public Action Command_AutoBhop(int client, int args) {
	if (!IsValidClient(client))
		return Plugin_Handled;
	
	if (g_AutoBhop[client]) {
		ReplyToCommand(client, "%s Auto Jump is now [\x09OFF\x01]", PREFIX);
		g_AutoBhop[client] = false;
		g_CurrentStyle[client] = Style_Legit;
	} else {
		ReplyToCommand(client, "%s Auto Jump is now [\x06ON\x01]", PREFIX);
		g_AutoBhop[client] = true;
		g_CurrentStyle[client] = Style_Auto;
	}
		
	return Plugin_Handled;
}

/**
 *	Starts a players timer.
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void StartTimer(int client)
{
	if(!IsValidClient(client, true) || IsFakeClient(client))
		return;

	g_TimerEnabled[client] = true;
	g_Jumps[client] = 0;
	g_StartTime[client] = GetEngineTime();
	g_PauseTotalTime[client] = 0.0;
	g_TimerPaused[client] = false;
}

/**
 *	Stops a players timer.
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void StopTimer(int client)
{
	if(!IsValidClient(client, true))
		return;

	g_TimerEnabled[client] = false;
	g_Jumps[client] = 0;
	g_StartTime[client] = 0.0;
	g_PauseTotalTime[client] = 0.0;
	g_TimerPaused[client] = false;
}

/**
 *	Pauses a players timer.
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void PauseTimer(int client)
{
	if(!IsValidClient(client, true) || IsFakeClient(client))
		return;

	g_PauseStartTime[client] = GetEngineTime();
	g_TimerPaused[client] = true;
}

/**
 *	Resumes a players timer.
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void ResumeTimer(int client)
{
	if(!IsValidClient(client, true) || IsFakeClient(client))
		return;

	g_PauseTotalTime[client] += GetEngineTime() - g_PauseStartTime[client];
	g_TimerPaused[client] = false;
}

/**
 *	Collect all data for finishing run
 *
 *  @param client                Player's index
 *	@param style                 Style the player completed the run with.
 *	@param time                  Time it took for player to complete run
 *	@param jumps                 How many times the player jumped during the run
 *	@noreturn
 */
public void Timer_OnFinish(int client, BhopStyle sstyle, float time, int jumps)
{
	
	if (!IsValidClient(client, true)) 
		return;
	
	//Client_Shake(client);
	BhopStyle style = g_CurrentStyle[client];
	
	char sTime[32];
	FormatSeconds(time, sTime, 32);

	// k people I made this forward so I'll use it to make cool text messages on WR (check timer-misc soon™)
	if(time < g_WRTime[style] || g_WRTime[style] == 0.0) {// WR?

		/*Call_StartForward(gH_OnWorldRecord);
		Call_PushCell(client);
		Call_PushCell(style);
		Call_PushCell(time);
		Call_PushCell(jumps);
		Call_Finish();*/

		UpdateWRCache();
	}

	// 0 - no query
	// 1 - insert
	// 2 - update
	int overwrite;
	
	if(g_PlayerRecord[client][style] == 0.0) {
		//PrintToChatAll("Insert Query %f", g_PlayerRecord[client][style]);
		overwrite = 1;
	} else if(time < g_PlayerRecord[client][style]) {
		//PrintToChatAll("Update Query %f, %f", time, g_PlayerRecord[client][style])
		overwrite = 2;
	}
	
	float fDifference = Math_Abs(g_PlayerRecord[client][style] - time);
	
	char sDifference[16];
	FormatSeconds(fDifference, sDifference, 16, true);
	
	if(overwrite > 0) {
		char sAuthID[64];
		GetClientAuthId(client, AuthId_SteamID64, sAuthID, 64);

		char sQuery[512];

		if (overwrite == 1) { // Insert
			//[Auto] - Hoodrich has finished in 23.232 improving by 1.00 Rank: 3/200
			//PrintToChatAll("%s \x03%N\x01 finished (%s) on \x07%s\x01 with %d jumps.", PREFIX, client, bsStyle == Style_Auto? "Auto":"Legit", sTime, jumps);
			FormatEx(sQuery, 512, "INSERT INTO playertimes (auth, map, time, jumps, date, style) VALUES ('%s', '%s', %.03f, %d, CURRENT_TIMESTAMP(), '%d');", sAuthID, g_Map, time, jumps, style);
		} else { // update
		
			//PrintToChatAll("%s \x03%N\x01 finished (%s) on \x07%s\x01 with %d jumps. \x0C(%s)", PREFIX, client, bsStyle == Style_Auto? "Auto":"Legit", sTime, jumps, sDifference);
			FormatEx(sQuery, 512, "UPDATE playertimes SET time = '%.03f', jumps = '%d', date = CURRENT_TIMESTAMP() WHERE map = '%s' AND auth = '%s' AND style = '%d';", time, jumps, g_Map, sAuthID, style);
		}
		
		//PrintToChatAll("%s", sQuery);
		
		Handle datapack = CreateDataPack();
		WritePackCell(datapack, GetClientSerial(client));
		WritePackCell(datapack, overwrite);
		WritePackCell(datapack, time);
		WritePackCell(datapack, fDifference);
		WritePackCell(datapack, view_as<int>(style));
		WritePackCell(datapack, g_RealTimePosWR[client][g_CurrentStyle[client]]);
		
		SQL_TQuery(g_SQLhandle, SQL_OnFinish_Callback, sQuery, datapack);
		#if defined SQL_DEBUG
			LogMessage("SQL_OnFinish_Callback");
		#endif
		
	} else {
		if(!overwrite) {
			// Finished with time spare
			//PrintToChat(client, "%s You have finished (%s) on \x07%s\x01 with %d jumps. \x08(+%s)", PREFIX, bsStyle == Style_Auto? "Auto":"Legit", sTime, jumps, sDifference);
			//PrintToChat(client, " \x08[\x05%s\x08] - \x01You finished in \x0B%s", g_StyleNames[style], sTime);
			//PrintToChat(client, " \x08[\x05%s\x08] - \x01Time not improved \x0B+%s \x01to beat personal best.", g_StyleNames[style], sDifference);
			
			PrintToChat(client, " \x01[\x06%s\x01]\x05 \x01You finished in \x0B%s\x01 missed out on improving your run by \x09+%s", g_StyleNames[style], sTime, sDifference);
		}
	}
}

public void SQL_OnFinish_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if(handle == null) {
		LogError("Timer (WR OnFinish) SQL query failed. Reason: %s", error);
		return;
	}
	
	ResetPack(data);
	int client = GetClientFromSerial(ReadPackCell(data));
	int overwrite = ReadPackCell(data);
	float time = ReadPackCell(data);
	float fDiff = ReadPackCell(data);
	int style = ReadPackCell(data);
	int RealTime = ReadPackCell(data);
	CloseHandle(data);

	if(!client)
		return;

	UpdateWRCache();
	UpdateClientCache(client);
	
	DataPack datapack = CreateDataPack();
	WritePackCell(datapack, GetClientSerial(client));
	WritePackCell(datapack, overwrite);
	WritePackCell(datapack, time);
	WritePackCell(datapack, fDiff);
	WritePackCell(datapack, RealTime);
	
	Timer_GetMaxWR(style);

	char sQuery[512];
	FormatEx(sQuery, sizeof(sQuery), "SELECT COUNT(*) FROM `playertimes` WHERE `map` = '%s' AND `time` <= '%.3f' AND style = '%i' ORDER BY `time` ASC", g_Map, time, style);
	SQL_TQuery(g_SQLhandle, SQL_WRDisplay_Callback, sQuery, datapack); //[Auto] - Hoodrich has finished in 23.232 improving by 1.00 Rank: 3/200
	#if defined SQL_DEBUG
		LogMessage("SQL_WRDisplay_Callback");
	#endif
}

public void SQL_WRDisplay_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null)
		return;
		
	ResetPack(data);
	
	int client = GetClientFromSerial(ReadPackCell(data));
	int overwrite = ReadPackCell(data);
	float time = ReadPackCell(data);
	float fDiff = ReadPackCell(data);
	int RealRank = ReadPackCell(data);
	
	CloseHandle(data);
	
	if (SQL_FetchRow(handle)) {
		
		char name[MAX_NAME_LENGTH];
		GetClientName(client, name, MAX_NAME_LENGTH);
		
		char sTime[32];
		FormatSeconds(time, sTime, sizeof(sTime));
		
		char sDiff[32];
		FormatSeconds(fDiff, sDiff, sizeof(sDiff));
		
		int style = view_as<int>(g_CurrentStyle[client]);
		
		int maxRank = g_SMaxRecords[style];
		
		/*int rank = SQL_FetchInt(handle, 0);
		
		if (rank == RealRank) {
			PrintToServer("Rank is correct!");
		} else {
			PrintToServer("Rank was inncorrect! %i, %i", rank, RealRank);
		}
		
		if (rank == 0) {
			rank = 1;
		}*/
		
		int rank = RealRank;
		
		if (overwrite == 1) {

			if (rank == 1) {
				
				if (maxRank == 1) {
					PrintToChatAll(" \x01[\x05%s\x01] \x0B%N\x01 finished in \x0B%s\x01 beating the world record - Rank: \x0B%i\x01/\x0B%i", g_StyleNames[style], client, sTime, rank, maxRank);
				} else {
					PrintToChatAll(" \x01[\x05%s\x01] \x0B%N\x01 finished in \x0B%s\x01 beating the world record - Rank: \x0B%i\x01/\x0B%i", g_StyleNames[style], client, sTime, rank, maxRank);
				}
				
				AddPoints(client, (PointsByWRRank(rank) + g_PointsGiven[Points_OnFirstFinish]));
				PrintToChat(client, "%s You have gained %i points +%i for beating the world record!", PREFIX, PointsByWRRank(rank), g_PointsGiven[Points_OnFirstFinish]);
			} else {
				PrintToChatAll(" \x01[\x05%s\x01] \x0B%N\x01 finished in \x0B%s\x01 - Rank: \x0B%i\x01/\x0B%i", g_StyleNames[style], client, sTime, rank, maxRank);
				
				AddPoints(client, (PointsByWRRank(rank) + g_PointsGiven[Points_OnFirstFinish]));
				PrintToChat(client, "%s You have gained %i points +%i for completing the map", PREFIX, PointsByWRRank(rank), g_PointsGiven[Points_OnFirstFinish]);
			}
			StolenWorldRecord(client, time, style);
		} else {
			if (fDiff == 0.000) {
				PrintToChat(client, " \x08[\x05%s\x08] \x01You finished in \x0B%s", g_StyleNames[style], sTime);
				PrintToChat(client, " \x08[\x05%s\x08] \x01Time not improved \x0B+%s \x01to beat personal best.", g_StyleNames[style], sDiff);
			} else {
				
				if (rank == 1) {
					PrintToChatAll(" \x01[\x05%s\x01] \x0B%N\x01 finished in \x0B%s\x01 beating the world record by \x04-%s\x01 - Rank: \x0B%i\x01/\x0B%i", g_StyleNames[style], client, sTime, sDiff, rank, maxRank);
					
					AddPoints(client, PointsByWRRank(rank, FLAG_WRADD));
					PrintToChat(client, "%s You have gained %i points for beating the world record!", PREFIX, PointsByWRRank(rank, FLAG_WRADD));
				} else {
					PrintToChatAll(" \x01[\x05%s\x01] \x0B%N\x01 finished in \x0B%s\x01 improving their run by \x04-%s\x01 - Rank: \x0B%i\x01/\x0B%i", g_StyleNames[style], client, sTime, sDiff, rank, maxRank);
					
					AddPoints(client, PointsByWRRank(rank, FLAG_WRIMPROVE));
					PrintToChat(client, "%s You gained %i points for improving your time", PREFIX, PointsByWRRank(rank, FLAG_WRIMPROVE));
				}
				StolenWorldRecord(client, time, style);
			}
		}
	}

}

/**
 *	Starts a players timer.
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void UpdateClientCache(int client) {
	char sAuthID[64];
	GetClientAuthId(client, AuthId_SteamID64, sAuthID, 64);

	char sQuery[512];
	FormatEx(sQuery, 512, "SELECT time, style FROM playertimes WHERE map = '%s' AND auth = '%s';", g_Map, sAuthID);
	SQL_TQuery(g_SQLhandle, SQL_UpdateCache_Callback, sQuery, GetClientSerial(client), DBPrio_High);
	#if defined SQL_DEBUG
		LogMessage("SQL_UpdateCache_Callback");
	#endif
}

public void SQL_UpdateCache_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if(handle == null) {
		LogError("Timer (PB cache update) SQL query failed. Reason: %s", error);
		return;
	}

	int client = GetClientFromSerial(data);

	if(!client)
		return;

	while(SQL_FetchRow(handle))
		g_PlayerRecord[client][SQL_FetchInt(handle, 1)] = SQL_FetchFloat(handle, 0);
}

/**
 *	Reload the World Record cache
 *
 *  @noparam
 *	@noreturn
 */
public void UpdateWRCache() {
	char sQuery[256];
	FormatEx(sQuery, 256, "SELECT u.name, p.time FROM playertimes p JOIN users u ON p.auth = u.auth WHERE map = '%s' AND style = '0' ORDER BY time ASC LIMIT 1;", g_Map);
	SQL_TQuery(g_SQLhandle, SQL_UpdateWRCache_Callback, sQuery, 0, DBPrio_High);
	#if defined SQL_DEBUG
		LogMessage("SQL_UpdateWRCache_Callback");
	#endif

	// I FUCKING KNOW THERE'S A WAY TO DO THIS IN 1 QUERY BUT I SUCK AT SQL SO FORGIVE PLS ;-;
	FormatEx(sQuery, 256, "SELECT u.name, p.time FROM playertimes p JOIN users u ON p.auth = u.auth WHERE map = '%s' AND style = '1' ORDER BY time ASC LIMIT 1;", g_Map);
	SQL_TQuery(g_SQLhandle, SQL_UpdateWRCache_Callback, sQuery, 1, DBPrio_High);
	#if defined SQL_DEBUG
		LogMessage("SQL_UpdateWRCache_Callback");
	#endif
}

public void SQL_UpdateWRCache_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	if(handle == null) {
		LogError("Timer (WR forwards cache update) SQL query failed. Reason: %s", error);
		return;
	}

	if(!SQL_FetchRow(handle)) {
		FormatEx(g_WRName[data], MAX_NAME_LENGTH, "invalid");
		g_WRTime[data] = 0.0;
	} else {
		SQL_FetchString(handle, 0, g_WRName[data], MAX_NAME_LENGTH);
		g_WRTime[data] = SQL_FetchFloat(handle, 1);
	}
}

/**
 *	Gets all World Records from the database and load it.
 *
 *  @noparam
 *	@noreturn
 */
public void GetCurrentRecords() {

	// Get Top records for styles
	for (int i = 0; i < MAX_STYLES; i++) {
		//g_WRTime[i];
		char sQuery[512];
		Format(sQuery, 512, "SELECT u.name, p.time FROM playertimes p JOIN users u ON p.auth = u.auth WHERE map = '%s' AND style = '%i' ORDER BY time ASC LIMIT 1;", g_Map, i);
		SQL_TQuery(g_SQLhandle, SQL_SetTopRecords_Callback, sQuery, i);
		#if defined SQL_DEBUG
			LogMessage("SQL_SetTopRecords_Callback");
		#endif
	}
	
	// Get top records for each player and styles.
	
	for (int i = 1; i < MaxClients; i++) {
		if (!IsValidClient(i)) {
			continue;
		}
		
		char sAuthID3[64];
		GetClientAuthId(i, AuthId_SteamID64, sAuthID3, 64);
		
		char sQuery[512];
		Format(sQuery, 512, "SELECT time, style FROM playertimes WHERE map = '%s' AND auth = '%s'", g_Map, sAuthID3);
		
		SQL_TQuery(g_SQLhandle, SQL_SetPlayerTopRecords_Callback, sQuery, GetClientSerial(i));
		
		#if defined SQL_DEBUG
			LogMessage("SQL_SetPlayerTopRecords_Callback");
		#endif
		
	}

}

public void SQL_SetTopRecords_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) {
		LogError("Timer (Set Top Records) SQL query failed. Reason: %s", error);
		return;
	}
	
	if (!SQL_FetchRow(handle)) {
		FormatEx(g_WRName[data], MAX_NAME_LENGTH, "INVALID");
		g_WRTime[data] = 0.0;
	} else {
		SQL_FetchString(handle, 0, g_WRName[data], MAX_NAME_LENGTH);
		g_WRTime[data] = SQL_FetchFloat(handle, 1);
	}
}

public void LoadClientRecords(int client) {

	if (!IsValidClient(client))
		return;
	
	char sAuthID3[64];
	GetClientAuthId(client, AuthId_SteamID64, sAuthID3, 64);
	
	char sQuery[512];
	Format(sQuery, 512, "SELECT time, style FROM playertimes WHERE map = '%s' AND auth = '%s'", g_Map, sAuthID3);
	
	SQL_TQuery(g_SQLhandle, SQL_SetPlayerTopRecords_Callback, sQuery, GetClientSerial(client));
	
	#if defined SQL_DEBUG
		LogMessage("SQL_SetPlayerTopRecords_Callback");
	#endif

}

public void SQL_SetPlayerTopRecords_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) {
		LogError("Timer (Set Player Top Records) SQL query failed. Reason: %s", error);
		return;
	}
	
	int client = GetClientFromSerial(data);
	
	for (int i = 0; i < MAX_STYLES; i++) {
		g_PlayerRecord[client][i] = 0.0;
	}
	
	while (SQL_FetchRow(handle)) {
		g_PlayerRecord[client][SQL_FetchInt(handle, 1)] = SQL_FetchFloat(handle, 0);
	}
	
}

public Action Command_Auto(int client, int args) {
	if(!IsValidClient(client))
		return Plugin_Handled;
	
	g_CurrentStyle[client] = Style_Auto;
	
	if (g_AutoBhop[client]) {
		ReplyToCommand(client, "%s Auto Jump is now [\x09OFF\x01]", PREFIX);
		g_AutoBhop[client] = false;
		g_CurrentStyle[client] = Style_Legit;
	} else {
		ReplyToCommand(client, "%s Auto Jump is now [\x06ON\x01]", PREFIX);
		g_AutoBhop[client] = true;
		g_CurrentStyle[client] = Style_Auto;
	}
	
	//ReplyToCommand(client, "%s You have selected to play \x03%s", PREFIX, g_StyleNames[Style_Type]);
	
	StopTimer(client);

	Command_StartTimer(client, -1);
	return Plugin_Handled;
}

/*public Action Command_Legit(int client, int args) {
	if(!IsValidClient(client))
		return Plugin_Handled;
	
	g_CurrentStyle[client] = Style_Legit;
	g_AutoBhop[client] = false;
	
	ReplyToCommand(client, "%s Auto Jump is now [\x09OFF\x01]", PREFIX);
	//ReplyToCommand(client, "%s You have selected to play \x03%s", PREFIX, g_StyleNames[Style_Type]);
	
	StopTimer(client);

	Command_StartTimer(client, -1);
	return Plugin_Handled;
}*/

public Action Command_AddRecord(int client, int args) {

	// !record Steve 23.435 23 Legit

	if (args == 4) {
		char auth[20];
		char convert[64];
		float time;
		int jumps, style;
		
		//GetClientAuthId(client, AuthId_SteamID64, auth);
		
		GetCmdArg(1, convert, sizeof(convert));
		int target = FindTarget(client, convert, true);
		
		if (target == -1) {
			PrintToChat(client, "%s%s couldn't be found.", PREFIX, convert);
			return Plugin_Handled;
		}
		
		GetClientAuthId(target, AuthId_SteamID64, auth, sizeof(auth));
		
		GetCmdArg(2, convert, sizeof(convert));
		time = StringToFloat(convert);

		GetCmdArg(3, convert, sizeof(convert));
		jumps = StringToInt(convert);
		
		GetCmdArg(4, convert, sizeof(convert));

		for (int i = 0; i < MAX_STYLES; i++) {
			if (StrContains(convert, g_StyleNames[i], false) == -1) {
				style = -1;
			} else {
				style = i;
				PrintToChat(client, "%s (%s) has been found (%i)", g_StyleNames[i], convert, i);
				break;
			}
		}
		
		if (style == -1) {
			PrintToChat(client, "%s %s couldn't be found.", PREFIX, convert);
			return Plugin_Handled;
		}
		
		char sQuery[512];
		FormatEx(sQuery, 512, "INSERT INTO playertimes (auth, map, time, jumps, date, style) VALUES ('%s', '%s', %.03f, %d, CURRENT_TIMESTAMP(), '%d');", auth, g_Map, time, jumps, style);
		SQL_TQuery(g_SQLhandle, SQL_AddRecord_Callback, sQuery, GetClientSerial(client));

	} else {
		PrintToChat(client, "%sUsage: !addrecord <name> <time> <jumps> <style>", PREFIX);
	}
	return Plugin_Handled;

}

public void SQL_AddRecord_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null) 
		return;

	int client = GetClientFromSerial(data);

	PrintToChat(client, "%s Time has been added to the database.", PREFIX);
	GetCurrentRecords();

}