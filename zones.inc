#if defined JumesLoadedZones
	#endinput
#endif
#define JumesLoadedZones

/**
 *	Unloads zones by slot, but if slot = 0 unloads all zones
 *
 *	@param slot                   Slot for all types of zones
 *	@noreturn
 */
public void UnloadZones(int slot) {
	if (!slot) {
		for (int i = 0; i < MAX_ZONES; i++) {
			for (int x = 0; x < ZONE_COUNT; x++) {
				for (int c = 0; c < 3; c++) {
					g_MapZones[i][x][0][c] = 0.0;
					g_MapZones[i][x][1][c] = 0.0;
				}
			}
		}
	} else {
		for (int x = 0; x < ZONE_COUNT; x++) {
			for (int i = 0; i < 3; i++) {
				g_MapZones[slot][x][0][i] = 0.0;
				g_MapZones[slot][x][1][i] = 0.0;
			}
		}
	}
}

/**
 *	Unloads zones by SQL id.
 *
 *	@param id                    SQL id of a zone 
 *	@noreturn
 */
public void UnloadZoneBySQLID(int id) {
	for (int i = 0; i < MAX_ZONES; i++) {
		for (int x = 0; x < ZONE_COUNT; x++) {
			if (g_MapZoneSQLInt[i][x] == id) {
				for (int c = 0; c < 3; c++) {
					g_MapZones[i][x][0][c] = 0.0;
					g_MapZones[i][x][1][c] = 0.0;
				}
			}
		}
	}
}

/**
 *	Unloads zones by a type
 *
 *	@param zoneType              A type of zone
 *	@noreturn
 */
public void UnloadZoneByType(MapZones zoneType) {
	for (int i = 0; i < MAX_ZONES; i++) {
		for (int x = 0; x < 3; x++) {
			g_MapZones[i][view_as<int>(zoneType)][0][x] = 0.0;
			g_MapZones[i][view_as<int>(zoneType)][1][x] = 0.0;
		}
	} 
}

/**
 *	Checks if a client is inside a type of zone
 *
 *  @param client                Player's index
 *	@param zoneType              A type of zone
 *	@noreturn
 */
public int InsideZoneByType(int client, MapZones zoneType) {
	for (int i = 0; i < MAX_ZONES; i++) {
		if (EmptyZone(g_MapZones[i][zoneType][0]) && EmptyZone(g_MapZones[i][zoneType][1]))
			continue;
		if (InsideZone(client, g_MapZones[i][zoneType][0], g_MapZones[i][zoneType][1]))
			return true;
	}
	return false;
}

/**
 *	Refreshes all zones
 *
 *  @noparam
 *	@noreturn
 */
public void RefreshZones()
{
	char sQuery[256];
	FormatEx(sQuery, 256, "SELECT type, corner1_x, corner1_y, corner1_z, corner2_x, corner2_y, corner2_z, id FROM mapzones WHERE map = '%s';", g_Map);
	
	SQL_TQuery(g_SQLhandle, SQL_RefreshZones_Callback, sQuery, DBPrio_High);
	#if defined SQL_DEBUG
		LogMessage("SQL_RefreshZones_Callback");
	#endif
}

public void SQL_RefreshZones_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	if(handle == null) {
		LogError("Timer (zone refresh) SQL query failed. Reason: %s", error);
		return;
	}
	
	int zoneIteration[ZONE_COUNT] =  { 0, ... };
	
	while(SQL_FetchRow(handle))
	{
		MapZones type = view_as<MapZones>(SQL_FetchInt(handle, 0));
		
		for (int i = 0; i < 3; i++) {
			g_MapZones[zoneIteration[type]][type][0][i] = SQL_FetchFloat(handle, (i + 1));
			g_MapZones[zoneIteration[type]][type][1][i] = SQL_FetchFloat(handle, (i + 4));
		}
		
		g_MapZoneSQLInt[zoneIteration[type]][type] = SQL_FetchInt(handle, 7);
		
		zoneIteration[type]++;
	}
}

