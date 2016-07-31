public Action Command_WR(int client, int args) {
	if(!IsValidClient(client))
		return Plugin_Handled;

	Menu menu = CreateMenu(CommandWR_Handler);
	SetMenuTitle(menu, "Select a style");
	
	float time;
	char text[64];
	
	Timer_GetWRTime(Style_Auto, time);
	if (time == 0.0)
		Format(text, sizeof(text), "Auto - WR: N/A");
	else {
		FormatSeconds(time, text, sizeof(text));
		Format(text, sizeof(text), "Auto - WR: %s", text);
	}
	AddMenuItem(menu, "0", text);
	
	Timer_GetWRTime(Style_Legit, time);
	if (time == 0.0) 
		Format(text, sizeof(text), "Legit - WR: N/A");
	else {
		FormatSeconds(time, text, sizeof(text));
		Format(text, sizeof(text), "Legit - WR: %s", text);
	}
	AddMenuItem(menu, "1", text);
	
	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);

	return Plugin_Handled;
}

public int CommandWR_Handler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		char info[16];
		GetMenuItem(menu, param2, info, 16);
		Command_ProcessWR(param1, StringToInt(info));
	} else if(action == MenuAction_End)
		CloseHandle(menu);
}

public void Command_ProcessWR(int client, int style) {	
	char sQuery[256];
	FormatEx(sQuery, 256, "SELECT p.id, u.name, p.time, p.jumps FROM playertimes p JOIN users u ON p.auth = u.auth WHERE map = '%s' AND style = '%i' ORDER BY time ASC LIMIT 50;", g_Map, style);

	SQL_TQuery(g_SQLhandle, SQL_WR_Callback, sQuery, GetClientSerial(client), DBPrio_High);
	
	#if defined SQL_DEBUG
		LogMessage("SQL_WR_Callback");
	#endif

	gBS_LastWR[client] = Style_Auto;
}

public void SQL_WR_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	if(handle == null)
	{
		LogError("Timer (WR SELECT) SQL query failed. Reason: %s", error);

		return;
	}

	int client = GetClientFromSerial(data);

	if(!client)
	{
		return;
	}

	Menu menu = CreateMenu(WRMenu_Handler);
	SetMenuTitle(menu, "Records for %s:", g_Map);

	int iCount = 0;

	while(SQL_FetchRow(handle))
	{
		iCount++;

		// 0 - record id, for statistic purposes.
		int id = SQL_FetchInt(handle, 0);
		char sID[8];
		IntToString(id, sID, 8);

		// 1 - player name
		char sName[MAX_NAME_LENGTH];
		SQL_FetchString(handle, 1, sName, MAX_NAME_LENGTH);

		// 2 - time
		float fTime = SQL_FetchFloat(handle, 2);
		char sTime[16];
		//FormatSeconds(fTime, sTime, 16);
		
		FormatSeconds(fTime, sTime, sizeof(sTime));

		// 3 - jumps
		//int iJumps = SQL_FetchInt(handle, 3);

		char sDisplay[128];
		//FormatEx(sDisplay, 128, "#%d - %s - %s (%d Jumps)", iCount, sName, sTime, iJumps);
		FormatEx(sDisplay, 128, "%s by %s", sTime, sName);
		AddMenuItem(menu, sID, sDisplay);
	}

	if(!GetMenuItemCount(menu))
	{
		AddMenuItem(menu, "-1", "No records found.");
	}

	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);
}

public int WRMenu_Handler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		char info[16];
		GetMenuItem(menu, param2, info, 16);
		int id = StringToInt(info);

		OpenSubMenu(param1, id);
	} else if(action == MenuAction_End) {
		CloseHandle(menu);
	}
}

