#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "bosshead.h"


CAIBosshead::CAIBosshead(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	// do smth
}


void CAIBosshead::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->SetCustomWeapon(HAMMER_BOTBASIC);
	pChr->SetHealth(400+GameServer()->Difficulty()*200);
	
	m_State = 0;
	m_StateHealth = 400+GameServer()->Difficulty()*200;
	
	m_BombSize = 10;
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "limekitty", 64);
}


void CAIBosshead::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	m_Hook = 0;
	
	SeekPlayer();

	// state change if damaged enough
	if (Player()->GetCharacter()->m_HiddenHealth < m_StateHealth - 100)
	{
		m_State++;
		m_StateHealth = Player()->GetCharacter()->m_HiddenHealth;
		
		m_Move = 0;
		m_Attack = 0;
		
		// stop body too
		if (Player()->GetCharacter()->m_Bottom->GetPlayer()->m_pAI)
			Player()->GetCharacter()->m_Bottom->GetPlayer()->m_pAI->StandStill(100);
		
		StandStill(100);
		
		int Emoticon = EMOTICON_EXCLAMATION;
		
		if (m_State == 1)
		{
			Player()->GetCharacter()->SetCustomWeapon(HAMMER_BOTBASIC);
			Player()->GetCharacter()->m_Bottom->SetCustomWeapon(HAMMER_BOTFLYHAMMER);
			Emoticon = EMOTICON_EXCLAMATION;
		}
		if (m_State == 2)
		{
			Player()->GetCharacter()->SetCustomWeapon(GUN_BOTPISTOL);
			Player()->GetCharacter()->m_Bottom->SetCustomWeapon(GUN_BOTPISTOL);
			Emoticon = EMOTICON_ZOMG;
		}
		if (m_State == 3)
		{
			Player()->GetCharacter()->SetCustomWeapon(RIFLE_BOTLIGHTNING);
			Player()->GetCharacter()->m_Bottom->SetCustomWeapon(HAMMER_BOTBASIC);
			Emoticon = EMOTICON_SUSHI;
		}
		if (m_State == 4)
		{
			Player()->GetCharacter()->m_Bottom->SetCustomWeapon(HAMMER_BOTFLYHAMMER);
			Emoticon = EMOTICON_WTF;
		}
		if (m_State == 5)
		{
			
			Player()->GetCharacter()->SetCustomWeapon(RIFLE_BOTLIGHTNING);
			Player()->GetCharacter()->m_Bottom->SetCustomWeapon(RIFLE_BOTLIGHTNING);
			Emoticon = EMOTICON_SPLATTEE;
		}
		if (m_State == 6)
		{
			
			Player()->GetCharacter()->SetCustomWeapon(RIFLE_BOTLASER);
			Player()->GetCharacter()->m_Bottom->SetCustomWeapon(RIFLE_BOTLASER);
			Emoticon = EMOTICON_DEVILTEE;
		}
		
		GameServer()->SendEmoticon(Player()->GetCID(), Emoticon);
		Player()->GetCharacter()->SetEmoteFor(EMOTE_PAIN, 1200, 1200);
		
		return;
	}
	
	
	if (m_PlayerSpotCount > 0)
	{
		if (m_PlayerSpotCount == 1)
			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);
				
		if (m_PlayerDistance < BotAttackRange[Player()->GetCharacter()->m_ActiveCustomWeapon])
			m_Attack = 1;
		
		m_Direction = m_PlayerDirection;
	}
	
	// next reaction in
	m_ReactionTime = 2;
	
}