/**
 *	Resets the placement of new zones
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void Reset(int client) {
	g_MapStep[client] = 0;
	
	for(int i = 0; i < 3; i++) {
		g_Point1[client][i] = 0.0;
		g_Point2[client][i] = 0.0;
	}
}

public Action Command_TeleportZone(int client, int args) {
	if(!Client_IsValid(client) || !Client_IsIngame(client))
		return Plugin_Handled;
		
	if (!IsPlayerAlive(client)) {
		ReplyToCommand(client, "%s You cannot teleport to a mapzone when you're dead.", PREFIX);
		return Plugin_Handled;
	}
	
	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	if (!(StrEqual(auth, "76561198220151661"))) {
		g_TimerEnabled[client] = false;
	}
	
	MapZones find;
	
	if (args == 1) {
		char arg1[64];
		bool found;
		
		GetCmdArg(1, arg1, sizeof(arg1));
		// Try and find the zone
		for (int i = 0; i < ZONE_COUNT; i++) {
			if (StrContains(g_ZoneNames[i], arg1, false) == -1) {
				// Param given doesn't match any of the zones
				found = false;
			} else {
				found = true;
				find = view_as<MapZones>(i);
				break;
			}
		}
		if (!found)
			PrintToChat(client, "%s Couldn't find %s as a type of zone.", PREFIX, arg1);
	} else {
		ReplyToCommand(client, "%s Usage: !zonetp <ZoneType>", PREFIX);
		return Plugin_Handled;
	}
	
	Handle menu = CreateMenu(TeleportZone_Handler);
	
	SetMenuTitle(menu, "Zone Teleport");
	
	for (int i = 0; i < MAX_ZONES; i++) {
		if (EmptyZone(g_MapZones[i][find][0]) && EmptyZone(g_MapZones[i][find][1]))
			continue;
			
		g_ZoneTeleport[client] = view_as<int>(find);
		
		float vPos[3];
		Entity_GetAbsOrigin(client, vPos);
		
		float vCenter[3];
		GetCenterPos(g_MapZones[i][find][0], g_MapZones[i][find][1], vCenter);
		
		float difference;
		difference = GetPositionDiff(vPos, vCenter);
		
		char text[64];
		
		Format(text, sizeof(text), "%s (%.0f)", g_ZoneNames[find], difference);
		
		char info[8];
		
		Format(info, sizeof(info), "%i", i);
		
		AddMenuItem(menu, info, text);
	}
	
	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);
	
	return Plugin_Handled;
}

public int TeleportZone_Handler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		char info[8];
		GetMenuItem(menu, param2, info, 8);
		
		int count, type;
		
		count = StringToInt(info);
		type = g_ZoneTeleport[param1];
		
		float vPosCenter[3];
		
		GetCenterPos(g_MapZones[count][type][0], g_MapZones[count][type][1], vPosCenter);

		TeleportEntity(param1, vPosCenter, NULL_VECTOR, NULL_VECTOR);
	}
	else if(action == MenuAction_End)
		CloseHandle(menu);
}

public Action Command_Zones(int client, int args) {
	
	if(!Client_IsValid(client) || !Client_IsIngame(client))
		return Plugin_Handled;

	if(!IsPlayerAlive(client))
	{
		ReplyToCommand(client, "%s You can't setup mapzones when you're dead.", PREFIX);

		return Plugin_Handled;
	}
	
	if (args >= 1) {
		char arg1[64];
		bool found;
		
		GetCmdArg(1, arg1, sizeof(arg1));
		// Try and find the zone
		for (int i = 0; i < ZONE_COUNT; i++) {
			if (StrContains(g_ZoneNames[i], arg1, false) == -1) {
				// Param given doesn't match any of the zones
				found = false;
			} else {
				g_Type[client] = view_as<MapZones>(i);

				ShowPanel(client, 1);
				return Plugin_Handled;
			}
		}
		if (!found)
			PrintToChat(client, "%s Couldn't find %s as a zone.", PREFIX, arg1);
	}
	
	Reset(client);

	Handle menu = CreateMenu(Select_Type_MenuHandler);
	SetMenuTitle(menu, "Select a zone type:");

	char temp[2];
	for (int i = 0; i < ZONE_COUNT; i++) {
		
		Format(temp, sizeof(temp), "%i", i);
		AddMenuItem(menu, temp, g_ZoneNames[i]);
	}

	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);

	return Plugin_Handled;
}

public int Select_Type_MenuHandler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		char info[8];
		GetMenuItem(menu, param2, info, 8);

		g_Type[param1] = view_as<MapZones>(StringToInt(info));

		ShowPanel(param1, 1);
	} else if(action == MenuAction_End) 
		CloseHandle(menu);
}

public void ShowPanel(int client, int step) {
	g_MapStep[client] = step;
	
	Handle hPanel = CreatePanel();

	char sPanelText[128];
	FormatEx(sPanelText, 128, "Press USE (default \"E\") to set the %s corner in your current position.", step == 1? "FIRST":"SECOND");

	DrawPanelItem(hPanel, sPanelText, ITEMDRAW_RAWLINE);
	DrawPanelItem(hPanel, "Abort zone creation");

	SendPanelToClient(hPanel, client, ZoneCreation_Handler, 540);
	CloseHandle(hPanel);
}

public Action Timer_DrawEverything(Handle Timer, any data) {
	
	for (int i = 0; i < MAX_ZONES; i++) {
		for (int x = 0; x < ZONE_COUNT; x++) {
			float vPoints[8][3];
			
			if (EmptyZone(g_MapZones[i][x][0]) && EmptyZone(g_MapZones[i][x][1]))
				continue;
			
			vPoints[0] = g_MapZones[i][x][0];
			vPoints[7] = g_MapZones[i][x][1];
			CreateZonePoints(vPoints);
		
			DrawZone(0, vPoints, g_BeamSprite, 0, g_ZoneColours[x], 0.10);
		}
	}
}

public Action Timer_Draw(Handle Timer, any data) {
	if(!IsValidClient(data, true) || g_MapStep[data] == 0) {
		Reset(data);
		return Plugin_Stop;
	}
	
	float vOrigin[3];
	
	if(g_MapStep[data] == 1 || g_Point2[data][0] == 0.0) {
		GetClientAbsOrigin(data, vOrigin);
		
		vOrigin[2] += 144;
	} else if (g_MapStep[data] == 4) {
		GetClientAbsOrigin(data, vOrigin);
		vOrigin[2] += 144;
	} else 
		vOrigin = g_Point2[data];
	
	float vPoints[8][3];
	vPoints[0] = g_Point1[data];
	vPoints[7] = vOrigin;
	
	CreateZonePoints(vPoints);
	
	//DrawZone(0, vPoints, gI_BeamSprite, 0, gI_Colors[gMZ_Type[data]], 0.1);
	DrawZone(0, vPoints, g_BeamSprite, 0, {255, 255, 255, 255}, 0.1);
	
	return Plugin_Continue;
}

/**
 *	Inserts a zone
 *
 *  @param client                Player's index
 *	@noreturn
 */
