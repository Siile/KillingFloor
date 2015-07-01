/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

// this include should perhaps be removed
#include "entities/character.h"
#include "gamecontext.h"
#include "ai.h"

#include <game/server/upgradelist.h>



enum TryBuyWeapon
{
	 BUY_OK = 0,
	 BUY_NOCHARACTER = -1,
	 BUY_NOPOINTS = -2,
	 BUY_HASALREADY = -3,
	 BUY_NOWEAPON = -4,
	 BUY_CLIPSFULL = -5,
	 BUY_INVALIDWEAPON = -6
};



// player object
class CPlayer
{
	MACRO_ALLOC_POOL_ID()

public:
	CPlayer(CGameContext *pGameServer, int ClientID, int Team);
	~CPlayer();

	void Init(int CID);

	void TryRespawn();
	void Respawn();
	void SetTeam(int Team, bool DoChatMsg=true);
	int GetTeam() const { return m_Team; };
	int GetCID() const { return m_ClientID; };

	void Tick();
	void PostTick();
	void Snap(int SnappingClient);

	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);
	void OnDisconnect(const char *pReason);

	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();

	//---------------------------------------------------------
	// this is used for snapping so we know how we can clip the view for the player
	vec2 m_ViewPos;

	// states if the client is chatting, accessing a menu etc.
	int m_PlayerFlags;

	// used for snapping to just update latency if the scoreboard is active
	int m_aActLatency[MAX_CLIENTS];

	// used for spectator mode
	int m_SpectatorID;

	bool m_IsReady;

	//
	int m_Vote;
	int m_VotePos;
	//
	int m_LastVoteCall;
	int m_LastVoteTry;
	int m_LastChat;
	int m_LastSetTeam;
	int m_LastSetSpectatorMode;
	int m_LastChangeInfo;
	int m_LastEmote;
	int m_LastKill;

	// TODO: clean this up
	struct
	{
		char m_SkinName[64];
		int m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;
	} m_TeeInfos;

	int m_Respawns;
	
	int m_RespawnTick;
	int m_DieTick;
	int m_Score;
	int m_ScoreStartTick;
	bool m_ForceBalanced;
	int m_LastActionTick;
	int m_TeamChangeTick;
	struct
	{
		int m_TargetX;
		int m_TargetY;
	} m_LatestActivity;

	// network latency calculations
	struct
	{
		int m_Accum;
		int m_AccumMin;
		int m_AccumMax;
		int m_Avg;
		int m_Min;
		int m_Max;
	} m_Latency;
	
	CAI *m_pAI;
	
	void AITick();
	bool AIInputChanged();
	
	
	// custom
	bool BuyableWeapon(int i);
	void ListBuyableWeapons();
	void VoteBuyableWeapons();
	bool BuyWeapon(int CustomWeapon);

	bool GotWeapon(int CustomWeapon)
	{
		if (!GetCharacter())
			return false;
		return m_pCharacter->GotWeapon(CustomWeapon);
	}
	
	bool WeaponDisabled(int CustomWeapon)
	{
		if (!GetCharacter())
			return false;
		return m_pCharacter->WeaponDisabled(CustomWeapon);
	}
	
	void IDDQD();
	void IDKFA();
	void GreedIsGood();
	
	// commando, medic, pioneer, berserk
	int m_Class;
	int m_ClassPickWhine;
	
	void TrySetClass(int WantedClass);
	
	int m_TipTimer[NUM_TIPS];
	int m_TipEnable[NUM_TIPS]; // 0 = disabled
	
	void ShowTip(int Tip);
	void DisableTip(int Tip);
	
private:
	CCharacter *m_pCharacter;
	CGameContext *m_pGameServer;

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const;

	//
	bool m_Spawning;
	int m_ClientID;
	int m_Team;
};

#endif
