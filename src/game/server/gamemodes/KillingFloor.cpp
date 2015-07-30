/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/entities/teleport.h>
#include <game/server/entities/losepoint.h>
#include <game/server/entities/pickup.h>
#include <game/server/entities/staticlaser.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "KillingFloor.h"

#include <game/server/ai.h>
#include <game/server/ai/runner.h>
#include <game/server/ai/ninja.h>
#include <game/server/ai/pistolbot.h>
#include <game/server/ai/bomber.h>
#include <game/server/ai/tank.h>
#include <game/server/ai/shotgunner.h>
#include <game/server/ai/spiderbomber.h>
#include <game/server/ai/madbomber.h>
#include <game/server/ai/laserbot.h>
#include <game/server/ai/electro.h>
#include <game/server/ai/grabber.h>
#include <game/server/ai/lightningbot.h>
#include <game/server/ai/flybot.h>
#include <game/server/ai/bossbody.h>
#include <game/server/ai/bosshead.h>
#include <game/server/ai/bossminion.h>


/*
static const char *aDifficulty[NUM_DIFFICULTIES] = 
{
	"Normal",
	"Hard",
	"Suicidal"
};
*/



CGameControllerKillingFloor::CGameControllerKillingFloor(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "KF";

	for (int i = 0; i < MAX_TELEPORTS; i++)
		m_apTeleport[i] = NULL;
	
	for (int i = 0; i < MAX_LOSEPOINTS; i++)
		m_apLosepoint[i] = NULL;
		
	for (int i = 0; i < MAX_PICKUPS; i++)
		m_apPickup[i] = NULL;
	
	for (int i = 0; i < MAX_LASERWALLS; i++)
		m_apLaserwall[i] = NULL;
		
	m_LosepointCount = 0;
	m_LaserwallCount = 0;
	
	m_PickupCount = 0;
	m_PickupDropCount = 0;
	m_ZombiesCreated = false;
	m_DroppablesCreated = false;
	
	m_MaxRounds = 7;
	Restart();
	
	m_TeleportCounter = 0;
	
	m_LosezoneWhine = 0;
	
	m_LastReleaseSound = 0;
}



void CGameControllerKillingFloor::DropPickup(vec2 Pos, int PickupType, int PickupSubtype)
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && m_apPickup[i]->m_Dropable && m_apPickup[i]->m_Life <= 0 && m_apPickup[i]->GetType() == PickupType)
		{
			m_apPickup[i]->m_Pos = Pos;
			m_apPickup[i]->RespawnDropable();
			if (m_apPickup[i]->GetType() == POWERUP_WEAPON)
				m_apPickup[i]->SetSubtype(WEAPON_SHOTGUN+frandom()*3);
			return;
		}
	}
}



bool CGameControllerKillingFloor::InsideLosepoint(vec2 Pos)
{
	for (int i = 0; i < m_LosepointCount; i++)
	{
		if (Pos.x > m_apLosepoint[i]->m_Pos.x - m_apLosepoint[i]->m_Range.x &&
			Pos.x < m_apLosepoint[i]->m_Pos.x + m_apLosepoint[i]->m_Range.y &&
			Pos.y > m_apLosepoint[i]->m_Pos.y - m_apLosepoint[i]->m_Range.z &&
			Pos.y < m_apLosepoint[i]->m_Pos.y + m_apLosepoint[i]->m_Range.w)
		{
			return true;	
		}
	}
	
	return false;
}


