#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/entities/lightning.h>
#include <game/server/player.h>

#include "electro.h"

#define RAD 0.017453292519943295769236907684886f

CAIElectro::CAIElectro(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIElectro::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(14+GameServer()->Difficulty()*3);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "bluekitty", 64);	
	m_LightningTimer = 0;
	
	m_Reward = 15;
}


void CAIElectro::DoBehavior()
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

			Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
			m_ReactionTime = 40;
			return;
		}

		MoveTowardsPlayer(40);
		m_Direction = m_PlayerDirection;
		
		if (m_PlayerDistance < 120)
			m_Attack = 1;
			
		if (m_PlayerDistance < 600)
		{
			float Angle = frandom()*360.0f*RAD;
			new CLightning(Player()->GetCharacter()->GameWorld(), Pos, vec2(cosf(Angle), sinf(Angle)), 50, 50, Player()->GetCID(), 1);
			Angle = frandom()*360.0f*RAD;
			new CLightning(Player()->GetCharacter()->GameWorld(), Pos, vec2(cosf(Angle), sinf(Angle)), 50, 50, Player()->GetCID(), 1);
		}
		
		
		JumpIfPlayerIsAbove();
			
		if (Pos.y < m_PlayerPos.y)
			m_Jump = 0;
	}
	
	Unstuck();
	HeadToMovingDirection();
	
	m_ReactionTime = 5;
	
}