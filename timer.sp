#include <sourcemod>
#include <geoip>
#include <sdktools>
#include <sdkhooks>
#include <smlib>

#include "vars"
#include "events"
#include "timers"
#include "core"
#include "zones"
#include "timer"
#include "hud"
#include "wr"
#include "ranks"
#include "profile"
#include "misc"

#pragma semicolon 1
#pragma dynamic 131072
#pragma newdecls required

public Plugin myinfo = {
	name = "Timer",
	author = "Jumes",
	description = "Bhop Timer",
	version = "0.0.5",
	url = "http://www.google.com/Narina"
}

public void OnPluginStart() {
	
	LoadRanks();
	
	SQL_DBConnect();
	GetCurrentMap(g_Map, 128);
	
	for (int i = 0; i < MaxClients; i++) {
		if (IsValidClient(i)) {
			OnPlayerConnect(i, false);
			CreatePlayer(i);
			g_ExtraLife[i] = true;
		}
	}
	
	//g_BulletPositions = CreateArray(3);
	//g_BulletPositions2 = CreateArray(3);
	
	HookEvent("player_jump", Player_Jump);
	HookEvent("player_death", Player_Death);
	HookEvent("player_team", Player_Death);
	HookEvent("player_spawn", Player_Spawned);
	//HookEvent("bullet_impact", Bullet_Impact);
	HookEvent("player_spawned", Player_Spawned);
	HookEvent("player_team", Player_Team_Change, EventHookMode_Pre);
	HookEvent("player_connect", Player_Connect, EventHookMode_Pre);
	HookEvent("player_say", Player_Say, EventHookMode_Pre);
	HookEvent("player_disconnect", Player_Disconnect, EventHookMode_Pre);
	
	RegConsoleCmd("say", Command_SayChat);
	RegConsoleCmd("say_team", Command_SayChat);
	
	CreateTimer(0.1, UpdateHUD_Timer, INVALID_HANDLE, TIMER_REPEAT);
	CreateTimer(1.0, JumpCheck_Timer, INVALID_HANDLE, TIMER_REPEAT);
	//CreateTimer(1.0, MessageCheck_Timer, INVALID_HANDLE, TIMER_REPEAT);
	CreateTimer(10.0, UpdateMaxRank_Timer, INVALID_HANDLE, TIMER_REPEAT);
	CreateTimer(0.2, UpdateRealTime_Timer, INVALID_HANDLE, TIMER_REPEAT);
	//CreateTimer(0.1, UpdatePoints_Timer, INVALID_HANDLE, TIMER_REPEAT);
	
	/*************************************************************************************************************************************************************************
																					Timer
	*************************************************************************************************************************************************************************/

	RegConsoleCmd("sm_s", Command_StartTimer, "Start your timer.");
	RegConsoleCmd("sm_start", Command_StartTimer, "Start your timer.");
	RegConsoleCmd("sm_r", Command_StartTimer, "Start your timer.");
	RegConsoleCmd("sm_restart", Command_StartTimer, "Start your timer.");

	RegConsoleCmd("sm_stop", Command_StopTimer, "Stop your timer.");

	//RegConsoleCmd("sm_pause", Command_TogglePause, "Toggle pause.");
	//RegConsoleCmd("sm_unpause", Command_TogglePause, "Toggle pause.");
	//RegConsoleCmd("sm_resume", Command_TogglePause, "Toggle pause");
	
	/*************************************************************************************************************************************************************************
																					World Record
	*************************************************************************************************************************************************************************/
	
	RegConsoleCmd("sm_wr", Command_WR);
	RegConsoleCmd("sm_worldrecord", Command_WR);

	// WRSW command
	RegConsoleCmd("sm_wrsw", Command_WRSW);
	RegConsoleCmd("sm_worldrecordsw", Command_WRSW);

	// delete records
	RegAdminCmd("sm_delete", Command_Delete, ADMFLAG_RCON, "Opens a record deletion menu interface");
	RegAdminCmd("sm_deleterecord", Command_Delete, ADMFLAG_RCON, "Opens a record deletion menu interface");
	RegAdminCmd("sm_deleterecords", Command_Delete, ADMFLAG_RCON, "Opens a record deletion menu interface");
	RegAdminCmd("sm_deleteall", Command_DeleteAll, ADMFLAG_RCON, "Deletes all the records");
	
	GetCurrentRecords();
	
	/*************************************************************************************************************************************************************************
																					Styles
	*************************************************************************************************************************************************************************/
	
	RegConsoleCmd("sm_auto", Command_Auto, "Style shortcut: Auto");
	/*RegConsoleCmd("sm_sw", Command_Sideways, "Style shortcut: Sideways");*/
	//RegConsoleCmd("sm_legit", Command_Legit, "Style shortcut: Legit");
	/*RegConsoleCmd("sm_w", Command_WOnly, "Style shortcut: W-Only");
	RegConsoleCmd("sm_d", Command_DOnly, "Style shortcut: D-Only");
	RegConsoleCmd("sm_s", Command_SOnly, "Style shortcut: S-Only");
	RegConsoleCmd("sm_a", Command_AOnly, "Style shortcut: A-Only");
	RegConsoleCmd("sm_easy", Command_Easy, "Style shortcut: Easy");
	RegConsoleCmd("sm_hsw", Command_HalfSideWays, "Style shortcut: Half Side Ways");*/
	
	/*************************************************************************************************************************************************************************
																					Auto Bhop
	*************************************************************************************************************************************************************************/
	
	//RegConsoleCmd("sm_auto", Command_AutoBhop, "Toggle autobhop.");
	//RegConsoleCmd("sm_autobhop", Command_AutoBhop, "Toggle autobhop.");
	//RegConsoleCmd("sm_colours", Command_Colours, "Displays all colours");
	
	/*************************************************************************************************************************************************************************
																					Ranks
	*************************************************************************************************************************************************************************/
	
	RegAdminCmd("sm_givepoints", Command_GivePoints, ADMFLAG_RCON, "Gives a player points");
	RegConsoleCmd("sm_rank", Command_ShowPoints, "Display points");
	RegConsoleCmd("sm_chatranks", Command_ShowRank, "Display rank");
	
	/*************************************************************************************************************************************************************************
																					Zones
	*************************************************************************************************************************************************************************/
	
	//RegAdminCmd("sm_modifier", Command_Modifier, ADMFLAG_RCON, "Changes the axis modifier for the zone editor. Usage: sm_modifier <number>");
	
	// menu
	RegAdminCmd("sm_zone", Command_Zones, ADMFLAG_RCON, "Opens the mapzones menu");
	RegAdminCmd("sm_mapzones", Command_Zones, ADMFLAG_RCON, "Opens the mapzones menu");
	
	RegAdminCmd("sm_zonetp", Command_TeleportZone, ADMFLAG_RCON, "Teleports you to a selected zone");
	RegAdminCmd("sm_zonedelete", Command_DeleteZone, ADMFLAG_RCON, "Delete a mapzone");
	RegAdminCmd("sm_deletetype", Command_DeleteZoneType, ADMFLAG_RCON, "Find map zones by type");
	//RegAdminCmd("sm_deleteallzones", Command_DeleteAllZones, ADMFLAG_RCON, "Delete all mapzones");
	
	RegConsoleCmd("sm_spec", Command_Spectate, "Spectate a player");
	RegConsoleCmd("sm_spectate", Command_Spectate, "Spectate a player");
	
	//RegAdminCmd("sm_round", Command_Round, ADMFLAG_RCON);
	RegConsoleCmd("sm_top", Command_Top100, "Shows top 100");
	RegAdminCmd("sm_removeweapon", Command_Skip, ADMFLAG_RCON);
	RegAdminCmd("sm_addrecord", Command_AddRecord, ADMFLAG_RCON);
	RegConsoleCmd("sm_profile", Command_Profile, "Shows your current stats");
	
	RegConsoleCmd("sm_hide", Command_HidePlayers, "Hides everyone from you.");
	
	// draw
	// start drawing timer here
	CreateTimer(0.10, Timer_DrawEverything, INVALID_HANDLE, TIMER_REPEAT);
	
	/*************************************************************************************************************************************************************************
																					ConVars
	*************************************************************************************************************************************************************************/
	
	RegConsoleCmd("sm_knife", Command_Knife, "Get your knife back!");
	RegConsoleCmd("sm_hud", Command_HideHud, "Disable/Enable your hud!");
	
	//gCV_ZoneStyle = CreateConVar("shavit_zones_style", "0", "Style for mapzone drawing.\n0 - 3D box\n1 - 2D box");
	//HookConVarChange(gCV_ZoneStyle, OnConVarChanged);
	
	AutoExecConfig();
	//gB_ZoneStyle = GetConVarBool(gCV_ZoneStyle);
}