public void InsertZone(int client) {
	char sQuery[256];
	
	MapZones type = g_Type[client];
	
	FormatEx(sQuery, 256, "INSERT INTO mapzones (map, type, corner1_x, corner1_y, corner1_z, corner2_x, corner2_y, corner2_z) VALUES ('%s', '%d', '%.03f', '%.03f', '%.03f', '%.03f', '%.03f', '%.03f');", g_Map, type, g_Point1[client][0], g_Point1[client][1], g_Point1[client][2], g_Point2[client][0], g_Point2[client][1], g_Point2[client][2]);
	
	for (int i = 0; i < MAX_ZONES; i++) {
		if (!EmptyZone(g_MapZones[i][type][0]) && !EmptyZone(g_MapZones[i][type][1]))
			continue;
		
		g_MapZones[i][type][0] = g_Point1[client];
		g_MapZones[i][type][1] = g_Point2[client];
		
		break;
	}
	
	SQL_TQuery(g_SQLhandle, SQL_InsertZone_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_InsertZone_Callback");
	#endif
	Reset(client);
}

public int CreateZoneConfirm_Handler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		char info[8];
		GetMenuItem(menu, param2, info, 8);

		if(StrEqual(info, "yes")) {
			InsertZone(param1);
			g_MapStep[param1] = 0;
		} else if (StrEqual(info, "adjust")) {
			ShowPanel(param1, 4);
		} else {
			Command_Zones(param1, 0);
			g_MapStep[param1] = 0;
		}
	} else if(action == MenuAction_End) {
		g_MapStep[param1] = 0;
		CloseHandle(menu);
	}
}

public Action Command_DeleteZone(int client, int args) {
	
	if(!IsValidClient(client))
		return Plugin_Handled;
	
	bool found;
	
	// check if client is inside a zone
	for (int i = 0; i < MAX_ZONES; i++) {
		for (int x = 0; x < ZONE_COUNT; x++) {
			if (InsideZone(client, g_MapZones[i][x][0], g_MapZones[i][x][1])) {
				found = true;
				char sQuery[256];
				FormatEx(sQuery, 256, "DELETE FROM mapzones WHERE map = '%s' AND id = '%d';", g_Map, g_MapZoneSQLInt[i][x]);
				
				SQL_TQuery(g_SQLhandle, SQL_DeleteZonePosition_Callback, sQuery, g_MapZoneSQLInt[i][x]);
				#if defined SQL_DEBUG
					LogMessage("SQL_DeleteZonePosition_Callback");
				#endif
				g_MapZoneSQLInt[i][x] = 0;
				break;
			}
		}
	}
	
	if (found)
		ReplyToCommand(client, "%s Deleting zone you are in.", PREFIX);
	else 
		ReplyToCommand(client, "%s You're not in a zone!", PREFIX);

	return Plugin_Handled;
}

