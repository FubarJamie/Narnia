#if defined JumesLoadedVars
	#endinput
#endif
#define JumesLoadedVars

#define PREFIX " \x01[\x06Timer\x01] \x06-\x01"
#define BEAM_SIZE 1.2
#define BEAM_JIGGLE 0.0
#define MAX_ZONES 8

//#define SQL_DEBUG false

#define SPECMODE_NONE           0
#define SPECMODE_FIRSTPERSON    4
#define SPECMODE_3RDPERSON      5
#define SPECMODE_FREELOOK       6

#include "nativelink"

// Map Zone types
enum MapZones(+=1) {
	Zone_Start = 0,
	Zone_End,
	Zone_Block,
	Zone_Hover,
	Zone_Slay,
	Zone_Count
};

#define ZONE_COUNT view_as<int>(Zone_Count)

// Map Zone type names
char g_ZoneNames[ZONE_COUNT][] = {
	"Start Zone",
	"End Zone",
	"Block Player",
	"Hover Zone",
	"Slay Zone"
};

int g_ZoneColours[ZONE_COUNT][4] = {
	{59, 179, 0, 255},  // Zone_Start
	{209, 0, 0, 255},   // Zone_End
	{255, 200, 0, 0},   // Zone_Block
	{31, 64, 255, 255}, // Zone_Hover
	{255, 78, 255, 255} // Zone_Slay
};

// Bhop styles
enum BhopStyle(+=1) {
	Style_Auto = 0,
	//Style_Sideways,
	Style_Legit,
	/*Style_WOnly,
	Style_DOnly,
	Style_SOnly,
	Style_AOnly,
	Style_Easy,
	Style_HalfSideWays*/
	BhopStyle_Size
};

#define MAX_STYLES view_as<int>(BhopStyle_Size)

char g_StyleNames[MAX_STYLES][] = {
	"Auto",
	//"SW",
	"Legit"
	/*"W-Only",
	"D-Only",
	"S-Only",
	"A-Only",
	"Easy",
	"HalfSW"*/
};

enum Points(+=1) {
	Points_OnFirstFinish = 0,
	Points_Size
};

char g_WRGrad[21][8] =  { "#00CC00","#0DC700","#1AC200","#26BD00","#33B800","#40B200","#4CAD00","#59A800","#66A300","#739E00","#809900","#8C9400","#998F00","#A68A00","#B28500","#BF8000","#CC7A00","#D97500","#E67000","#F26B00","#FF6600" };
char g_OWRGrad[21][8] =  { "#FF6600","#FF6100","#FF5C00","#FF5700","#FF5200","#FF4C00","#FF4700","#FF4200","#FF3D00","#FF3800","#FF3300","#FF2E00","#FF2900","#FF2400","#FF1F00","#FF1A00","#FF1400","#FF0F00","#FF0A00","#FF0500","#FF0000" };
char g_SpeedGrad[21][8] =  { "#FFFFFF","#FFF7EA","#FFEFD6","#FFE7C1","#FFDFAD","#FFD899","#FFD084","#FFC870","#FFC05B","#FFB847","#FFB133","#FFA534","#FF9A35","#FF8E36","#FF8337","#FF7838","#FF6C39","#FF613A","#FF553B","#FF4A3C","#FF3F3D" };

// Database handle
Database g_SQLhandle = null;

// Auto Bhop
bool g_AutoBhop[MAXPLAYERS + 1] = {true, ...};

// Jump Detection
bool g_Jumping[MAXPLAYERS + 1];
int g_JumpIteration[MAXPLAYERS + 1];

// Hud Variables
int g_CurrentButtons[MAXPLAYERS + 1];

// Stats Variables


// WR Variables

BhopStyle gBS_LastWR[MAXPLAYERS+1];
float g_WRTime[MAX_STYLES];
char g_WRName[MAX_STYLES][MAX_NAME_LENGTH];

//float g_BonusWRTime[MAX_STYLES];
//char g_BonusWRName[MAX_STYLES][MAX_NAME_LENGTH];

