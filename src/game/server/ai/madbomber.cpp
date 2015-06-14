#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "madbomber.h"


CAIMadbomber::CAIMadbomber(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIMadbomber::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(14+GameServer()->Difficulty()*3);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "redstripe", 64);
	
	m_BombSize = 5 + GameServer()->Difficulty()*5;
	
	m_Reward = 20;
}


void CAIMadbomber::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	m_Hook = 0;
	
	if (m_BombTimer > 0)
	{
		m_Jump = 0;
		m_Attack = 0;
		m_Move = 0;
	}
	else
	{
		CheckAITiles();
		SeekPlayer();
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
				Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
				GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_HEARTS);
			}

			MoveTowardsPlayer(40);
			JumpIfPlayerIsAbove();
			m_Direction = m_PlayerDirection;
			
			
			if (m_PlayerDistance < 300)
			{
				m_BombTimer = 100;
					
				Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
				GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_DEVILTEE);
				GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			}
			
			if (m_Pos.y < m_PlayerPos.y)
				m_Jump = 0;
		}
	}
	

	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}