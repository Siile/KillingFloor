#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "ninja.h"


CAINinja::CAINinja(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAINinja::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(12+GameServer()->Difficulty()*3);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "saddo", 64);
}


void CAINinja::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	CheckAITiles();
	SeekPlayer();

	
	if (m_PlayerSpotCount > 0)
	{
		MoveTowardsPlayer(40);
		JumpIfPlayerIsAbove();
		
		m_Direction = m_PlayerDirection;
		
	
		if (abs(m_PlayerPos.x - m_Pos.x) < 140 && m_Pos.y > m_PlayerPos.y + 100)
		{
			Player()->GetCharacter()->SetCustomWeapon(SWORD_KATANA);
			m_Direction = m_PlayerDirection;
			m_Attack = 1;
		}
		else
			Player()->GetCharacter()->SetCustomWeapon(HAMMER_BOTBASIC);
		
		if (m_PlayerDistance < 120)
			m_Attack = 1;
		
		if (m_Pos.y < m_PlayerPos.y)
			m_Jump = 0;
	}
	else
	{
		HeadToMovingDirection();
	}
	
	Unstuck();
	
	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}