void CGameControllerKillingFloor::AddLaserwall(vec2 Pos)
{
	// check for invalidity
	//if(!(GameServer()->Collision()->GetCollisionAt(Pos.x, Pos.y)&CCollision::COLFLAG_NOGO))
	//	return;
	
	// find edges
	int StepSize = 16;
	
	
	//new CStaticlaser(&GameServer()->m_World, Pos, Pos + vec2(100, 0)); // testing
	
	
	// x-
	for (int x = Pos.x; x > Pos.x - 512; x -= StepSize)
	{
		if(!(GameServer()->Collision()->GetCollisionAt(x - StepSize, Pos.y+32)&CCollision::COLFLAG_NOGO))
			break;
		
		vec2 From = vec2(x, Pos.y);
		vec2 To = From;
				
		// find start and end positions for laser wall
		for (int y = Pos.y - 32; y > Pos.y - 256; y -= 4)
		{
			From.y = y;
			if(!(GameServer()->Collision()->GetCollisionAt(x, y)&CCollision::COLFLAG_NOGO))
				break;
		}
				
		for (int y = Pos.y + 32; y < Pos.y + 256; y += 4)
		{
			To.y = y;
			if(!(GameServer()->Collision()->GetCollisionAt(x, y)&CCollision::COLFLAG_NOGO))
				break;
		}
				
		// add laser wall
		CStaticlaser *L = new CStaticlaser(&GameServer()->m_World, From, To);
		m_apLaserwall[m_LaserwallCount++] = L;
	}
	
	// x+
	for (int x = Pos.x + StepSize; x < Pos.x + 512; x += StepSize)
	{
		if(!(GameServer()->Collision()->GetCollisionAt(x, Pos.y+32)&CCollision::COLFLAG_NOGO))
			break;
		
		vec2 From = vec2(x, Pos.y);
		vec2 To = From;
				
		// find start and end positions for laser wall
		for (int y = Pos.y - 32; y > Pos.y - 256; y -= 4)
		{
			From.y = y;
			if(!(GameServer()->Collision()->GetCollisionAt(x, y)&CCollision::COLFLAG_NOGO))
				break;
		}
				
		for (int y = Pos.y + 32; y < Pos.y + 256; y += 4)
		{
			To.y = y;
			if(!(GameServer()->Collision()->GetCollisionAt(x, y)&CCollision::COLFLAG_NOGO))
				break;
		}
				
		// add laser wall
		CStaticlaser *L = new CStaticlaser(&GameServer()->m_World, From, To);
		m_apLaserwall[m_LaserwallCount++] = L;
	}	
	
	
	// x+
	for (int x = Pos.x + 32; x < Pos.x + 512; x += StepSize)
	{
		if(!(GameServer()->Collision()->GetCollisionAt(x, Pos.y)&CCollision::COLFLAG_NOGO))
		{
			if (GameServer()->Collision()->GetCollisionAt(x, Pos.y) <= 0)
			{
				// edge found, step back a bit
				x -= StepSize*2;

				vec2 From = vec2(x, Pos.y);
				vec2 To = From;
				
				// find start and end positions for laser wall
				for (int y = Pos.y; y > Pos.y - 256; y -= StepSize)
				{
					From.y = y;
					if(!(GameServer()->Collision()->GetCollisionAt(x, y)&CCollision::COLFLAG_NOGO))
						break;
				}
				
				for (int y = Pos.y; y < Pos.y + 256; y += StepSize)
				{
					To.y = y;
					if(!(GameServer()->Collision()->GetCollisionAt(x, y)&CCollision::COLFLAG_NOGO))
						break;
				}
				
				// add laser wall
				CStaticlaser *L = new CStaticlaser(&GameServer()->m_World, From, To);
				m_apLaserwall[m_LaserwallCount++] = L;
			}
			
			break;
		}
	}
}


