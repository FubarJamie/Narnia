
public bool EntityInsideZone(float vPos[3]) {
	
	for (int i = 0; i < MAX_ZONES; i++)
		if (CoordsInsideCoord(vPos, g_MapZones[i][Zone_PushFix][0], g_MapZones[i][Zone_PushFix][1]))
			return true;
	
	return false;
}

public void FixPush() {
	
	new FindMaxEntities = GetMaxEntities();
	char name[64];
	float vPos[3];
    
	for(new i = 0; i < FindMaxEntities; i++) {
        if(IsValidEntity(i)) {
            GetEdictClassname(i, name, sizeof(name));
            if(!strcmp(name, "trigger_push", false)) {
            	
				Entity_GetAbsOrigin(i, vPos);
				
				if (EntityInsideZone(vPos)) {
					LogMessage("EntityHook: found entity inside zone.");
					HookSingleEntityOutput(i, "OnStartTouch", PushTouch);
				}
			}
        }
    }
}

public PushTouch(const char[] output, ent, client, float delay)
{
	if(!(1<=client<= MaxClients ) || (ent == -1) || (!IsClientInGame(client)) || !IsPlayerAlive(client))
		return;
		
	new fixflags = GetEntityFlags(client);
	if (fixflags & FL_ONGROUND)
	{
		//SetEntityFlags(client, (fix != FL_ONGROUND)); - i dunno how make this work xD
		float Pushfix[3];
		float vel[3];
		GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", vel);
		GetClientAbsOrigin(client, Pushfix);
		Pushfix[2] += 5.0;
		TeleportEntity(client, Pushfix, NULL_VECTOR, vel);
		CreateTimer(0.5,TimerFix,client);
	}
}
public Action TimerFix(Handle timer,any client)
{
	float Pushfix[3];
	GetClientAbsOrigin(client, Pushfix);
	float vel[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", vel);
	Pushfix[2] -= 5.0;
	TeleportEntity(client, Pushfix, NULL_VECTOR, vel);
}