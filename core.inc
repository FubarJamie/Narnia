#if defined JumesLoadedCore
	#endinput
#endif
#define JumesLoadedCore

/*************************************************************************************************************************************************************************
																					EVENT HANDLERS
*************************************************************************************************************************************************************************/

/*public void OnMapStart() {
	
	// Zones
	GetCurrentMap(g_Map, 128);
	UnloadZones(0);
	g_BeamSprite = PrecacheModel("materials/sprites/laserbeam.vmt");
	g_HaloSprite = PrecacheModel("materials/sprites/halo01.vmt");
	RefreshZones();
	
}*/

/**
 *	Checks if client is valid
 *
 *  @param client                  Player's index
 *  @param bAlive                  Wether you want to check if the player is alive or not
 *	@return bool                   Wether the client is valid or not
 */
stock bool IsValidClient(int client, bool bAlive = false) {
	return (client >= 1 && client <= MaxClients && IsClientConnected(client) && IsClientInGame(client) && !IsClientSourceTV(client) && (!bAlive || IsPlayerAlive(client)));
}

/**
 *	Formats time
 *
 *  @param time                  Time
 *  @param newtime               String to be copied to
 *	@param newtimesize           Size of string
 *	@param precise               Whether you want the format to be precise
 *	@noreturn
 */
stock void FormatSeconds(float time, char[] newtime, int newtimesize, bool precise = true) {
	
	int iTemp = RoundToFloor(time);

	int iMinutes;
	int iSeconds;
	char cSeconds[8];
	float fSeconds;
	
	if (iTemp >= 60) {
		iMinutes = RoundToFloor(iTemp / 60.0);
		iTemp %= 60;
	} else
		iMinutes = -1;
	
	iSeconds = iTemp;
	iTemp %= 1;

	fSeconds = ((iTemp) + time - RoundToFloor(time));
	
	if (iMinutes >= 1) {
		if (iSeconds < 10) {
			Format(cSeconds, sizeof(cSeconds), "0%i", iSeconds);
		} else {
			Format(cSeconds, sizeof(cSeconds), "%i", iSeconds);
		}
		
		if (precise) {
			Format(newtime, newtimesize, "%i:%s.%i", iMinutes, cSeconds, RoundToFloor(fSeconds*1000));
		} else {
			Format(newtime, newtimesize, "%i:%s.%i", iMinutes, cSeconds, RoundToFloor(fSeconds*10));
		}
	} else if (iSeconds >= 1) {
		if (precise) {
			Format(newtime, newtimesize, "%i.%i", iSeconds, RoundToFloor(fSeconds*1000));
		} else {
			Format(newtime, newtimesize, "%i.%i", iSeconds, RoundToFloor(fSeconds*10));
		}
	} else {
		if (precise) {
			Format(newtime, newtimesize, "%.3f", fSeconds);
		} else {
			Format(newtime, newtimesize, "%.1f", fSeconds);
		}
	}
	
}

