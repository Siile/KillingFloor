#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "bossbody.h"


CAIBossbody::CAIBossbody(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIBossbody::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(9999);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "toptri", 64);
}


void CAIBossbody::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	CheckAITiles();
	SeekPlayer();

	
	if (m_PlayerSpotCount > 0)
	{
		if (m_PlayerSpotCount == 1)
			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);
		
		MoveTowardsPlayer(BotAttackRange[Player()->GetCharacter()->m_ActiveCustomWeapon] / 2);
		JumpIfPlayerIsAbove();
		
		if (m_PlayerDistance < BotAttackRange[Player()->GetCharacter()->m_ActiveCustomWeapon])
			m_Attack = 1;
		
		m_Direction = m_PlayerDirection;
	}
	else
		HeadToMovingDirection();

	Unstuck();
	Unstuck();
	Unstuck();
	Unstuck();
	
	// next reaction in
	m_ReactionTime = 5;
	
}