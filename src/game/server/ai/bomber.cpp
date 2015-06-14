#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "bomber.h"


CAIBomber::CAIBomber(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIBomber::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(8+GameServer()->Difficulty()*2);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "pinky", 64);
	
	m_BombSize = 1 + GameServer()->Difficulty();
}


void CAIBomber::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	CheckAITiles();
	SeekPlayer();

	
	if (m_PlayerSpotCount > 0)
	{
		m_TargetTimer = 0;
	
		// on "first" sight
		if (m_PlayerSpotCount == 1 && m_BombTimer == 0)
		{
			Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
			GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_HEARTS);
		}

		MoveTowardsPlayer(40);
		JumpIfPlayerIsAbove();
		
		m_Direction = m_PlayerDirection;
		
	
		if (m_BombTimer == 0)
		{
			if (m_PlayerDistance < 200)
			{
				m_BombTimer = 60;
				
				Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, 1200, 1200);
				GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_DEVILTEE);
				GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			}
		}

		
		if (m_Pos.y < m_PlayerPos.y)
			m_Jump = 0;
	}
	
	Unstuck();
	HeadToMovingDirection();
	
	// next reaction in
	m_ReactionTime = 6 + frandom()*3;
	
}