bool CGameControllerKillingFloor::CheckLose()
{
	int Players = 0;
	int Inside = 0;
	
	for (int c = 0; c < FIRST_BOT_ID; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;
		
		if (pPlayer->GetTeam() != TEAM_SPECTATORS)
			Players++;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;

		if (InsideLosepoint(pCharacter->m_Pos))
		{
			Inside++;
			pCharacter->Cry();
		}
	}
	
	if (m_GameState != GAMESTATE_ROUND)
	{
		return false;
	}
	
	if (Inside > 0 && m_LosezoneWhine <= 0 && Inside < Players)
	{
		m_LosezoneWhine = 600;
		
		char aBuf[128];
		if (Inside == 1)
			str_format(aBuf, sizeof(aBuf), "%d player inside lose zone", Inside);
		else
			str_format(aBuf, sizeof(aBuf), "%d players inside lose zone", Inside);
			
		GameServer()->SendBroadcast(aBuf, -1);
	}
	
	if (Inside == Players)
		return true;
	
	return false;
}
	
	
bool CGameControllerKillingFloor::OnEntity(int Index, vec2 Pos)
{
	if(IGameController::OnNonPickupEntity(Index, Pos))
		return true;

	if (!m_DroppablesCreated)
		CreateDroppables();
	
	// pickup part of IGameController::OnEntity
	int Type = -1;
	int SubType = 0;
	bool Dropable = false;
	
	if(Index == ENTITY_ARMOR_1)
	{
		Type = POWERUP_ARMOR;
		SubType = 0;
	}
	else if(Index == ENTITY_HEALTH_1)
	{
		Type = POWERUP_HEALTH;
		SubType = 0;
	}
	
	/*

	else if(Index == ENTITY_WEAPON_SHOTGUN)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_SHOTGUN;
	}
	else if(Index == ENTITY_WEAPON_GRENADE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_GRENADE;
	}
	else if(Index == ENTITY_WEAPON_RIFLE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_RIFLE;
	}
	else if(Index == ENTITY_POWERUP_NINJA && g_Config.m_SvPowerups)
	{
		Type = POWERUP_NINJA;
		SubType = WEAPON_NINJA;
	}
	*/

	// pickups & droppables
	if(Type != -1)
	{
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, Type, SubType);
		m_apPickup[m_PickupCount]->m_Pos = Pos;
		m_apPickup[m_PickupCount]->m_Dropable = Dropable;
		m_PickupCount++;
		return true;
	}
	
	
	// lose points for lose condition
	if(Index == ENTITY_LOSEPOINT)
	{
		m_apLosepoint[m_LosepointCount] = new CLosepoint(&GameServer()->m_World);
		m_apLosepoint[m_LosepointCount]->m_Pos = Pos;
		m_apLosepoint[m_LosepointCount]->UpdateRange();
		m_LosepointCount++;
		return true;
	}
	
	// laserwalls for show
	if(Index == ENTITY_LASERWALL)
	{
		AddLaserwall(Pos);
		return true;
	}
	
		
	// teleports
	int TeleportType = -1;
	if(Index == ENTITY_TELEPORT_IN) TeleportType = TELEPORT_IN;
	if(Index == ENTITY_TELEPORT_OUT) TeleportType = TELEPORT_OUT;
	
	if(TeleportType == -1)
		return false;

	for (int i = 0; i < MAX_TELEPORTS; i++)
	{
		if (!m_apTeleport[i])
		{
			CTeleport *F = new CTeleport(&GameServer()->m_World, TeleportType);
			F->m_Pos = Pos;
			m_apTeleport[i] = F;
			GameServer()->m_World.InsertEntity(F);
			return true;
		}
	}
	
	return false;
}


void CGameControllerKillingFloor::CreateDroppables()
{
	for (int i = 0; i < MAX_DROPPABLES; i++)
	{
		// hearts
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_HEALTH, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;

		// armors
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_ARMOR, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
	}
	
	m_DroppablesCreated = true;
}
	
	
int CGameControllerKillingFloor::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID)
{
	//IGameController::OnCharacterDeath(pVictim, pKiller, WeaponID);
	
	if (pVictim->GetPlayer()->GetCID() < FIRST_BOT_ID)
	{
		// on player's death
		pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*5.0f;
	}
	else
	// on zombie's death
	{
		// boss death
		
		if (m_BossHead)
		{
			if (m_BossHead == pVictim)
			{
				m_ZombiesLeft = 0;
				GameServer()->SendBroadcast("Boss defeated!", -1);
			}
		}
		
		
		// when player gets the kill
		if (pKiller && pKiller->GetCID() < FIRST_BOT_ID)
		{
			if (pVictim->GetPlayer()->m_pAI)
				pKiller->m_Score += pVictim->GetPlayer()->m_pAI->m_Reward;
			
			// random drop
			if (frandom()*11 < 5 - GameServer()->Difficulty())
				DropPickup(pVictim->m_Pos, POWERUP_HEALTH);
			else if (frandom()*11 < 5 - GameServer()->Difficulty())
				DropPickup(pVictim->m_Pos, POWERUP_ARMOR);
			//else if (frandom()*11 < 4)
			//	DropPickup(pVictim->m_Pos, POWERUP_WEAPON);
		}
	
		pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*1.0f;
	}
	
	return 0;
}