public Action Command_DeleteZoneType(int client, int args) {
	
	if(!Client_IsValid(client) || !Client_IsIngame(client))
		return Plugin_Handled;
	
	if (args >= 1) {
		char arg1[64];
		bool found;
		MapZones type;
		
		GetCmdArg(1, arg1, sizeof(arg1));
		// Try and find the zone
		for (int i = 0; i < ZONE_COUNT; i++) {
			if (StrContains(g_ZoneNames[i], arg1, false) == -1) {
				// Param given doesn't match any of the zones
				found = false;
			} else {
				type = view_as<MapZones>(i);
				
				// Show Menu
				Handle menu = CreateMenu(DeleteZoneType_Handler);
	
				SetMenuTitle(menu, "Zone deletion");
				
				for (int x = 0; x < MAX_ZONES; x++) {
					if (EmptyZone(g_MapZones[x][type][0]) && EmptyZone(g_MapZones[x][type][1]))
						continue;
					
					g_ZoneTeleport[client] = view_as<int>(type);

					char info[8];
					Format(info, sizeof(info), "%i", x);
					
					float vPos[3];
					Entity_GetAbsOrigin(client, vPos);
					
					float vCenter[3];
					GetCenterPos(g_MapZones[x][type][0], g_MapZones[x][type][1], vCenter);
					
					float difference;
					difference = GetPositionDiff(vPos, vCenter);
					
					char text[64];
					
					Format(text, sizeof(text), "%s (%.0f)", g_ZoneNames[type], difference);
					
					AddMenuItem(menu, info, text);
				}
				
				SetMenuExitButton(menu, true);

				DisplayMenu(menu, client, 20);
				
				return Plugin_Handled;
			}
		}
		if (!found)
			PrintToChat(client, "%s Couldn't find %s as a zone.", PREFIX, arg1);
	} else {
		ReplyToCommand(client, "Usage: !deletetype <ZoneType>");
	}
	
	Handle menu = CreateMenu(DeleteZoneDisplay_Handler);
	
	SetMenuTitle(menu, "Zone Deletion");
	
	for (int i = 0; i < ZONE_COUNT; i++) {
		
		char info[8];
		char text[64];
		
		Format(info, sizeof(info), "%i", i);
		
		Format(text, sizeof(text), "%s", g_ZoneNames[i]);
		
		AddMenuItem(menu, info, text);
	}
	
	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);
	
	return Plugin_Handled;
	
}


public int DeleteZoneType_Handler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		char info[8];
		GetMenuItem(menu, param2, info, 8);
		
		int count, type;
		
		count = StringToInt(info);
		type = g_ZoneTeleport[param1];
		
		//g_MapZoneSQLInt[count][type]
		
		char sQuery[512];
		Format(sQuery, sizeof(sQuery), "DELETE FROM mapzones WHERE map = '%s' AND id = '%i'", g_Map, g_MapZoneSQLInt[count][type]);
				
		SQL_TQuery(g_SQLhandle, SQL_DeleteZoneType_Callback, sQuery, g_MapZoneSQLInt[count][type]);
		#if defined SQL_DEBUG 
			LogMessage("SQL_DeleteZoneType_Callback");
		#endif
		
		g_MapZoneSQLInt[count][type] = 0;
		
		//float vPosCenter[3];
		
		//GetCenterPos(g_MapZones[count][type][0], g_MapZones[count][type][1], vPosCenter);

		//TeleportEntity(param1, vPosCenter, NULL_VECTOR, NULL_VECTOR);
	} else if(action == MenuAction_End)
		CloseHandle(menu);
}

