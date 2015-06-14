#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "grabber.h"


CAIGrabber::CAIGrabber(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIGrabber::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(16+GameServer()->Difficulty()*3);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "brownbear", 64);
	
	m_Reward = 10;
}


void CAIGrabber::DoBehavior()
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
	
	Unstuck();

	
	if (m_PlayerSpotCount > 0)
	{
		m_TargetTimer = 0;
	
		// on first time look stupid
		if (m_PlayerSpotCount == 1)
		{
			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);
		}

		if (Player()->GetCharacter()->HookGrabbed())
		{
			if (m_PlayerPos.x < Pos.x)
				m_TargetPos.x = m_PlayerPos.x + 600;
			else
				m_TargetPos.x = m_PlayerPos.x - 600;
			
			MoveTowardsTarget(80);
		}
		else
		{
			m_Hook = 0;
			MoveTowardsPlayer(40);
		}
		
		
		if (m_PlayerDistance < 80)
			m_Attack = 1;
		
		if (m_PlayerDistance < 400 && m_LastHook == 0)
			m_Hook = 1;
		
		
		if (m_PlayerDistance < 700)
			m_Direction = m_PlayerDirection;
		
		
		// start timer
		if (m_PlayerSpotTimer == 0)
			m_PlayerSpotTimer++;
	}
	else
	{
		// head to moving direction
		if (m_Move != 0)
			m_Direction = vec2(m_Move, 0);
		
		m_Hook = 0;
	}
	
	if (frandom()*10 < 3)
		m_Hook = 0;
	
	

	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}