CAI *CGameControllerKillingFloor::GetNextEnemyAI(class CCharacter *pChr)
{
	CAI *AI = NULL;
	
	
	// boss wave
	if (m_Round == m_MaxRounds)
	{
		// create body for the boss
		if (m_RoundZombieCount == 0)
		{
			AI = new CAIBossbody(GameServer(), pChr->GetPlayer());
			m_BossBody = pChr;
		}
		else
		// create head for boss
		if (m_RoundZombieCount == 1)
		{
			AI = new CAIBosshead(GameServer(), pChr->GetPlayer());
			m_BossHead = pChr;
			pChr->m_Bottom = m_BossBody;
			m_BossBody->m_Top = m_BossHead;
		}
		else
		{
			AI = new CAIRunner(GameServer(), pChr->GetPlayer());
		}
		
		m_RoundZombieCount++;
		
		return AI;
	}
	
	

	
	switch (m_Round)
	{
	case 1:
		if (m_RoundZombieCount < 12)
			AI = new CAIRunner(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIPistolbot(GameServer(), pChr->GetPlayer());
		break;
	
	case 2:
		if (m_RoundZombieCount < 12)
			AI = new CAIBomber(GameServer(), pChr->GetPlayer());
		else
		if (m_RoundZombieCount%2 == 1)
			AI = new CAIRunner(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIPistolbot(GameServer(), pChr->GetPlayer());
		break;
	
	case 3:
		if (m_RoundZombieCount%4 == 1)
			AI = new CAITank(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 2)
			AI = new CAIShotgunner(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIRunner(GameServer(), pChr->GetPlayer());
		break;
	
	case 4:
		if (m_RoundZombieCount < 12)
			AI = new CAINinja(GameServer(), pChr->GetPlayer());
		
		if (m_RoundZombieCount%4 == 1)
			AI = new CAIFlybot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 2)
			AI = new CAIShotgunner(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 3)
			AI = new CAITank(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIElectro(GameServer(), pChr->GetPlayer());
		break;
		
	case 5:
		if (m_RoundZombieCount < 16)
			AI = new CAISpiderbomber(GameServer(), pChr->GetPlayer());
			
		else if (m_RoundZombieCount%4 == 1)
			AI = new CAIFlybot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 2)
			AI = new CAILightningbot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 3)
			AI = new CAIPistolbot(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIGrabber(GameServer(), pChr->GetPlayer());
		break;
	
	case 6:
		if (m_RoundZombieCount < 16)
			AI = new CAIElectro(GameServer(), pChr->GetPlayer());
		
		else if (m_RoundZombieCount%5 == 1)
			AI = new CAILightningbot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%5 == 2)
			AI = new CAILaserbot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%5 == 3)
			AI = new CAIGrabber(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%5 == 4)
			AI = new CAINinja(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIMadbomber(GameServer(), pChr->GetPlayer());
		break;
	
	// skipped
	case 7:
		if (m_RoundZombieCount%4 == 1)
			AI = new CAILightningbot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 2)
			AI = new CAILaserbot(GameServer(), pChr->GetPlayer());
		else if (m_RoundZombieCount%4 == 3)
			AI = new CAIGrabber(GameServer(), pChr->GetPlayer());
		else
			AI = new CAIMadbomber(GameServer(), pChr->GetPlayer());
		break;
			
	};
	
	m_RoundZombieCount++;

	return AI;
}


void CGameControllerKillingFloor::OnCharacterSpawn(CCharacter *pChr)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (pChr->GetPlayer()->GetCID() >= FIRST_BOT_ID)
	{
		if (pChr->GetPlayer()->m_pAI)
			delete pChr->GetPlayer()->m_pAI;
			
		//pChr->GetPlayer()->m_pAI = new CAIRunner(GameServer(), pChr->GetPlayer());
		pChr->GetPlayer()->m_pAI = GetNextEnemyAI(pChr);
	}
}

bool CGameControllerKillingFloor::CanCharacterSpawn(int ClientID)
{
	if (ClientID < FIRST_BOT_ID)
	{
		return true;
	}
	
	if (m_GameState == GAMESTATE_ROUND && m_ZombiesLeft > 0)
	{
		// infinite zombies on boss round
		if (m_Round != m_MaxRounds)
			m_ZombiesLeft--;
		return true;
	}

	return false;
}

void CGameControllerKillingFloor::DoWincheck()
{

}

bool CGameControllerKillingFloor::CanBeMovedOnBalance(int ClientID)
{
	return false;
}

void CGameControllerKillingFloor::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;
}



void CGameControllerKillingFloor::ClearPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Hide();
	}
}

void CGameControllerKillingFloor::RespawnPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Respawn();
	}
}

void CGameControllerKillingFloor::FlashPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && !m_apPickup[i]->m_Dropable && m_apPickup[i]->m_SpawnTick <= 0)
			m_apPickup[i]->m_Flashing = true;
	}
}