public int DeleteZoneDisplay_Handler(Handle menu, MenuAction action, int param1, int param2) {
	
	Handle sMenu = CreateMenu(DeleteZoneType_Handler);
	SetMenuTitle(sMenu, "Zone deletion");
	
	for (int x = 0; x < MAX_ZONES; x++) {
		
		char data[8];
		GetMenuItem(menu, param2, data, 8);
		
		int type;
		type = StringToInt(data);
		
		if (EmptyZone(g_MapZones[x][type][0]) && EmptyZone(g_MapZones[x][type][1]))
			continue;
		
		g_ZoneTeleport[param1] = type;

		char info[8];
		Format(info, sizeof(info), "%i", x);
		
		float vPos[3];
		Entity_GetAbsOrigin(param1, vPos);
		
		float vCenter[3];
		GetCenterPos(g_MapZones[x][type][0], g_MapZones[x][type][1], vCenter);
		
		float difference;
		difference = GetPositionDiff(vPos, vCenter);
		
		char text[64];
		
		Format(text, sizeof(text), "%s (%.0f)", g_ZoneNames[type], difference);
		
		AddMenuItem(sMenu, info, text);
	}
	
	SetMenuExitButton(sMenu, true);

	DisplayMenu(sMenu, param1, 20);
}

/*public void Bullet_Impact(Event event, const char[] name, bool dontBroadcast) {
	int client = GetClientOfUserId(event.GetInt("userid"));
	
	char nameStr[64];
	
	GetClientName(client, nameStr, 64);
	
	if (StrEqual(nameStr, "steve", false)) {
		float x = event.GetFloat("x");
		float y = event.GetFloat("y");
		float z = event.GetFloat("z");
		
		float vPos[3], vPos2[3], vPos3[3];
		
		vPos[0] = x;
		vPos[1] = y;
		vPos[2] = z;
		
		PushArrayArray(g_BulletPositions, vPos, 3);
		
		vPos3 = vPos;
		vPos3[2] += 100.0;
		
		PushArrayArray(g_BulletPositions2, vPos3, 3);
		
		vPos2 = vPos;
		
		vPos[0] += 10.0;
		vPos[1] += 10.0;
		vPos[2] += 10.0;
		
		vPos2[0] -= 10.0;
		vPos2[1] -= 10.0;
		vPos2[2] -= 10.0;
		
		float vPoints[8][3];
		vPoints[0] = vPos;
		vPoints[7] = vPos2;
		
		CreateZonePoints(vPoints);
		
		//DrawZone(0, vPoints, gI_BeamSprite, 0, gI_Colors[gMZ_Type[data]], 0.1);
		DrawZone(0, vPoints, g_BeamSprite, 0, {255, 255, 255, 255}, 10.0);
	}
	
}

public Action UpdatePoints_Timer(Handle Timer) {
	
	int arraySize = GetArraySize(g_BulletPositions) - 1;
	if (arraySize <= 1)
		return;
	for (int i = 0; i <= arraySize; i++) {
		if (i == arraySize) {
		} else {
			
			float vPos[3], vPos2[3];
			GetArrayArray(g_BulletPositions, i, vPos);
			GetArrayArray(g_BulletPositions,(i+1), vPos2);
			
			TE_SetupBeamPoints(vPos, vPos2, g_BeamSprite, 0, 0, 0, 0.1, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, {255,255,255,255}, 0);
			TE_SendToAll(0.0);
		}
	}
	
	for (int i = 0; i <= arraySize; i++) {
		if (i == arraySize) {
		} else {
			
			float vPos[3], vPos2[3];
			GetArrayArray(g_BulletPositions2, i, vPos);
			GetArrayArray(g_BulletPositions2,(i+1), vPos2);
			
			TE_SetupBeamPoints(vPos, vPos2, g_BeamSprite, 0, 0, 0, 0.1, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, {255,255,255,255}, 0);
			TE_SendToAll(0.0);
		}
	}
	
	for (int i = 0; i <= arraySize; i++) {
		
		float vPos[3], vPos2[3];
		GetArrayArray(g_BulletPositions, i, vPos);
		GetArrayArray(g_BulletPositions2,i, vPos2);
		
		TE_SetupBeamPoints(vPos, vPos2, g_BeamSprite, 0, 0, 0, 0.1, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, {255,255,255,255}, 0);
		TE_SendToAll(0.0);
	
	}
	
	float vPos[3], vPos2[3];
	
	GetArrayArray(g_BulletPositions, 0, vPos);
	GetArrayArray(g_BulletPositions, arraySize, vPos2);
	
	TE_SetupBeamPoints(vPos, vPos2, g_BeamSprite, 0, 0, 0, 0.1, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, {255,255,255,255}, 0);
	TE_SendToAll(0.0);
	
	GetArrayArray(g_BulletPositions2, 0, vPos);
	GetArrayArray(g_BulletPositions2, arraySize, vPos2);
	
	TE_SetupBeamPoints(vPos, vPos2, g_BeamSprite, 0, 0, 0, 0.1, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, {255,255,255,255}, 0);
	TE_SendToAll(0.0);

}*/