public Action UpdateMaxRank_Timer(Handle Timer) {
	
	SQL_TQuery(g_SQLhandle, SQL_UpdateMaxRank_Callback, "SELECT COUNT(*) FROM ranks");
	
	#if defined SQL_DEBUG 
		LogMessage("SQL_UpdateMaxRank_Callback");
	#endif
	
	for (int i = 1; i < MaxClients; i++) {
		if (!IsValidClient(i, true))
			continue;
			
		GetPosWR(i, view_as<int>(g_CurrentStyle[i]));
		Timer_GetMaxWR(view_as<int>(g_CurrentStyle[i]));
		UpdateEpoch(i);
	}
}

public Action UpdateRealTime_Timer(Handle Timer) {

	for (int i = 1; i < MaxClients; i++) {
		if (!IsValidClient(i, true))
			continue;
		
		float fTime = CalculateTime(i);
		GetRLPosWR(i, fTime, view_as<int>(g_CurrentStyle[i]));
	}

}

public Action JumpCheck_Timer(Handle Timer) {
	for (int i = 1; i < MaxClients; i++) {
		if(!IsValidClient(i, true)) 
			break;
		if (g_JumpIteration[i] > 1) {
			g_Jumping[i] = false;
			g_JumpIteration[i] = 0;
		}
		g_JumpIteration[i]++;
	}
	
}

public Action UpdateHUD_Timer(Handle Timer) {
	for (int i = 1; i < MaxClients; i++) {
		if (!IsValidClient(i))
			continue;
		UpdateHUD(i);
	}
}