float g_PlayerRecord[MAXPLAYERS+1][MAX_STYLES];
//float g_BonusPlayerRecord[MAXPLAYERS + 1][MAX_STYLES];

//int g_MaxRecords = 0;
int g_SMaxRecords[MAX_STYLES];
int g_CurrPosWR[MAXPLAYERS + 1][MAX_STYLES];
int g_RealTimePosWR[MAXPLAYERS + 1][MAX_STYLES];

//int g_CurrPosWR[MAXPLAYERS + 1];
//int g_SCurrPosWR[MAXPLAYERS + 1][MAX_STYLES];
//int g_CurrRLPosWR[MAXPLAYERS + 1] =  { 0, ... };
//int g_SCurrRLPosWR[MAXPLAYERS + 1][MAX_STYLES];

//int g_BonusMaxRecords = 0;
//int g_BonusCurrPosWR[MAXPLAYERS + 1];
//int g_BonusCurrRLPosWR[MAXPLAYERS + 1] =  { 0, ... };

// Timer Variables
BhopStyle g_CurrentStyle[MAXPLAYERS + 1] =  { Style_Auto, ... };
MapZones g_LastZone[MAXPLAYERS + 1] = Zone_Count;
bool g_TimerEnabled[MAXPLAYERS + 1];
float g_StartTime[MAXPLAYERS + 1];
float g_PauseStartTime[MAXPLAYERS + 1];
float g_PauseTotalTime[MAXPLAYERS + 1];
bool g_TimerPaused[MAXPLAYERS + 1];
int g_Jumps[MAXPLAYERS + 1];

bool g_ExtraLife[MAXPLAYERS + 1] = true;

// Zone Variables
char g_Map[128];
int g_MapStep[MAXPLAYERS + 1];
bool g_Button[MAXPLAYERS + 1];
MapZones g_Type[MAXPLAYERS + 1];
float g_lastPos[MAXPLAYERS + 1][3];

int g_BeamSprite;
int g_HaloSprite;

float g_Point1[MAXPLAYERS + 1][3];
float g_Point2[MAXPLAYERS + 1][3];
int g_ZoneTeleport[MAXPLAYERS + 1];

float g_MapZones[MAX_ZONES][ZONE_COUNT][2][3];
int g_MapZoneSQLInt[MAX_ZONES][ZONE_COUNT];

//ArrayList g_BulletPositions;
//ArrayList g_BulletPositions2;

// Ranks

/*int g_PlayersPoints[MAXPLAYERS + 1];
int g_PlayersRank[MAXPLAYERS + 1] = {0, ...};

//int g_RankPoints[100];
int g_RankPosition[100];
int g_PointsGiven[Points_Size];

char g_RankNames[100][64];
char g_ClanTag[100][64];
char g_ChatPrefix[100][64];*/
//int  g_NeededPosition[100];

#define MAX_RANKS 50
#define FLAG_WRADD 0
#define FLAG_WRREMOVE 1
#define FLAG_WRIMPROVE 2

int g_RanksMaxRecord = 0;

int g_PlayersPoints[MAXPLAYERS + 1];
int g_PlayersRank[MAXPLAYERS + 1] = {0, ...};
bool g_ShowChatRank[MAXPLAYERS + 1] = {true, ...};

int g_PointsGiven[Points_Size];

int g_WRRank[MAX_RANKS];
int g_WRAdd[MAX_RANKS];
int g_WRRemove[MAX_RANKS];
int g_WRImprove[MAX_RANKS];

int g_RankPosition[100];

char g_RankNames[100][64];
char g_ClanTag[100][64];
char g_ChatPrefix[100][64];

bool g_Hide[MAXPLAYERS+1] = {false, ...};
bool g_HideSave[MAXPLAYERS + 1] =  { false, ... };
bool g_HideHud[MAXPLAYERS + 1] =  { false, ... };

//cvars
ConVar gCv_prespeed = null;