public Action Command_HidePlayers(int client, int args) {
	
	if ( !(IsValidClient(client, true)) ) {
		PrintToChat(client, " [\x03SourceRuns\x01] - You cannot run this command while dead.");
		return Plugin_Handled;
	}
	
	if (g_Hide[client]) {
		g_Hide[client] = false;
		PrintToChat(client, " [\x03SourceRuns\x01] - Players are now [\x09VISIBLE\x01]");
	} else {
		g_Hide[client] = true;
		PrintToChat(client, " [\x03SourceRuns\x01] - Players are now [\x09HIDDEN\x01]");
	}
	
	return Plugin_Handled;
	
}

/*************************************************************************************************************************************************************************
																					OnPlayerRunCmd
*************************************************************************************************************************************************************************/

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon, int &subtype, int &cmdnum, int &tickcount, int &seed, int mouse[2]) {

	if(!Client_IsValid(client) && !Client_IsIngame(client) && !IsPlayerAlive(client)) {
		return Plugin_Continue;
	}
	
	g_CurrentButtons[client] = buttons;
	
	// Get Last zone
	
	for (int i = 0; i < ZONE_COUNT; i++) {
		if (InsideZoneByType(client, view_as<MapZones>(i))) {
			g_LastZone[client] = view_as<MapZones>(i);
			break;
		}
	}
	
	if (GetEntityMoveType(client) == MOVETYPE_NOCLIP) {
		StopTimer(client);
	}
	
	if (InsideZoneByType(client, Zone_Slay)) {
		ForcePlayerSuicide(client);
	}
	
	if (InsideZoneByType(client, Zone_Start)) {
		float maxSpeed = 600.0;
		
		float gen = Math_GetRandomFloat(-20.0, 20.0);
		
		if (gen >= 0) {
			maxSpeed += gen;
		} else {
			maxSpeed -= gen;
		}
		
		CheckVelocity(client, 1, maxSpeed);
	}
	
	if ( InsideZoneByType(client, Zone_Start) && g_ExtraLife[client] && (GetClientHealth(client) < 100) ) {
		SetEntityHealth(client, 100);
		g_ExtraLife[client] = false;
	}
	
	/*************************************************************************************************************************************************************************
																					Stats
	*************************************************************************************************************************************************************************/
	
	
	
	/*************************************************************************************************************************************************************************
																					Zones
	*************************************************************************************************************************************************************************/
	
	if(buttons & IN_USE) {
		if(!g_Button[client] && g_MapStep[client] > 0 && g_MapStep[client] != 3)
		{
			float vOrigin[3];
			GetClientAbsOrigin(client, vOrigin);
			
			if(g_MapStep[client] == 1)
			{
				g_Point1[client] = vOrigin;
				
				CreateTimer(0.1, Timer_Draw, client, TIMER_REPEAT);
				
				ShowPanel(client, 2);
			} else if (g_MapStep[client] == 4) {
				
				vOrigin[2] += 144;
				g_Point2[client] = vOrigin;
				
				CreateTimer(0.1, Timer_Draw, client, TIMER_REPEAT);
				//ShowPanel(client, 2);
				g_MapStep[client] = 2;
				
				Handle menu = CreateMenu(CreateZoneConfirm_Handler);
				SetMenuTitle(menu, "Confirm?");
			
				AddMenuItem(menu, "yes", "Yes");
				AddMenuItem(menu, "no", "No");
				AddMenuItem(menu, "adjust", "Revert");
			
				SetMenuExitButton(menu, true);
			
				DisplayMenu(menu, client, 20);
				
			} else if(g_MapStep[client] == 2) {
				//vOrigin[2] += 72; // was requested to make it higher
				vOrigin[2] += 144;
				g_Point2[client] = vOrigin;
				
				g_MapStep[client]++;
				
				Handle menu = CreateMenu(CreateZoneConfirm_Handler);
				SetMenuTitle(menu, "Confirm?");
			
				AddMenuItem(menu, "yes", "Yes");
				AddMenuItem(menu, "no", "No");
				AddMenuItem(menu, "adjust", "Revert");
			
				SetMenuExitButton(menu, true);
			
				DisplayMenu(menu, client, 20);
			}
		}
		
		g_Button[client] = true;
	} else {
		g_Button[client] = false;
	}
	
	/*************************************************************************************************************************************************************************
																					World Record
	*************************************************************************************************************************************************************************/
	
	if (InsideZoneByType(client, Zone_Start)) {
		ResumeTimer(client);
		StartTimer(client);
	}
	
	if(g_TimerEnabled[client])
	{
		
		if(InsideZoneByType(client, Zone_End))
		{
			if (GetEntityMoveType(client) == MOVETYPE_NOCLIP) {
			} else {
				Timer_FinishMap(client);
			}
		}
	}
	
	/*************************************************************************************************************************************************************************
																					Styles
	*************************************************************************************************************************************************************************/
	
	bool bEdit = false;
	
	if (InsideZoneByType(client, Zone_Hover)) {
		float fVelo[3];
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", fVelo);
		fVelo[2] = -1.0;
		TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, fVelo);
	}
	
	if (InsideZoneByType(client, Zone_Block)) {
		// Stop Timer
		float fVel[3] =  { 0.0, ... };
		TeleportEntity(client, g_lastPos[client], NULL_VECTOR, fVel);
	}

	/*************************************************************************************************************************************************************************
																					Custom
	*************************************************************************************************************************************************************************/
	
	if ( GetEntityFlags(client) & FL_ONGROUND )
		g_Jumping[client] = false;
	else 
		g_Jumping[client] = true;
	
	if (buttons & IN_JUMP) 
		g_Jumping[client] = true;
	
	if(/*gCV_Autobhop.BoolValue &&*/ g_AutoBhop[client] && buttons & IN_JUMP && !(GetEntityFlags(client) & FL_ONGROUND) && !Client_IsOnLadder(client) && GetEntProp(client, Prop_Send, "m_nWaterLevel") <= 1) {
		buttons &= ~IN_JUMP;
	}

	if(g_TimerPaused[client]){
		bEdit = true;
		vel = view_as<float>({0.0, 0.0, 0.0});
	}
	
	float vPos[3];
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", vPos);
	g_lastPos[client] = vPos;
	
	return bEdit? Plugin_Changed:Plugin_Continue;

}

public Action Hook_SetTransmit(int entity, int client) 
{ 
    if (client != entity && (0 < entity <= MaxClients) && g_Hide[client]) 
        return Plugin_Handled; 
     
    return Plugin_Continue; 
}  