public void OpenSubMenu(int client, int id)
{
	char sQuery[512];
	FormatEx(sQuery, 512, "SELECT u.name, p.time, p.jumps, p.style, u.auth, p.date FROM playertimes p JOIN users u ON p.auth = u.auth WHERE p.id = '%d' LIMIT 1;", id);

	SQL_TQuery(g_SQLhandle, SQL_SubMenu_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG
		LogMessage("SQL_SubMenu_Callback");
	#endif
}

public void SQL_SubMenu_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	if(handle == null)
	{
		LogError("Timer (WR SUBMENU) SQL query failed. Reason: %s", error);

		return;
	}

	int client = GetClientFromSerial(data);

	if(!client)
	{
		return;
	}

	Handle menu = CreateMenu(SubMenu_Handler);

	char sName[MAX_NAME_LENGTH];
	char sAuthID[64];

	int iCount = 0;

	while(SQL_FetchRow(handle))
	{
		iCount++;

		// 0 - name
		SQL_FetchString(handle, 0, sName, MAX_NAME_LENGTH);

		// 1 - time
		float fTime = SQL_FetchFloat(handle, 1);
		char sTime[16];
		FormatSeconds(fTime, sTime, 16);

		char sDisplay[128];
		FormatEx(sDisplay, 128, "Time: %s", sTime);
		AddMenuItem(menu, "-1", sDisplay);

		// 2 - jumps
		int iJumps = SQL_FetchInt(handle, 2);
		FormatEx(sDisplay, 128, "Jumps: %d", iJumps);
		AddMenuItem(menu, "-1", sDisplay);

		// 3 - style
		int iStyle = SQL_FetchInt(handle, 3);
		char sStyle[16];
		FormatEx(sStyle, 16, "%s", iStyle == view_as<int>(Style_Auto)? "Auto":"Legit");
		FormatEx(sDisplay, 128, "Style: %s", sStyle);
		AddMenuItem(menu, "-1", sDisplay);

		// 4 - steamid3
		SQL_FetchString(handle, 4, sAuthID, 64);

		// 5 - date
		char sDate[32];
		SQL_FetchString(handle, 5, sDate, 32);
		FormatEx(sDisplay, 128, "Date: %s", sDate);
		AddMenuItem(menu, "-1", sDisplay);
	}

	SetMenuTitle(menu, "%s %s\n--- %s:", sName, sAuthID, g_Map);

	SetMenuExitBackButton(menu, true);

	DisplayMenu(menu, client, 20);
}

public int SubMenu_Handler(Handle menu, MenuAction action, int param1, int param2)
{
	if((action == MenuAction_Cancel && (param2 == MenuCancel_ExitBack && param2 != MenuCancel_Exit)) || action == MenuAction_Select)
		OpenWR(param1);
	else if(action == MenuAction_End)
		CloseHandle(menu);
}

public void OpenWR(int client)
{
	if(!IsValidClient(client))
		return;

	if(gBS_LastWR[client] == Style_Auto)
		Command_WR(client, 0);
	else
		Command_WRSW(client, 0);
}

public Action Command_WRSW(int client, int args)
{
	if(!IsValidClient(client))
	{
		return Plugin_Handled;
	}

	char sQuery[256];
	FormatEx(sQuery, 256, "SELECT p.id, u.name, p.time, p.jumps FROM playertimes p JOIN users u ON p.auth = u.auth WHERE map = '%s' AND style = '1' ORDER BY time ASC LIMIT 100;", g_Map);

	SQL_TQuery(g_SQLhandle, SQL_WR_Callback, sQuery, GetClientSerial(client), DBPrio_High);
	#if defined SQL_DEBUG
		LogMessage("SQL_WR_Callback");
	#endif

	gBS_LastWR[client] = Style_Legit;

	return Plugin_Handled;
}

public int Timer_FinishMap(int client) {

	Timer_OnFinish(client, g_CurrentStyle[client], CalculateTime(client), g_Jumps[client]);

	StopTimer(client);
}

