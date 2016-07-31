#define SPECMODE_NONE           0
#define SPECMODE_FIRSTPERSON    4
#define SPECMODE_3RDPERSON      5
#define SPECMODE_FREELOOK       6

public void UpdateHUD(int client) {
	
	/*if (InsideZoneByType(client, Zone_Start)) {
		PrintHintText(client, "Inside start zone\n%i\n%i",GetArraySize(g_BulletPositions), GetArraySize(g_BulletPositions2));
	} else {
		PrintHintText(client, "Outside start zone\n%i\n%i", GetArraySize(g_BulletPositions), GetArraySize(g_BulletPositions2));
	}*/
	
	if (g_HideHud[client])
		return;
	
	int target = client;

	if(IsClientObserver(client))
	{
		if(GetEntProp(client, Prop_Send, "m_iObserverMode") >= 3)
		{
			int iTarget = GetEntPropEnt(client, Prop_Send, "m_hObserverTarget");

			if(IsValidClient(iTarget, true))
			{
				target = iTarget;
			}
		}
	}
	
	if (!(Jumes_IsFollowerGetLeader(client) == -1)) {
		target = Jumes_IsFollowerGetLeader(client);
	}
	
	if(!IsFakeClient(target)) {
		
		//bool inConga = false;
		//int targetsLeader = Jumes_IsFollowerGetLeader(target);
		//if (targetsLeader == -1)
			//inConga = false;
		//else
			//inConga = true;
			
		//if (inConga) 
			//target = targetsLeader;
	
		float fTime = CalculateTime(target);
		
		if (g_TimerEnabled[target] == false) {
			fTime = -1.0;
		}
		
		float fSpeed[3];
		GetEntPropVector(target, Prop_Data, "m_vecVelocity", fSpeed);

		float fSpeed_New = SquareRoot(Pow(fSpeed[0], 2.0) + Pow(fSpeed[1], 2.0));
		
		//float fWR = g_WRTime[g_CurrentStyle[target]];
		
		char sTime[32];
		if (fTime == -1.0) {
			Format(sTime, 32, "Stopped");
		} else {
			FormatSeconds(fTime, sTime, 32, false);
		}
		
		// Style, Sync, Spec, PersonalBest, Time, Speed, keys.
		
		char sPB[64], style[64], sync[64], time[64], speed[64], bottomKeys[128], WR[64], spec[64];
		
		float fPB = g_PlayerRecord[target][g_CurrentStyle[target]];
		if (fPB <= 0)
			sPB = "N/A";
		else
			FormatSeconds(fPB, sPB, sizeof(sPB));
			
		Format(style, sizeof(style), "%s", g_StyleNames[view_as<BhopStyle>(g_CurrentStyle[target])]);
		
		int syncInt = Jumes_GetSync(target);
		Format(sync, sizeof(sync), "%iX", syncInt);
		
		char colour2[8];
		
		if (!g_TimerEnabled[target]) {
			Format(colour2, 8, "#FF0055");
		} else if (g_SMaxRecords[g_CurrentStyle[target]] == 0) {
			Format(colour2, 8, "#00CC00");
		} else if (g_WRTime[g_CurrentStyle[target]] > fTime) {
			Format(colour2, 8, "#66ff33");
			
			int size = sizeof(g_WRGrad);

			int percent = RoundFloat(Math_GetPercentageFloat(fTime, g_WRTime[g_CurrentStyle[target]]));
			int percentInc = Math_GetPercentage(1, size);
			
			for (int i = 0; i < size; i++) {
				if (percent <= (percentInc * (i+1))) {
					Format(colour2, 8, g_WRGrad[i]);
					break;
				}
			}
		} else if (g_PlayerRecord[target][g_CurrentStyle[target]] > fTime) {
			
			int size = sizeof(g_OWRGrad);
			
			int percent = RoundFloat(Math_GetPercentageFloat(fTime, g_PlayerRecord[target][g_CurrentStyle[target]]));
			int percentInc = Math_GetPercentage(1, size);
			
			for (int i = 0; i < size; i++) {
				if (percent <= (percentInc * (i+1))) {
					Format(colour2, 8, g_OWRGrad[i]);
					break;
				}
			}
		} else {
			Format(colour2, 8, "#FF0055");
		}

		if (g_TimerEnabled[target])
			Format(time, sizeof(time), "<font color='%s'>%s</font> #%i", colour2, sTime, g_RealTimePosWR[target][g_CurrentStyle[target]]);
		else
			Format(time, sizeof(time), "<font color='%s'>%s</font>", colour2, sTime);
		
		char cSpeed[7];
		int currentspeed = RoundFloat(fSpeed_New);
		
		if(currentspeed < 240)
	        Format(cSpeed, sizeof(cSpeed), "FFFFAA");
		else if(currentspeed < 300)
	        Format(cSpeed, sizeof(cSpeed), "FFFF88");
		else if(currentspeed < 500)
	        Format(cSpeed, sizeof(cSpeed), "FFFF66");
		else if(currentspeed < 700)
	        Format(cSpeed, sizeof(cSpeed), "FFFF44");
		else if(currentspeed < 900)
	        Format(cSpeed, sizeof(cSpeed), "FFFF22");
		else if(currentspeed < 1100)
	        Format(cSpeed, sizeof(cSpeed), "FFCC11");
		else if(currentspeed < 1300)
	        Format(cSpeed, sizeof(cSpeed), "FFBB00");
		else if(currentspeed < 1500)
	        Format(cSpeed, sizeof(cSpeed), "FF8800");
		else
	        Format(cSpeed, sizeof(cSpeed), "FF0000");
	        
		int percentageSpeed;
	    
		percentageSpeed = Math_GetPercentage(currentspeed, 850);
		
		Format(speed, sizeof(speed), "<font color='#%s'>%i</font>", cSpeed, currentspeed);
		
		if (g_WRTime[g_CurrentStyle[target]] <= 0.0001) {
			Format(WR, sizeof(WR), "N/A");
		} else {
			FormatSeconds(g_WRTime[g_CurrentStyle[target]], WR, sizeof(WR));
			Format(WR, sizeof(WR), "<font color='#cc33ff'>%s</font>", WR);
		}
		
		int specInt = GetSpectators(target);
		Format(spec, sizeof(spec), "%i", specInt);
		
		int buttons = GetClientButtons(target);
		
		char jump[23];

		if (g_Jumping[target]) {
			jump = "Jump";
		}
		
		Format(bottomKeys, sizeof(bottomKeys), "%s		", jump);
	
		char asd[128];
	
		if (buttons & IN_MOVELEFT) {
			Format(asd, sizeof(asd), "%sA", asd);
		} else {
			Format(asd, sizeof(asd), "%s_", asd);
		}
			
		if (buttons & IN_BACK) {
			Format(asd, sizeof(asd), "%s S", asd);
		} else {
			Format(asd, sizeof(asd), "%s _", asd);
		}
			
		if (buttons & IN_MOVERIGHT) {
			Format(asd, sizeof(asd), "%s D", asd);
		} else {
			Format(asd, sizeof(asd), "%s _", asd);
		}
		
		char duck[10];
		
		if (buttons & IN_DUCK) 
			duck = "		Duck";
			
		Format(bottomKeys, sizeof(bottomKeys), "%s%s%s", bottomKeys, asd, duck);
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		char sHintText[1024];
		
		FormatEx(sHintText, sizeof(sHintText), "<font face='Stratum2'>");
		
		if (!(Jumes_IsFollowerGetLeader(client) == -1)) {
			FormatEx(sHintText, sizeof(sHintText), "<font size='16'>");

			Format(sHintText, sizeof(sHintText), "%s	       <font color='#0080ff'>CongaLine</font>\n", sHintText);
			
			Format(sHintText, sizeof(sHintText), "%sPB: %s", sHintText, sPB);
			Format(sHintText, sizeof(sHintText), "%s			", sHintText);

			Format(sHintText, sizeof(sHintText), "%sWR: %s", sHintText, WR);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);

			Format(sHintText, sizeof(sHintText), "%sSpec: %s", sHintText, spec);
			
			char chara[3] = "_";
			if (buttons & IN_FORWARD) {
				chara = "W";
			}
			
			Format(sHintText, sizeof(sHintText), "%s	   %s		", sHintText, chara);
			
			Format(sHintText, sizeof(sHintText), "%s %su/s", sHintText, speed);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);
		
			Format(sHintText, sizeof(sHintText), "%s%s", sHintText, bottomKeys);
			
		} else if (InsideZoneByType(client, Zone_End) || InsideZoneByType(client, Zone_Start) || g_LastZone[target] == Zone_End || g_LastZone[target] == Zone_Count) { // In end zone
			
			FormatEx(sHintText, sizeof(sHintText), "<font size='16'>");

			Format(sHintText, sizeof(sHintText), "%s	       SourceRuns", sHintText);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);
			
			Format(sHintText, sizeof(sHintText), "%sPB: %s", sHintText, sPB);
			Format(sHintText, sizeof(sHintText), "%s			", sHintText);
			
			// WR
			Format(sHintText, sizeof(sHintText), "%sWR: %s", sHintText, WR);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);

			Format(sHintText, sizeof(sHintText), "%sSpec: %s", sHintText, spec);
			
			char chara[3] = "_";
			if (buttons & IN_FORWARD) {
				chara = "W";
			}
			
			Format(sHintText, sizeof(sHintText), "%s	   %s		", sHintText, chara);
			
			//%s<font size=\"16\">%s</font>        
			
			Format(sHintText, sizeof(sHintText), "%s%s u/s", sHintText, speed);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);
			
		
			Format(sHintText, sizeof(sHintText), "%s%s", sHintText, bottomKeys);
			
			//FormatEx(sHintText, sizeof(sHintText), "%s</font>", sHintText);
			
		} else { // timer started
			
			FormatEx(sHintText, sizeof(sHintText), "<font size='16'>");

			Format(sHintText, sizeof(sHintText), "%sTime: %s", sHintText, time);
			Format(sHintText, sizeof(sHintText), "%s   		", sHintText);
	
			//Format(sHintText, sizeof(sHintText), "%sSpeed: %s (%i)", sHintText, speed, percentageSpeed);
			Format(sHintText, sizeof(sHintText), "%sSpeed: %s", sHintText, speed);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);
			
			Format(sHintText, sizeof(sHintText), "%sPB: %s", sHintText, sPB);
			Format(sHintText, sizeof(sHintText), "%s			", sHintText);
			
			Format(sHintText, sizeof(sHintText), "%sSync: %s", sHintText, sync);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);

			Format(sHintText, sizeof(sHintText), "%sSpec: %s", sHintText, spec);
			
			char chara[3] = "_";
			if (buttons & IN_FORWARD) {
				chara = "W";
			}
			
			Format(sHintText, sizeof(sHintText), "%s	   %s		", sHintText, chara);
			
			
			Format(sHintText, sizeof(sHintText), "%sStyle: %s", sHintText, style);
			Format(sHintText, sizeof(sHintText), "%s\n", sHintText);
			
			Format(sHintText, sizeof(sHintText), "%s%s", sHintText, bottomKeys);
		}
		
		Format(sHintText, sizeof(sHintText), "%s</font>", sHintText);
	
		ReplaceString(sHintText, sizeof(sHintText), "X", "%c");
		
		PrintHintText(client, sHintText, "%");
	}
}

int GetSpectators(int client) {
	int SpecCount = 0;
	if (IsPlayerAlive(client)) {
		for (int i = 1; i <= MaxClients; i++) {
			if (!IsClientInGame(i) || !IsClientObserver(i)) continue;
			
			int SpecMode = GetEntProp(i, Prop_Send, "m_iObserverMode");
			if (SpecMode != SPECMODE_FIRSTPERSON && SpecMode != SPECMODE_3RDPERSON) continue;
			int Target = GetEntPropEnt(i, Prop_Send, "m_hObserverTarget"); 
			if (Target == client) SpecCount++;
		}
	}
	return SpecCount;
}