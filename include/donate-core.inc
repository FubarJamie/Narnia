#if defined _donate_core_included
 #endinput
#endif
#define _donate_core_included

native Donate_GetTime(int client);

public __pl_donate_core_SetNTVOptional() 
{
	MarkNativeAsOptional("Donate_GetTime");
}