#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "runner.h"


CAIRunner::CAIRunner(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIRunner::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(5+GameServer()->Difficulty());
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "saddo", 64);
	
}


void CAIRunner::ReceiveDamage()
{
	if (m_PlayerSpotCount == 0)
	{
		// something maybe
		
	}
}


void CAIRunner::DoBehavior()
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
	
	

	
	if (m_PlayerSpotCount > 0)
	{
		m_TargetTimer = 0;
	
		// on first time standstill and look stupid
		if (m_PlayerSpotCount == 1)
		{
			m_Move = 0;
			m_Jump = 0;
			m_Direction = m_PlayerDirection;

			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);
			//GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_QUESTION);
			m_ReactionTime = 40;
			return;
		}

		MoveTowardsPlayer(40);
		m_Direction = m_PlayerDirection;
		
		if (m_PlayerDistance < 120)
			m_Attack = 1;
		
		// if the player is above bot
		if (abs(m_PlayerPos.x - Pos.x) < 100 && Pos.y > m_PlayerPos.y + 100)
		{
			if (frandom() * 10 < 3)
				m_Jump = 1;
		}

			
		if (Pos.y < m_PlayerPos.y)
			m_Jump = 0;
	}
	
	// unstuck move
	if (abs(Pos.x - m_LastPos.x) < 14 && Player()->GetCharacter()->IsGrounded())
	{
		if (frandom() * 10 < 3)
			m_Move = -1;
		if (frandom() * 10 < 3)
			m_Move = 1;
		if (frandom() * 10 < 2)
			m_Jump = 1;
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
	
	
	// head to moving direction
	if (m_Move != 0)
		m_Direction = vec2(m_Move, 0);
	

	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}