int CGameControllerKillingFloor::UpdateZombieScores()
{
	int NumZombies = m_ZombiesLeft;
	
	for (int i = FIRST_BOT_ID; i < LAST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
			
		if (!pCharacter->IsAlive())
			continue;
		
		NumZombies++;
	}
	
	for (int i = FIRST_BOT_ID; i < LAST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		pPlayer->m_Score = NumZombies;
	}
	
	return NumZombies;
}


CTeleport *CGameControllerKillingFloor::GetClosestTeleport(vec2 Pos, int Type)
{
	CTeleport *pClosestTeleport = NULL;
	
	for (int f = 0; f < MAX_TELEPORTS; f++)
	{
		if (m_apTeleport[f])
		{
			if (m_apTeleport[f]->m_Type != Type && Type != -1)
				continue;
			
			if (!pClosestTeleport)
			{
				pClosestTeleport = m_apTeleport[f];
			}
			else
			{
				if (abs(m_apTeleport[f]->m_Pos.x - Pos.x) + abs(m_apTeleport[f]->m_Pos.y - Pos.y) < abs(pClosestTeleport->m_Pos.x - Pos.x) + abs(pClosestTeleport->m_Pos.y - Pos.y))
				{
					pClosestTeleport = m_apTeleport[f];
				}
			}
		}
	}
	
	return pClosestTeleport;
}



CTeleport *CGameControllerKillingFloor::GetTeleport(int Type)
{	
	for (int f = 0; f < MAX_TELEPORTS; f++)
	{
		m_TeleportCounter++;
		if (m_TeleportCounter >= MAX_TELEPORTS)
			m_TeleportCounter = 0;
		
		if (m_apTeleport[m_TeleportCounter])
		{
			if (m_apTeleport[m_TeleportCounter]->m_Type == Type || Type != -1)
				return m_apTeleport[m_TeleportCounter];
		}
	}
	
	return NULL;
}


int CGameControllerKillingFloor::CountPlayers()
{
	int NumPlayers = 0;
	
	for (int i = 0; i < FIRST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		NumPlayers++;
	}
	
	return NumPlayers;
}


int CGameControllerKillingFloor::CountPlayersAlive()
{
	int NumPlayersAlive = 0;
	
	for (int i = 0; i < FIRST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
			
		if (!pCharacter->IsAlive())
			continue;
		
		NumPlayersAlive++;
	}
		
	return NumPlayersAlive;
}



void CGameControllerKillingFloor::Restart()
{
	m_GameState = GAMESTATE_STARTING;
	m_RoundTick = 0;
	m_Round = 0;
	//m_Round = m_MaxRounds-1; // skip to boss
	m_ZombiesLeft = 0;
	m_TeleportCounter = 0;
	
	m_LosezoneWhine = 0;

	//m_Boss = NULL;
	
	GameServer()->SendBroadcast("", -1);
	
	// "reset" players
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_Head[i] = NULL;
	
	m_BossBody = NULL;
	m_BossHead = NULL;
	
	
	
	switch (GameServer()->Difficulty())
	{
		case NORMAL:
		GameServer()->SendBroadcast("Difficulty: Normal", -1);
		break;
		
		case HARD:
		GameServer()->SendBroadcast("Difficulty: Hard", -1);
		break;
		
		case SUICIDAL:
		GameServer()->SendBroadcast("Difficulty: Suicidal", -1);
		break;
	
		default:
		GameServer()->m_Difficulty = NORMAL;
		GameServer()->SendBroadcast("Difficulty: Normal", -1);
		break;
	};
	
	for (int i = 0; i < FIRST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
	
		pPlayer->m_Respawns = 0;
		pPlayer->m_Class = -1;
		pPlayer->m_ClassPickWhine = 400;
		pPlayer->m_Score = 0;
	}
	
	KillZombies();
	UpdateZombieScores();
}


void CGameControllerKillingFloor::KillZombies()
{
	for (int i = FIRST_BOT_ID; i < LAST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
		
		pCharacter->Reset();
	}
}


void CGameControllerKillingFloor::LoseRound()
{
	m_GameState = GAMESTATE_LOSE;
	m_RoundTick = 0;
	
	//GameServer()->SendBroadcast("Defeat", -1);
	
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Defeated on wave %d", m_Round);
		
	GameServer()->SendBroadcast(aBuf, -1);
}