/*
	Creates databases if not already created.

*/
public void SQL_DBConnect() {
	if(g_SQLhandle != null)
		CloseHandle(g_SQLhandle);

	if(SQL_CheckConfig("fsociety")) {
		char sError[255];

		if(!(g_SQLhandle = SQL_Connect("fsociety", true, sError, 255)))
			SetFailState("Timer startup failed. Reason: %s", sError);

		SQL_LockDatabase(g_SQLhandle);
		SQL_FastQuery(g_SQLhandle, "SET NAMES 'utf8';");
		SQL_UnlockDatabase(g_SQLhandle);
		// Users table
		SQL_TQuery(g_SQLhandle, SQL_CreateTable_Callback, "CREATE TABLE IF NOT EXISTS `users` (`auth` VARCHAR(64) NOT NULL, `name` VARCHAR(32), `country` VARCHAR(45), `ip` VARCHAR(32), `playtime` INT, PRIMARY KEY (`auth`));");
		// Times table
		SQL_TQuery(g_SQLhandle, SQL_CreateTable_Callback, "CREATE TABLE IF NOT EXISTS `playertimes` (`id` INT NOT NULL AUTO_INCREMENT, `auth` VARCHAR(64), `map` VARCHAR(128), `time` FLOAT, `jumps` VARCHAR(32), `style` VARCHAR(32), `date` DATE, PRIMARY KEY (`id`));");
		// Time bonus table
		SQL_TQuery(g_SQLhandle, SQL_CreateTable_Callback, "CREATE TABLE IF NOT EXISTS `playertimesbonus` (`id` INT NOT NULL AUTO_INCREMENT, `auth` VARCHAR(64), `map` VARCHAR(128), `time` FLOAT, `jumps` VARCHAR(32), `style` VARCHAR(32), `date` DATE, PRIMARY KEY (`id`));");
		// Zones table
		SQL_TQuery(g_SQLhandle, SQL_CreateTable_Callback, "CREATE TABLE IF NOT EXISTS `mapzones` (`id` INT AUTO_INCREMENT, `map` VARCHAR(128), `type` INT, `corner1_x` FLOAT, `corner1_y` FLOAT, `corner1_z` FLOAT, `corner2_x` FLOAT, `corner2_y` FLOAT, `corner2_z` FLOAT, PRIMARY KEY (`id`));");
		// Ranks // Points, auth, epoch time
		SQL_TQuery(g_SQLhandle, SQL_CreateTable_Callback, "CREATE TABLE IF NOT EXISTS `ranks` (`id` INT AUTO_INCREMENT, `points` INT, `lostpoints` INT, `lastname` VARCHAR(32), `auth` VARCHAR(64), `epoch` INT NOT NULL, PRIMARY KEY (`id`));");
		// Messages
		SQL_TQuery(g_SQLhandle, SQL_CreateTable_Callback, "CREATE TABLE IF NOT EXISTS `messages` (`id` INT AUTO_INCREMENT, `name` varchar(32), `gameshown` INT, `webshown` INT, `epoch` INT NOT NULL, `message` VARCHAR(256), PRIMARY KEY (`id`));");
		
		#if defined SQL_DEBUG
			LogMessage("SQL_CreateTable_Callback");
		#endif
	
	} else 
		SetFailState("Timer startup failed. Reason: %s", "\"fsociety\" is not a specified entry in databases.cfg.");
}

/*
	Handles the database creation.

*/
public void SQL_CreateTable_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if(handle == null)
		LogError("Timer error! Users' data table creation failed. Reason: %s", error);
}

/*
	Handles the user creation.

*/
public void SQL_InsertUser_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if(handle == null) {
		int client = GetClientFromSerial(data);

		if(!client)
			LogError("Timer error! Failed to insert a disconnected player's data to the table. Reason: %s", error);
		else
			LogError("Timer error! Failed to insert \"%N\"'s data to the table. Reason: %s", client, error);

		return;
	}
}

public void SQL_InsertZone_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if(handle == null) {
		LogError("Timer (zone insert) SQL query failed. Reason: %s", error);
		return;
	}
	UnloadZones(0);
	RefreshZones();
}

/**
 *	Check if a position is inside two positions
 *
 *  @param reqPos                Position to check for
 *  @param point1                Position 1
 *	@param point2                Position 2
 *	@return bool                 If a coord is inisde the two positions
 */
public bool CoordsInsideCoord(float reqPos[3], float point1[3], float point2[3]) {
	int matches;
	for (int i = 0; i < 3; i++) {
		if (point1[i] > point2[i]) {
			if (point1[i] >= reqPos[i] && point2[i] <= reqPos[i])
				matches++;
		} else {
			if (point2[i] >= reqPos[i] && point1[i] <= reqPos[i])
				matches++;
		}
	}
	if (matches == 3)
		return true;
	else 
		return false;
}

/**
 *	Check if a player is inside two positions
 *
 *  @param client                Player's index
 *  @param point1                Position 1
 *	@param point2                Position 2
 *	@return bool                 If a player is inisde the two positions
 */
public bool InsideZone(int client, float point1[3], float point2[3]) {
	
	float clientPoints[8][3];
	float vPos[3], vPos2[3];
	
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", vPos);
	vPos2 = vPos;
	
	vPos[0] -= 5.0;
	vPos[1] -= 5.0;
	
	vPos2[0] += 5.0;
	vPos2[1] += 5.0;
	vPos2[2] += 78.0;
	
	clientPoints[0] = vPos;
	clientPoints[7] = vPos2;
	CreateZonePoints(clientPoints);

	for (int i = 0; i < 8; i++) {
		if (CoordsInsideCoord(clientPoints[i], point1, point2)) {
			return true;
		}
	}
	
	return false;
}  

/**
 *	Gets the distance between two points
 *
 *  @param vPos                  Position 1
 *	@param vPos2                 Position 2
 *	@return float                Distance between the two points given
 */
public float GetPositionDiff(const float vPos[3], const float vPos2[3]) {
	float diff;
	for (int idx; idx < 3; idx++) {
		diff += FloatAbs(vPos[idx] - vPos2[idx]);
	}
	return diff;
}