public Action Command_Delete(int client, int args)
{
	if(!IsValidClient(client))
		return Plugin_Handled;

	Handle menu = CreateMenu(MenuHandler_Delete);
	SetMenuTitle(menu, "Delete a record from:");

	AddMenuItem(menu, "auto", "Auto");
	AddMenuItem(menu, "legit", "Legit");

	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);

	return Plugin_Handled;
}

public int MenuHandler_Delete(Handle menu, MenuAction action, int param1, int param2)
{
	if(action == MenuAction_Select)
	{
		char info[16];
		GetMenuItem(menu, param2, info, 16);

		if(StrEqual(info, "auto"))
		{
			OpenDelete(param1, Style_Auto);
		}

		else if(StrEqual(info, "legit"))
		{
			OpenDelete(param1, Style_Legit);
		}
	}

	else if(action == MenuAction_End)
	{
		CloseHandle(menu);
	}
}

public Action Command_DeleteAll(int client, int args)
{
	if(!IsValidClient(client))
		return Plugin_Handled;

	Handle menu = CreateMenu(MenuHandler_DeleteAll);
	SetMenuTitle(menu, "Delete ALL the records for \"%s\"?", g_Map);
	
	AddMenuItem(menu, "-1", "No");
	AddMenuItem(menu, "yes", "Yes");

	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);

	return Plugin_Handled;
}

public int MenuHandler_DeleteAll(Handle menu, MenuAction action, int param1, int param2)
{
	if(action == MenuAction_Select)
	{
		char info[16];
		GetMenuItem(menu, param2, info, 16);

		if(StringToInt(info) == -1)
		{
			PrintToChat(param1, "%s Aborted deletion.", PREFIX);

			return;
		}
		
		char sQuery[256];
		FormatEx(sQuery, 256, "DELETE FROM playertimes WHERE map = '%s';", g_Map);

		SQL_TQuery(g_SQLhandle, DeleteAll_Callback, sQuery, GetClientSerial(param1), DBPrio_High);
		#if defined SQL_DEBUG
			LogMessage("DeleteAll_Callback");
		#endif
	}

	else if(action == MenuAction_End)
	{
		CloseHandle(menu);
	}
}

public void OpenDelete(int client, BhopStyle style)
{
	char sQuery[512];
	FormatEx(sQuery, 512, "SELECT p.id, u.name, p.time, p.jumps FROM playertimes p JOIN users u ON p.auth = u.auth WHERE map = '%s' AND style = '%d' ORDER BY time ASC LIMIT 1000;", g_Map, style);

	Handle datapack = CreateDataPack();
	WritePackCell(datapack, GetClientSerial(client));
	WritePackCell(datapack, style);

	SQL_TQuery(g_SQLhandle, SQL_OpenDelete_Callback, sQuery, datapack, DBPrio_High);
	#if defined SQL_DEBUG
		LogMessage("SQL_OpenDelete_Callback");
	#endif
}

public void SQL_OpenDelete_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	ResetPack(data);
	int client = GetClientFromSerial(ReadPackCell(data));
	BhopStyle style = ReadPackCell(data);
	CloseHandle(data);

	if(handle == null)
	{
		LogError("Timer (WR OpenDelete) SQL query failed. Reason: %s", error);

		return;
	}

	if(!client)
	{
		return;
	}

	Menu menu = CreateMenu(OpenDelete_Handler);
	menu.SetTitle("Records for %s:\n(%s)", g_Map, style == Style_Auto ? "Auto":"Legit");

	int iCount = 0;

	while(SQL_FetchRow(handle))
	{
		iCount++;

		// 0 - record id, for statistic purposes.
		int id = SQL_FetchInt(handle, 0);
		char sID[8];
		IntToString(id, sID, 8);

		// 1 - player name
		char sName[MAX_NAME_LENGTH];
		SQL_FetchString(handle, 1, sName, MAX_NAME_LENGTH);

		// 2 - time
		float fTime = SQL_FetchFloat(handle, 2);
		char sTime[16];
		FormatSeconds(fTime, sTime, 16);

		// 3 - jumps
		int iJumps = SQL_FetchInt(handle, 3);

		char sDisplay[128];
		FormatEx(sDisplay, 128, "#%d - %s - %s (%d Jumps)", iCount, sName, sTime, iJumps);
		menu.AddItem(sID, sDisplay);
	}

	if(!iCount)
	{
		AddMenuItem(menu, "-1", "No records found.");
	}

	SetMenuExitButton(menu, true);

	DisplayMenu(menu, client, 20);
}