void CGameControllerKillingFloor::WaveCleanup()
{
	m_GameState = GAMESTATE_STARTING;
	m_RoundTick = 0;
	
	GameServer()->SendBroadcast("", -1);
	RespawnPickups();
	
	// prepare for next map / game
	if (m_Round >= m_MaxRounds)
	{
		//EndRound();
		m_GameState = GAMESTATE_WIN;
		GameServer()->SendBroadcast("Map cleared!", -1);
	}
	
	for (int i = 0; i < FIRST_BOT_ID; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
	
		pPlayer->m_Score += 50;
		
		// add grenades to commando
		/*
		if (pPlayer->GetCharacter() && pPlayer->m_Class == CLASS_COMMANDO)
		{
			//pPlayer->GetCharacter()->m_Grenades += 2;
			pPlayer->GetCharacter()->AddGrenades(2);
		}
		*/
	}
	
}

void CGameControllerKillingFloor::StartRound()
{	
	ClearPickups();

	m_GameState = GAMESTATE_ROUND;
	m_RoundTick = 0;
	m_Round++;
	
	m_ZombiesLeft = 24 + m_Round * 6;
	m_RoundZombieCount = 0;
	
	if (!m_ZombiesCreated)
	{
		m_ZombiesCreated = true;
		GameServer()->CreateZombies();
	}
	
	
	char aBuf[128];
	
	// boss round
	if (m_Round == m_MaxRounds)
	{
		//str_format(aBuf, sizeof(aBuf), "Boss wave - Difficulty: %s", aDifficulty[GameServer()->Difficulty()]);
		str_format(aBuf, sizeof(aBuf), "Boss wave");
		m_ZombiesLeft = 1;
	}
	else
	{
		//str_format(aBuf, sizeof(aBuf), "Wave %d / %d - Difficulty: %s", m_Round, m_MaxRounds, aDifficulty[GameServer()->Difficulty()]);
		str_format(aBuf, sizeof(aBuf), "Wave %d / %d", m_Round, m_MaxRounds);
	}
		
	if (m_Round == 2)
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "Check www.ninslash.com for client & updates");
		
		
	GameServer()->SendBroadcast(aBuf, -1);
	//GameServer()->StartVote("Test vote", "/test", "For teh lulz");
}


// check if laser walls are released
void CGameControllerKillingFloor::ResetLaserwalls()
{
	for (int i = 0; i < m_LaserwallCount; i++)
	{
		m_apLaserwall[i]->m_Hidden = false;
	}
}






// check if laser walls are released
void CGameControllerKillingFloor::ReleaseLaserwalls(vec2 Pos)
{
	if (GameServer()->Collision()->GetCollisionAt(Pos.x, Pos.y)&CCollision::COLFLAG_RELEASENOGO)
	{
		for (int i = 0; i < m_LaserwallCount; i++)
		{
			if (abs(m_apLaserwall[i]->m_Pos.x - Pos.x) < 700 && abs(m_apLaserwall[i]->m_Pos.y - Pos.y) < 700)
				m_apLaserwall[i]->m_Hidden = true;
		}
	}
}





// check if laser walls are released
bool CGameControllerKillingFloor::ForceNogo(vec2 Pos)
{
	bool Force = true;

	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		vec2 CharacterPos = pCharacter->m_Pos;
		
		if (abs(CharacterPos.x - Pos.x) > 700 || abs(CharacterPos.y - Pos.y) > 700)
			continue;
		
		if (GameServer()->Collision()->GetCollisionAt(CharacterPos.x, CharacterPos.y)&CCollision::COLFLAG_RELEASENOGO)
		{
			Force = false;
			if (m_LastReleaseSound-- <= 0)
			{
				m_LastReleaseSound = 10;
				//GameServer()->CreateSound(CharacterPos, SOUND_BODY_LAND);
			}
		}
	}
	
	return Force;
}