/**
 *	Gets the center of 2 positions.
 *
 *  @param vPos                  Position 1 
 *	@param vPos2                 Position 2
 *	@param vCenter               Centered Position
 *	@noreturn
 */
public void GetCenterPos(float vPos[3], float vPos2[3], float vCenter[3]) {
	for (int i = 0; i < 3; i++) {
		if (vPos[i] > vPos2[i]) {
			vCenter[i] = vPos2[i] + FloatAbs( (vPos2[i] - vPos[i]) / 2 );
		} else {
			vCenter[i] = vPos[i] + FloatAbs( (vPos[i] - vPos2[i]) / 2 );
		}
		vCenter[2] = vPos[2] + 5.0;
	}
}

/**
 *	Draw a zone in the world using 8 different points
 *
 *  @param client                Player's id
 *	@param array                 8 Points of the zone
 *	@param beamsprite            Beam Sprite
 *	@param halosprite            ???
 *	@param color                 Color of zone RGBA
 *	@param life                  How long the zone will stay until it fades.
 *	@noreturn
 */
public void DrawZone(int client, float array[8][3], int beamsprite, int halosprite, int color[4], float life) {
	for(int i = 0, i2 = 3; i2 >= 0; i += i2--) {
		for(int j = 1; j <= 7; j += (j / 2) + 1) {
			if(j != 7 - i) {
				TE_SetupBeamPoints(array[i], array[j], beamsprite, halosprite, 0, 0, life, BEAM_SIZE, BEAM_SIZE, 0, BEAM_JIGGLE, color, 0);
				if (0 < client <= MaxClients)
					TE_SendToClient(client, 0.0);
				else
					TE_SendToAll(0.0);
			}
		}
	}
}

/**
 *	Checks if x,y,z coords are empty
 *
 *  @param vZone               XYZ Coords
 *	@noreturn
 */
public bool EmptyZone(float vZone[3]) {
	if(vZone[0] == 0.0 && vZone[1] == 0.0 && vZone[2] == 0.0)
		return true;
	return false;
}

/**
 *	Create 8 points from 2 points
 *
 *  @param point                8 different points
 *	@noreturn
 */
public void CreateZonePoints(float point[8][3]) {
	for(int i = 1; i < 7; i++) 
		for(int j = 0; j < 3; j++)
			point[i][j] = point[((i >> (2-j)) & 1) * 7][j];
}

public void SQL_DeleteZonePosition_Callback(Handle owner, Handle handle, const char[] error, any data) {
	
	if(handle == null) {
		LogError("Timer (single zone delete) SQL query failed. Reason: %s", error);
		return;
	}
	
	UnloadZones(0);
	UnloadZoneBySQLID(data);
	RefreshZones();
}

public void SQL_DeleteZoneType_Callback(Handle owner, Handle handle, const char[] error, any data) {
	if (handle == null) {
		LogError("Timer (single zone delete) SQL query failed. Reason: %s", error);
		return;
	}
	UnloadZoneBySQLID(data);
	UnloadZones(0);
	RefreshZones();
}

public int ZoneCreation_Handler(Handle menu, MenuAction action, int param1, int param2) {
	if(action == MenuAction_Select) {
		Reset(param1);
	} else if (action == MenuAction_End)
		CloseHandle(menu);
}

stock CheckVelocity(client, type, Float:maxspeed) {
	if(maxspeed > 0.0) {
		new Float:fVelocity[3];
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", fVelocity);
		
		if(fVelocity[0] == 0.0)
			fVelocity[0] = 1.0;
			
		if(fVelocity[1] == 0.0)
			fVelocity[1] = 1.0;
			
		if(fVelocity[2] == 0.0)
			fVelocity[2] = 1.0;
		
		new Float:currentspeed = SquareRoot(Pow(fVelocity[0],2.0)+Pow(fVelocity[1],2.0));
		
		if (type == 0) { // 0: slow
			maxspeed = currentspeed/2.0;
			new Float:Multpl = currentspeed / maxspeed;
			fVelocity[0] /= Multpl;
			fVelocity[1] /= Multpl;

			TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, fVelocity);
		} else if(type == 1) { // 1: limit
			if (currentspeed >= maxspeed)
			{
				new Float:Multpl = currentspeed / maxspeed;
				fVelocity[0] /= Multpl;
				fVelocity[1] /= Multpl;

				TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, fVelocity);
			}
		}
	}
}