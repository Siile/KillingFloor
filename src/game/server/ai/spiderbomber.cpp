#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "spiderbomber.h"


CAISpiderbomber::CAISpiderbomber(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAISpiderbomber::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(10+GameServer()->Difficulty()*2);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "pinky", 64);
	
	m_BombSize = 2 + GameServer()->Difficulty();
	m_Reward = 10;
}


void CAISpiderbomber::DoBehavior()
{
	// reset jump, hook and attack
	m_Jump = 0;
	m_Hook = 0;
	m_Attack = 0;
	
	
	CheckAITiles();
	SeekPlayer();
	Unstuck();

	
	if (m_PlayerSpotCount > 0)
	{
		m_TargetTimer = 0;
	
		// on first time standstill and look stupid
		if (m_PlayerSpotCount == 1 && m_BombTimer == 0)
		{
			Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
			GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_HEARTS);
		}

		MoveTowardsPlayer(40);
		JumpIfPlayerIsAbove();
		
		
		vec2 HookPos = m_Pos - vec2(0, 300);
			
		if (m_Move < 0)
			HookPos.x -= 250;
		else
			HookPos.x += 250;
			
				
		// hook if something in sight
		if (GameServer()->Collision()->IntersectLine(m_Pos, HookPos, NULL, NULL) && m_Hook == 0)
		{
			m_Hook = 1;
			m_Direction = HookPos - m_Pos;
			if (Player()->GetCharacter()->IsGrounded())
				m_Jump = 1;
		}
		
		if (m_Hook == 0)
			m_Direction = m_PlayerDirection;
			
	
		if (m_BombTimer == 0)
		{
			if (m_PlayerDistance < 200 && m_PlayerSpotCount > 0)
			{
				m_BombTimer = 60;
				
				Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
				GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_DEVILTEE);
				GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			}
		}
		
		
		if (m_Pos.y < m_PlayerPos.y)
		{
			m_Jump = 0;
			m_Hook = 0;
		}
	}
	else
		HeadToMovingDirection();
	
	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}