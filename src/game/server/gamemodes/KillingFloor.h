/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_KillingFloor_H
#define GAME_SERVER_GAMEMODES_KillingFloor_H
#include <game/server/gamecontroller.h>
#include <game/server/entity.h>

//#define MAX_FLAGS 10
#define MAX_TELEPORTS 40
#define MAX_LASERWALLS 200
#define MAX_LOSEPOINTS 40
#define MAX_PICKUPS 1024
#define MAX_DROPPABLES 30

enum GameState
{
	GAMESTATE_STARTING,
	GAMESTATE_ROUND,
	GAMESTATE_ENDING,
	GAMESTATE_WIN,
	GAMESTATE_LOSE
};


class CGameControllerKillingFloor : public IGameController
{
private:
	bool m_ZombiesCreated;
	bool m_DroppablesCreated;
	
	int m_ZombiesLeft;
	int m_GameState;
	int m_Round;
	int m_MaxRounds;
	int m_RoundTick;
	
	void KillZombies();
	
	void Restart();
	void StartRound();
	void WaveCleanup();
	
	bool CheckLose();
	void LoseRound();
	
	int m_LosezoneWhine;
	
	int CountPlayers();
	int CountPlayersAlive();
	
	int UpdateZombieScores();
	
	
	class CAI *GetNextEnemyAI(class CCharacter *pChr);
	int m_RoundZombieCount;
	
	// used for snapping flags to a closest player & checking for teleports
	//class CFlag *GetClosestFlag(vec2 Pos, int Team = -1);
	class CTeleport *GetClosestTeleport(vec2 Pos, int Type = -1);
	
	// store pickup pointers
	class CPickup *m_apPickup[MAX_PICKUPS];
	int m_PickupCount;
	
	// for item drops
	int m_PickupDropCount;
	
	void CreateDroppables();
	
	void RespawnPickups();
	void ClearPickups();
	
	bool ForceNogo(vec2 Pos);
	
	int m_LastReleaseSound;

	
	class CStaticlaser *m_apLaserwall[MAX_LASERWALLS];
	void AddLaserwall(vec2 Pos);
	int m_LaserwallCount;
	
	class CLosepoint *m_apLosepoint[MAX_LOSEPOINTS];
	bool InsideLosepoint(vec2 Pos);
	int m_LosepointCount;
	
	class CTeleport *m_apTeleport[MAX_TELEPORTS];
	class CTeleport *GetTeleport(int Team = -1);
	int m_TeleportCounter;
	
	void ResetLaserwalls();
	void ReleaseLaserwalls(vec2 Pos);
	
	
	class CCharacter *m_Head[MAX_CLIENTS];
	
	
	//class CCharacter *m_RequireBody;
	
	class CCharacter *m_BossBody;
	class CCharacter *m_BossHead;
	
	
public:
	
	CGameControllerKillingFloor(class CGameContext *pGameServer);
	
	virtual void DoWincheck();
	virtual bool CanBeMovedOnBalance(int ClientID);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
	
	virtual void DropPickup(vec2 Pos, int PickupType, int PickupSubtype = -1);
	void FlashPickups();

	/*
	virtual class CCharacter *GetHead();
	virtual void SetHead(class CCharacter *Boss);
	*/
	
	virtual bool OnEntity(int Index, vec2 Pos);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	void OnCharacterSpawn(class CCharacter *pChr);
	bool CanCharacterSpawn(int ClientID);
};
#endif