public int OpenDelete_Handler(Handle menu, MenuAction action, int param1, int param2)
{
	if(action == MenuAction_Select)
	{
		char info[16];
		GetMenuItem(menu, param2, info, 16);
		
		if(StringToInt(info) == -1)
		{
			return;
		}

		Handle hMenu = CreateMenu(DeleteConfirm_Handler);
		SetMenuTitle(hMenu, "Are you sure?");

		AddMenuItem(hMenu, "-1", "No");

		AddMenuItem(hMenu, info, "Yes");

		SetMenuExitButton(hMenu, true);

		DisplayMenu(hMenu, param1, 20);
	}

	else if(action == MenuAction_End)
	{
		CloseHandle(menu);
	}
}

public int DeleteConfirm_Handler(Handle menu, MenuAction action, int param1, int param2)
{
	if(action == MenuAction_Select)
	{
		char info[16];
		GetMenuItem(menu, param2, info, 16);

		if(StringToInt(info) == -1)
		{
			PrintToChat(param1, "%s Aborted deletion.", PREFIX);

			return;
		}
		
		char sQuery[256];
		FormatEx(sQuery, 256, "DELETE FROM playertimes WHERE id = '%s';", info);

		SQL_TQuery(g_SQLhandle, DeleteConfirm_Callback, sQuery, GetClientSerial(param1), DBPrio_High);
		#if defined SQL_DEBUG
			LogMessage("DeleteConfirm_Callback");
		#endif
	}

	else if(action == MenuAction_End)
	{
		CloseHandle(menu);
	}
}

public void DeleteConfirm_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	if(handle == null)
	{
		LogError("Timer (WR DeleteConfirm) SQL query failed. Reason: %s", error);

		return;
	}
	
	UpdateWRCache();
	GetCurrentRecords();

	int client = GetClientFromSerial(data);

	if(!client)
	{
		return;
	}

	PrintToChat(client, "%s Deleted record.", PREFIX);
}

public void DeleteAll_Callback(Handle owner, Handle handle, const char[] error, any data)
{
	if(handle == null)
	{
		LogError("Timer (WR DeleteAll) SQL query failed. Reason: %s", error);

		return;
	}
	
	UpdateWRCache();
	GetCurrentRecords();

	/*for(int i = 1; i <= MaxClients; i++)
	{
		OnClientInServer(i);
	}*/

	int client = GetClientFromSerial(data);

	if(!client)
	{
		return;
	}

	PrintToChat(client, "%s Deleted ALL records for \"%s\".", PREFIX, g_Map);
}

public int Timer_GetWRTime(BhopStyle style, float &time) {
	time = g_WRTime[view_as<int>(style)];
}

public void Timer_GetMaxWR(int style) {
	SQL_GetMaxRecords(style);
}

public void SQL_GetMaxRecords(int style) {
	char sQuery[512];
	FormatEx(sQuery, sizeof(sQuery), "SELECT COUNT(*), style FROM playertimes WHERE map = '%s' AND style = '%i';", g_Map, style);
	SQL_TQuery(g_SQLhandle, SQL_GetMaxRecords_Callback, sQuery);
	#if defined SQL_DEBUG
		LogMessage("SQL_GetMaxRecords_Callback");
	#endif
}

