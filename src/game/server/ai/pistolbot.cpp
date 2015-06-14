#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "pistolbot.h"


CAIPistolbot::CAIPistolbot(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIPistolbot::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(GUN_BOTPISTOL);
	pChr->SetHealth(3+GameServer()->Difficulty()*2);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "saddo", 64);
	
}


void CAIPistolbot::DoBehavior()
{
	vec2 Pos;

	// character check
	if (Player()->GetCharacter())
		Pos = Player()->GetCharacter()->m_Pos;
	else
		return;

	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	CheckAITiles();
	SeekPlayer();
	
	// unstuck move
	if (abs(Pos.x - m_LastPos.x) < 20)
	{
		if (frandom() * 10 < 3)
			m_Move = -1;
		if (frandom() * 10 < 3)
			m_Move = 1;
		
		if (frandom() * 10 < 2)
			m_Jump = 1;
	}

	
	if (m_PlayerSpotCount > 0)
	{
		m_TargetTimer = 0;
	
		// on first time standstill and look stupid
		if (m_PlayerSpotCount == 1)
		{
			m_Move = 0;
			m_Direction = m_PlayerDirection;
			m_TargetPos = m_PlayerPos;
			m_Jump = 0;

			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);
			m_ReactionTime = 40;
			return;
		}

		if (m_PlayerPos.x < Pos.x)
			m_TargetPos.x = m_PlayerPos.x + 400;
		else
			m_TargetPos.x = m_PlayerPos.x - 400;
		
		MoveTowardsTarget(140);
		
		if (m_PlayerDistance < 600)
			m_Attack = 1;
		
		
		if (m_PlayerDistance < 800)
		{
			m_Direction = m_PlayerDirection;
		}
		
		// start timer
		if (m_PlayerSpotTimer == 0)
			m_PlayerSpotTimer++;
	}
	else
	{
		// head to moving direction
		if (m_Move != 0)
			m_Direction = vec2(m_Move, 0);
	}
	

	
	
	/*
	// forget target after a while
	if (m_TargetTimer > 0)
	{
		MoveTowardsTarget();
		
		if (m_TargetTimer > 20)
		{
			m_TargetTimer = 0;
			m_Move = 0;
			m_Jump = 0;
			m_Attack = 0;
			m_ReactionTime = 60 + frandom()*3;
			
			Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200);
			GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_ZZZ);
			return;
		}
	}
	*/
	
	
	

	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}