void CGameControllerKillingFloor::Tick()
{
	IGameController::Tick();
	
	if (m_LosezoneWhine > 0)
		m_LosezoneWhine--;

	if(GameServer()->m_World.m_ResetRequested || GameServer()->m_World.m_Paused)
		return;	
	
	ResetLaserwalls();
	
	if (CountPlayers() == 0)
	{
		if (m_Round != 0)
			Restart();
		
		m_RoundTick = 0;
	}

	// win or lose
	if (m_GameState == GAMESTATE_LOSE || m_GameState == GAMESTATE_WIN)
	{
		m_RoundTick++;		
							
		if (m_RoundTick == 300)
			GameServer()->SendBroadcast("New game starting in 4...", -1);
		if (m_RoundTick == 400)
			GameServer()->SendBroadcast("New game starting in 3...", -1);
		if (m_RoundTick == 500)
			GameServer()->SendBroadcast("New game starting in 2...", -1);
		if (m_RoundTick == 600)
			GameServer()->SendBroadcast("New game starting in 1...", -1);
		if (m_RoundTick == 700)
		{
			Restart();
			EndRound();
		}
	}	
	
	if (m_GameState == GAMESTATE_STARTING)
	{
		m_RoundTick++;		
							
		if (m_RoundTick == 600)
			GameServer()->SendBroadcast("Wave incoming in 4...", -1);
		if (m_RoundTick == 700)
			GameServer()->SendBroadcast("Wave incoming in 3...", -1);
		if (m_RoundTick == 800)
			GameServer()->SendBroadcast("Wave incoming in 2...", -1);
		if (m_RoundTick == 900)
		{
			GameServer()->SendBroadcast("Wave incoming in 1...", -1);
			FlashPickups();
		}
		if (m_RoundTick == 1000)
			StartRound();
	}
	
	if (m_GameState == GAMESTATE_ROUND)
	{
		GameServer()->UpdateAI();
		if (UpdateZombieScores() <= 0)
		{
			m_RoundTick++;
				
			if (m_RoundTick == 50)
				GameServer()->SendBroadcast("Wave complete!", -1);
				
			if (m_RoundTick == 300)
			{
				WaveCleanup();
			}
		}
		else
		{
			m_RoundTick = 0;
			
			/*
			if (CheckLose())
			{
				LoseRound();
			}
			*/
		}
	}
	
	// also handles tee crying sound
	if (CheckLose() && m_GameState == GAMESTATE_ROUND)
	{
		LoseRound();
	}
	
	//bool Nogo = ForceNogo();
	
	
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;
			
		// remind player to pick a class once in a while
		if (pPlayer->m_Class < 0 && c < FIRST_BOT_ID)
		{
			if (pPlayer->m_ClassPickWhine-- < 0)
			{
				GameServer()->SendChatTarget(c, "Pick a class (write the wanted class to chat):");
				//GameServer()->SendChatTarget(c, "commando - throw grenades - start weapon: Assault Rifle");
				GameServer()->SendChatTarget(c, "commando - create clips - start weapon: Assault Rifle");
				GameServer()->SendChatTarget(c, "medic - heal - start weapon: Double barrel shotgun");
				GameServer()->SendChatTarget(c, "pioneer - land mines - start weapon: Rocket launcher");
				GameServer()->SendChatTarget(c, "engineer - electro mines - start weapon: Laser rifle");
				GameServer()->SendChatTarget(c, "berserker - bloodlust - start weapon: Katana");

				pPlayer->m_ClassPickWhine = 2000;
			}
		}
		
		
		

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
	
		ReleaseLaserwalls(pCharacter->m_Pos);
	
		// force nogo zones
		if (ForceNogo(pCharacter->m_Pos))
			pCharacter->ForceNogo();
	
		CTeleport *pClosestTeleport = GetClosestTeleport(pCharacter->m_Pos, TELEPORT_IN);
		
		if (pClosestTeleport)
		{
			// check is a teleport close enough to a player for teleporting
			if (abs(pClosestTeleport->m_Pos.x-pCharacter->m_Pos.x) < 40 && abs(pClosestTeleport->m_Pos.y-pCharacter->m_Pos.y) < 30)
			{
				CTeleport *pTeleport = GetTeleport(TELEPORT_OUT);
				
				if (pTeleport)
				{
					
					GameServer()->CreateSound(pCharacter->m_Pos, SOUND_WEAPON_SPAWN);
					GameServer()->CreatePlayerSpawn(pCharacter->m_Pos);
					//GameServer()->CreatePlayerSpawn(pTeleport->m_Pos);
				
					pCharacter->Teleport(pTeleport->m_Pos-vec2(0, 14));
					GameServer()->CreatePlayerSpawn(pCharacter->m_Pos);
					GameServer()->CreateSound(pTeleport->m_Pos, SOUND_WEAPON_SPAWN);
				}
			}
		}
	}
	
}











