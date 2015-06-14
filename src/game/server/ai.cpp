#include "ai.h"

#include "entities/character.h"
#include "entities/flag.h"
#include <game/server/player.h>
#include "gamecontext.h"


CAI::CAI(CGameContext *pGameServer, CPlayer *pPlayer)
{
	m_pGameServer = pGameServer;
	m_pPlayer = pPlayer;
	m_Reward = 5;	
	Reset();
}


void CAI::Reset()
{
	m_Sleep = 0;
	m_Stun = 0;
	m_ReactionTime = 20;
	m_NextReaction = m_ReactionTime;
	m_InputChanged = true;
	m_Move = 0;
	m_LastMove = 0;
	m_Jump = 0;
	m_LastJump = 0;
	m_LastAttack = 0;
	m_Hook = 0;
	m_LastHook = 0;
	m_LastPos = vec2(-9000, -9000);
	m_Direction = vec2(-1, 0);
	
	m_PlayerPos = vec2(0, 0);
	m_TargetPos = vec2(0, 0);
	m_PlayerSpotTimer = 0;
	m_PlayerSpotCount = 0;
		
	m_BombTimer = 0;
	m_BombSize = 0;
}


void CAI::Zzz(int Time)
{
	if (!Player()->GetCharacter())
		return;
		
	if (m_Sleep < Time)
	{
		m_Sleep = Time;
		Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, Time*17, Time*17, true);
		GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_ZZZ);
	}
}


void CAI::Stun(int Time)
{
	if (!Player()->GetCharacter())
		return;

	if (Player()->GetCharacter()->m_Top || Player()->GetCharacter()->m_Bottom)
		Time /= 3;
	
	if (m_Stun < Time)
	{
		m_Stun = Time;
		Player()->GetCharacter()->SetEmoteFor(EMOTE_SURPRISE, Time*17, Time*17, true);
	}
}


void CAI::StandStill(int Time)
{
	m_Sleep = Time;
}


void CAI::UpdateInput(int *Data)
{
	m_InputChanged = false;
	Data[0] = m_Move;
	Data[1] = m_Direction.x;
	Data[2] = m_Direction.y;
	Data[3] = m_Jump;
	Data[4] = m_Attack;
	Data[5] = m_Hook;
}




void CAI::JumpIfPlayerIsAbove()
{
		if (abs(m_PlayerPos.x - m_Pos.x) < 100 && m_Pos.y > m_PlayerPos.y + 100)
		{
			if (frandom() * 10 < 4)
				m_Jump = 1;
		}
}

void CAI::HeadToMovingDirection()
{
	if (m_Move != 0)
		m_Direction = vec2(m_Move, 0);
}

void CAI::Unstuck()
{
	if (abs(m_Pos.x - m_LastPos.x) < 14 && Player()->GetCharacter()->IsGrounded())
	{
		if (frandom() * 10 < 3)
			m_Move = -1;
		if (frandom() * 10 < 3)
			m_Move = 1;
		if (frandom() * 10 < 3)
			m_Jump = 1;
	}
}



bool CAI::SeekFlag()
{
	CFlag *apFlag[1];
	int Num = GameServer()->m_World.FindEntities(m_LastPos, 999999, (CEntity**)apFlag, 1, CGameWorld::ENTTYPE_FLAG);
	
	if (apFlag[0])
	{
		if (abs(m_LastPos.x - apFlag[0]->m_Pos.x) < 200)
			m_Move = 0;
		else if (m_LastPos.x < apFlag[0]->m_Pos.x)
			m_Move = 1;
		else
			m_Move = -1;
		
		return true;
	}
	
	return false;
}


bool CAI::MoveTowardsPlayer(int Dist)
{
	if (abs(m_LastPos.x - m_PlayerPos.x) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_PlayerPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_PlayerPos.x)
		m_Move = -1;
		
	return false;
}


bool CAI::MoveTowardsTarget(int Dist)
{

	if (abs(m_LastPos.x - m_TargetPos.x) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_TargetPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_TargetPos.x)
		m_Move = -1;
		
	return false;
}


void CAI::ReceiveDamage()
{
	
}


bool CAI::SeekPlayer()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
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
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 800 && 
			!GameServer()->Collision()->IntersectLine(pCharacter->m_Pos, m_LastPos, NULL, NULL))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (pClosestCharacter)
	{
		m_PlayerSpotCount++;
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	
	m_PlayerSpotCount = 0;
	return false;
}



void CAI::CheckAITiles()
{
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y)&CCollision::COLFLAG_AILEFT)
		m_Move = -1;
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y)&CCollision::COLFLAG_AIRIGHT)
		m_Move = 1;
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y)&CCollision::COLFLAG_AIUP)
		m_Jump = 1;
}



void CAI::Tick()
{
	m_NextReaction--;
	
	// character check & position update
	if (m_pPlayer->GetCharacter())
		m_Pos = m_pPlayer->GetCharacter()->m_Pos;
	else
		return;
	
	
	// suicide bombers
	if (m_BombTimer > 0)
	{
		if (--m_BombTimer <= 0)
		{
			Player()->GetCharacter()->Explode(m_BombSize);
		}
	}
	
	
	// skip if sleeping or stunned
	if (m_Sleep > 0 || m_Stun > 0)
	{
		if (m_Sleep > 0)
			m_Sleep--;
			
		if (m_Stun > 0)
			m_Stun--;
		
		m_Move = 0;
		m_Jump = 0;
		m_Hook = 0;
		m_Attack = 0;
		m_InputChanged = true;
		
		return;
	}
	
	// stupid AI can't even react to events every frame
	if (m_NextReaction <= 0)
	{
		m_NextReaction = m_ReactionTime;
	
		DoBehavior();
		
		if (m_pPlayer->GetCharacter())
			m_LastPos = m_pPlayer->GetCharacter()->m_Pos;
		m_LastMove = m_Move;
		m_LastJump = m_Jump;
		m_LastAttack = m_Attack;
		m_LastHook = m_Hook;
			
		m_InputChanged = true;
	}
	else
	{
		m_Attack = 0;
	}
}