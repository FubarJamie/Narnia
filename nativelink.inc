

/*
	Gets a players sync
	
	int client      Player's index in the server.
*/
native int Jumes_GetSync(int client);

/*
	Check if a player is a follower if so return their leader
	
	int client      Player's index in the server
	
*/
native int Jumes_IsFollowerGetLeader(int client);