#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "flybot.h"


CAIFlybot::CAIFlybot(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIFlybot::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTFLYHAMMER);
	pChr->SetHealth(12+GameServer()->Difficulty()*3);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "coala", 64);
	
	m_Reward = 15;
}


void CAIFlybot::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	CheckAITiles();
	SeekPlayer();
	
	if (m_PlayerSpotCount == 0)	
		m_Hook = 0;
	
	Unstuck();

	// hook moving (sucks)
	vec2 HookPos = m_Pos - vec2(0, 300);
			
	if (m_Move < 0)
		HookPos.x -= 250;
	else
		HookPos.x += 250;
			
				
	// hook if something in sight
	if (GameServer()->Collision()->IntersectLine(m_Pos, HookPos, NULL, NULL) && 
		m_LastHook == 0 && m_PlayerSpotCount == 0 && frandom()*10 < 4)
	{
		m_Hook = 1;
		m_Direction = HookPos - m_Pos;
		if (Player()->GetCharacter()->IsGrounded())
			m_Jump = 1;
	}
	else
		HeadToMovingDirection();
	
	
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


		if (m_PlayerPos.x < m_Pos.x)
			m_TargetPos.x = m_PlayerPos.x + 120;
		else
			m_TargetPos.x = m_PlayerPos.x - 120;
		
		MoveTowardsTarget(50);
		
		if (m_PlayerDistance < 400)
		{
			m_Attack = 1;
			m_Hook = 0;
		}
		
		
		if (m_PlayerDistance < 700)
			m_Direction = m_PlayerDirection;
	}

	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}