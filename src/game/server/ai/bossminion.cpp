#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "bossminion.h"


CAIBossminion::CAIBossminion(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIBossminion::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTFLYHAMMER);
	pChr->SetHealth(10);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "twintri", 64);
}




void CAIBossminion::DoBehavior()
{
	m_Jump = 0;
	m_Attack = 0;
	m_Hook = 0;
	
	CheckAITiles();
	SeekPlayer();
		
	
	if (m_PlayerSpotCount > 0)
	{
		m_TargetTimer = 0;
	

		if (m_PlayerPos.x < m_Pos.x)
			m_TargetPos.x = m_PlayerPos.x + 120;
		else
			m_TargetPos.x = m_PlayerPos.x - 120;
		
		MoveTowardsTarget(50);
		
		if (m_PlayerDistance < 500)
			m_Attack = 1;
		
		
		m_Direction = m_PlayerDirection;
	}
	else
		HeadToMovingDirection();
	
	Unstuck();

	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
}