public void SQL_GetMaxRecords_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) {
		LogError("Timer couldn't get GetMaxRecord, Error: %s", error);
		return;
	}
	SQL_FetchRow(handle);
	int style = SQL_FetchInt(handle, 1);
	g_SMaxRecords[style] = SQL_FetchInt(handle, 0);
	//g_MaxRecords = SQL_FetchInt(hndl, 0);
}

/*
	Deletes exess records 


*/
/*public void DeleteMulitpleRecords(int client, int style) {

	// Check if a player has already got record in that style
	// if the already has a record delete it before adding a new one.
	
	char auth[64];
	
	GetClientAuthId(client, AuthId_SteamID64, auth, 64);
	
	char sQuery[512];
	
	Format(sQuery, 512, "SELECT COUNT(*) FROM playertimes WHERE auth = '%s' AND map = '%s' AND style = %i", auth, g_Map, style);
	SQL_TQuery(g_SQLhandle, SQL_DeleteMulti_Callback, sQuery, GetClientSerial(client));

}

public void SQL_DeleteMulti_Callback(Handle owner, Handle handle, const char[] error, any data) {

	if (handle == null) {
		return;
	}
	
	if (SQL_FetchRow(handle)) {
		int count = SQL_FetchInt(handle, 0);
		if (count > 1) {
			
		}
	}

}*/

public void GetPosWR(int client, int style) {
	SQL_GetPosWR(client, style);
}

public void SQL_GetPosWR(int client, int style) {

	char auth[64];
	GetClientAuthId(client, AuthId_SteamID64, auth, sizeof(auth));

	char sQuery[512];
	FormatEx(sQuery, sizeof(sQuery), "SELECT COUNT(*), style FROM `playertimes` WHERE `map` = '%s' AND `time` <= (SELECT `time` FROM `playertimes` WHERE `auth` = '%s' AND `map` = '%s' AND style = '%i' LIMIT 1) AND style = '%i' ORDER BY `time` ASC LIMIT 1", g_Map, auth, g_Map, style, style);

	SQL_TQuery(g_SQLhandle, SQL_GetPosWR_Callback, sQuery, GetClientSerial(client));
	#if defined SQL_DEBUG 
		LogMessage("SQL_GetPosWR_Callback");
	#endif
}

public void SQL_GetPosWR_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) {
		LogError("Timer couldn't get GetPosWR, Error: %s", error);
		return;
	}
	
	SQL_FetchRow(handle);
	
	int client = GetClientFromSerial(data);
	int style = SQL_FetchInt(handle, 1);
	
	g_CurrPosWR[client][style] = SQL_FetchInt(handle, 0);
	
}

public int GetRLPosWR(int client, float time, int style) {
	SQL_GetCurrWRPos(client, time, style);
	return g_RealTimePosWR[client][style];
}

public void SQL_GetCurrWRPos(int client, float time, int style) {
	
	DataPack pack = CreateDataPack();
	
	WritePackCell(pack, GetClientSerial(client));
	WritePackCell(pack, style);
	
	char sQuery[512];
	FormatEx(sQuery, sizeof(sQuery), "SELECT COUNT(*) FROM `playertimes` WHERE `map` = '%s' AND `time` <= '%.3f' AND style = '%i' ORDER BY `time` ASC LIMIT 1", g_Map, time, style);
	SQL_TQuery(g_SQLhandle, SQL_GetCurrWRPos_Callback, sQuery, pack);
	#if defined SQL_DEBUG
		LogMessage("SQL_GetCurrWRPos_Callback");
	#endif
}


public void SQL_GetCurrWRPos_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) {
		LogError("Timer couldn't get GetCurrWRPos, Error: %s", error);
		return;
	}
	SQL_FetchRow(handle);
	
	ResetPack(data);
	
	int client = GetClientFromSerial(ReadPackCell(data));
	int style = ReadPackCell(data);
	
	CloseHandle(data);
	
	g_RealTimePosWR[client][style] = SQL_FetchInt(handle, 0) + 1;
}