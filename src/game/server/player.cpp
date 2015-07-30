
/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include "player.h"

#include <game/server/entities/staticlaser.h>
#include <game/server/upgradelist.h>



static const char *TipText[NUM_TIPS] =
{
	"Check vote menu to buy & upgrade weapons", // TIP_BUY
	"Armor shows your remaining clips, collect armor to get more ammo", // TIP_ARMOR
	//"You can throw grenades using heart emoticon", // TIP_COMMANDO
	"You can create clips by using any emoticon", // TIP_COMMANDO
	"You can heal by using any emoticon", // TIP_MEDIC
	"You can put electro mines by using any emoticon", // TIP_ENGINEER
	"You can put land mines by using any emoticon", // TIP_PIONEER
	"You can activate bloodlust by using any emoticon" // TIP_PIONEER
};




MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS)

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, int Team)
{
	m_pGameServer = pGameServer;
	m_RespawnTick = Server()->Tick();
	m_DieTick = Server()->Tick();
	m_ScoreStartTick = Server()->Tick();
	m_pCharacter = 0;
	m_ClientID = ClientID;
	m_Team = GameServer()->m_pController->ClampTeam(Team);
	m_SpectatorID = SPEC_FREEVIEW;
	m_LastActionTick = Server()->Tick();
	m_TeamChangeTick = Server()->Tick();
	
	m_Respawns = 0;
	
	// player start points
	if (ClientID < FIRST_BOT_ID)
		m_Score = 0;
		//m_Score = 60;

	m_pAI = NULL;
	m_Class = -1;
	m_ClassPickWhine = 400;
	
	for (int i = 0; i < NUM_TIPS; i++)
	{
		m_TipTimer[i] = TipTimer[i];
		m_TipEnable[i] = 1;
	}
}

CPlayer::~CPlayer()
{
	if (m_pAI)
		delete m_pAI;

	delete m_pCharacter;
	m_pCharacter = 0;
}

void CPlayer::Tick()
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	Server()->SetClientScore(m_ClientID, m_Score);

	// tip helpers
	/*
	for (int i = 0; i < NUM_TIPS; i++)
		m_TipTimer[i]--;
	
	
	if (m_TipTimer[TIP_BUY] <= 0 && m_Score >= 80 && GetCharacter())
		ShowTip(TIP_BUY);
	
	if (m_TipTimer[TIP_CLIPS] <= 0 && GetCharacter() && aCustomWeapon[GetCharacter()->m_ActiveCustomWeapon].m_ClipSize > 0 && 
		GetCharacter()->m_aWeapon[GetCharacter()->m_ActiveCustomWeapon].m_Ammo <= 0 && GetCharacter()->m_aWeapon[GetCharacter()->m_ActiveCustomWeapon].m_AmmoReserved <= 0 &&
		aCustomWeapon[GetCharacter()->m_ActiveCustomWeapon].m_MaxAmmo > 0)
	{
		ShowTip(TIP_CLIPS);
		DisableTip(TIP_CLIPS);
	}
	
	if (m_TipTimer[TIP_MEDIC] <= 0 && m_Class == CLASS_MEDIC && GetCharacter())
		ShowTip(TIP_MEDIC);
	
	if (m_TipTimer[TIP_ENGINEER] <= 0 && m_Class == CLASS_ENGINEER && GetCharacter())
		ShowTip(TIP_ENGINEER);
	
	if (m_TipTimer[TIP_PIONEER] <= 0 && m_Class == CLASS_PIONEER && GetCharacter())
		ShowTip(TIP_PIONEER);
		
	if (m_TipTimer[TIP_BERSERKER] <= 0 && m_Class == CLASS_BERSERKER && GetCharacter())
		ShowTip(TIP_BERSERKER);
	
	if (m_TipTimer[TIP_COMMANDO] <= 0 && m_Class == CLASS_COMMANDO && GetCharacter())
		ShowTip(TIP_COMMANDO);
	*/
	
	
	
	
	// do latency stuff
	{
		IServer::CClientInfo Info;
		if(Server()->GetClientInfo(m_ClientID, &Info))
		{
			m_Latency.m_Accum += Info.m_Latency;
			m_Latency.m_AccumMax = max(m_Latency.m_AccumMax, Info.m_Latency);
			m_Latency.m_AccumMin = min(m_Latency.m_AccumMin, Info.m_Latency);
		}
		// each second
		if(Server()->Tick()%Server()->TickSpeed() == 0)
		{
			m_Latency.m_Avg = m_Latency.m_Accum/Server()->TickSpeed();
			m_Latency.m_Max = m_Latency.m_AccumMax;
			m_Latency.m_Min = m_Latency.m_AccumMin;
			m_Latency.m_Accum = 0;
			m_Latency.m_AccumMin = 1000;
			m_Latency.m_AccumMax = 0;
		}
	}

	if(!GameServer()->m_World.m_Paused)
	{
		if(!m_pCharacter && m_Team == TEAM_SPECTATORS && m_SpectatorID == SPEC_FREEVIEW)
			m_ViewPos -= vec2(clamp(m_ViewPos.x-m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y-m_LatestActivity.m_TargetY, -400.0f, 400.0f));

		if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*3 <= Server()->Tick())
			m_Spawning = true;

		if(m_pCharacter)
		{
			if(m_pCharacter->IsAlive())
			{
				m_ViewPos = m_pCharacter->m_Pos;
			}
			else
			{
				delete m_pCharacter;
				m_pCharacter = 0;
			}
		}
		else if(m_Spawning && m_RespawnTick <= Server()->Tick())
			TryRespawn();
	}
	else
	{
		++m_RespawnTick;
		++m_DieTick;
		++m_ScoreStartTick;
		++m_LastActionTick;
		++m_TeamChangeTick;
 	}
}

void CPlayer::PostTick()
{
	// update latency value
	if(m_PlayerFlags&PLAYERFLAG_SCOREBOARD)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				m_aActLatency[i] = GameServer()->m_apPlayers[i]->m_Latency.m_Min;
		}
	}

	// update view pos for spectators
	if(m_Team == TEAM_SPECTATORS && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID])
		m_ViewPos = GameServer()->m_apPlayers[m_SpectatorID]->m_ViewPos;
}

void CPlayer::Snap(int SnappingClient)
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, m_ClientID, sizeof(CNetObj_ClientInfo)));
	if(!pClientInfo)
		return;

	StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
	StrToInts(&pClientInfo->m_Clan0, 3, Server()->ClientClan(m_ClientID));
	pClientInfo->m_Country = Server()->ClientCountry(m_ClientID);
	StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
	pClientInfo->m_UseCustomColor = m_TeeInfos.m_UseCustomColor;
	pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
	pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = SnappingClient == -1 ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = m_ClientID;
	pPlayerInfo->m_Score = m_Score;
	pPlayerInfo->m_Team = m_Team;

	if(m_ClientID == SnappingClient)
		pPlayerInfo->m_Local = 1;

	if(m_ClientID == SnappingClient && m_Team == TEAM_SPECTATORS)
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
			return;

		pSpectatorInfo->m_SpectatorID = m_SpectatorID;
		pSpectatorInfo->m_X = m_ViewPos.x;
		pSpectatorInfo->m_Y = m_ViewPos.y;
	}
}

void CPlayer::OnDisconnect(const char *pReason)
{
	KillCharacter();

	if(Server()->ClientIngame(m_ClientID))
	{
		char aBuf[512];
		if(pReason && *pReason)
			str_format(aBuf, sizeof(aBuf), "'%s' has left the floor (%s)", Server()->ClientName(m_ClientID), pReason);
		else
			str_format(aBuf, sizeof(aBuf), "'%s' has left the floor", Server()->ClientName(m_ClientID));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", m_ClientID, Server()->ClientName(m_ClientID));
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
	}
}

void CPlayer::OnPredictedInput(CNetObj_PlayerInput *NewInput)
{
	// skip the input if chat is active
	if((m_PlayerFlags&PLAYERFLAG_CHATTING) && (NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING))
		return;

	if(m_pCharacter)
		m_pCharacter->OnPredictedInput(NewInput);
}

void CPlayer::OnDirectInput(CNetObj_PlayerInput *NewInput)
{
	if(NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
		// skip the input if chat is active
		if(m_PlayerFlags&PLAYERFLAG_CHATTING)
			return;

		// reset input
		if(m_pCharacter)
			m_pCharacter->ResetInput();

		m_PlayerFlags = NewInput->m_PlayerFlags;
 		return;
	}

	m_PlayerFlags = NewInput->m_PlayerFlags;

	if(m_pCharacter)
		m_pCharacter->OnDirectInput(NewInput);

	if(!m_pCharacter && m_Team != TEAM_SPECTATORS && (NewInput->m_Fire&1))
		m_Spawning = true;

	// check for activity
	if(NewInput->m_Direction || m_LatestActivity.m_TargetX != NewInput->m_TargetX ||
		m_LatestActivity.m_TargetY != NewInput->m_TargetY || NewInput->m_Jump ||
		NewInput->m_Fire&1 || NewInput->m_Hook)
	{
		m_LatestActivity.m_TargetX = NewInput->m_TargetX;
		m_LatestActivity.m_TargetY = NewInput->m_TargetY;
		m_LastActionTick = Server()->Tick();
	}
}

CCharacter *CPlayer::GetCharacter()
{
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return 0;
}

void CPlayer::KillCharacter(int Weapon)
{
	if(m_pCharacter)
	{
		m_pCharacter->Die(m_ClientID, Weapon);
		delete m_pCharacter;
		m_pCharacter = 0;
	}
}

void CPlayer::Respawn()
{
	if(m_Team != TEAM_SPECTATORS)
		m_Spawning = true;
}

void CPlayer::SetTeam(int Team, bool DoChatMsg)
{
	// clamp the team
	Team = GameServer()->m_pController->ClampTeam(Team);
	if(m_Team == Team)
		return;

	char aBuf[512];
	if(DoChatMsg)
	{
		str_format(aBuf, sizeof(aBuf), "'%s' joined the %s", Server()->ClientName(m_ClientID), GameServer()->m_pController->GetTeamName(Team));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}

	KillCharacter();

	m_Team = Team;
	m_LastActionTick = Server()->Tick();
	m_SpectatorID = SPEC_FREEVIEW;
	// we got to wait 0.5 secs before respawning
	m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", m_ClientID, Server()->ClientName(m_ClientID), m_Team);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	GameServer()->m_pController->OnPlayerInfoChange(GameServer()->m_apPlayers[m_ClientID]);

	if(Team == TEAM_SPECTATORS)
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "Spectator join, segfault incoming");
		// update spectator modes
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_SpectatorID == m_ClientID)
				GameServer()->m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
		}
	}
}

void CPlayer::TryRespawn()
{
	// ask the game controller if we can respawn
	if (!GameServer()->m_pController->CanCharacterSpawn(GetCID()))
		return;

	vec2 SpawnPos;

	int Team = (GetCID() >= FIRST_BOT_ID) ? TEAM_BLUE : TEAM_RED;
	if (m_Team == TEAM_SPECTATORS)
		Team = TEAM_SPECTATORS;
	
	// respawn to lose zone if died already during ongoing wave
	if (GetCID() < FIRST_BOT_ID && m_Respawns > 0)
	{
		Team = 9000;
	}
	
	if(!GameServer()->m_pController->CanSpawn(Team, &SpawnPos))
		return;

	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GameServer()->CreatePlayerSpawn(SpawnPos);
	
	if (Team == 9000)
		GameServer()->CreateSound(m_pCharacter->m_Pos, SOUND_TEE_CRY);
	
	m_Respawns++;
	
	GameServer()->ResetVotes();
}




void CPlayer::AITick()
{
	if (m_pAI)
		m_pAI->Tick();
}
	

bool CPlayer::AIInputChanged()
{
	if (m_pAI)
		return m_pAI->m_InputChanged;

	return false;
}


bool CPlayer::BuyableWeapon(int i)
{
	DisableTip(TIP_BUY);

	if (!GotWeapon(i) && aCustomWeapon[i].m_Cost > 0 && (GotWeapon(aCustomWeapon[i].m_Require) || aCustomWeapon[i].m_Require == -1))
	{
		if (aCustomWeapon[i].m_Require >= 0 && WeaponDisabled(aCustomWeapon[i].m_Require))
			return false;

		return true;
	}
	
	return false;
}

/*
void CPlayer::VoteBuyableWeapons()
{
	DisableTip(TIP_BUY);
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		if (!GotWeapon(i) && aCustomWeapon[i].m_Cost > 0 && (GotWeapon(aCustomWeapon[i].m_Require) || aCustomWeapon[i].m_Require == -1))
		{
			if (aCustomWeapon[i].m_Require >= 0 && WeaponDisabled(aCustomWeapon[i].m_Require))
				continue;
			
			
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s - %d points", aCustomWeapon[i].m_Name, aCustomWeapon[i].m_Cost);

			GameServer()->AddVote(aBuf, aCustomWeapon[i].m_BuyCmd, GetCID());
		}
	}
}
*/

void CPlayer::ListBuyableWeapons()
{
	DisableTip(TIP_BUY);
	
	/*
	if (!GetCharacter())
	{
		GameServer()->SendChatTarget(GetCID(), "You must be alive to do shopping");
		return;
	}
	*/
	
	GameServer()->SendChatTarget(GetCID(), "Available weapons to buy:");
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		if (!GotWeapon(i) && aCustomWeapon[i].m_Cost > 0 && (GotWeapon(aCustomWeapon[i].m_Require) || aCustomWeapon[i].m_Require == -1))
		{
			if (aCustomWeapon[i].m_Require >= 0 && WeaponDisabled(aCustomWeapon[i].m_Require))
				continue;
			
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s - %s - %d points", aCustomWeapon[i].m_BuyCmd, aCustomWeapon[i].m_Name, aCustomWeapon[i].m_Cost);
			GameServer()->SendChatTarget(GetCID(), aBuf);
		}
	}
}


bool CPlayer::BuyWeapon(int CustomWeapon)
{
	if (CustomWeapon < 0 || CustomWeapon >= NUM_CUSTOMWEAPONS)
		return false;
	
	DisableTip(TIP_BUY);
	
	if (!GetCharacter())
	{
		GameServer()->SendChatTarget(GetCID(), "You must be alive to do shopping");
		return false;
	}
	
	if (aCustomWeapon[CustomWeapon].m_Cost == 0 || (!GotWeapon(aCustomWeapon[CustomWeapon].m_Require) && aCustomWeapon[CustomWeapon].m_Require != -1))
		return false;
	
	if (aCustomWeapon[CustomWeapon].m_Require >= 0 && WeaponDisabled(aCustomWeapon[CustomWeapon].m_Require))
		return false;
	
	if (GotWeapon(CustomWeapon))
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "You already have  %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetCID(), aBuf);
		return false;
	}
	
	if (m_Score < aCustomWeapon[CustomWeapon].m_Cost)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Not enough points for %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetCID(), aBuf);
		return false;
	}
	
	m_pCharacter->GiveCustomWeapon(CustomWeapon);
	//m_pCharacter->SetCustomWeapon(CustomWeapon);
	m_Score -= aCustomWeapon[CustomWeapon].m_Cost;

	if (CustomWeapon != HAMMER_BASIC)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s ready for action", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetCID(), aBuf);
	}
	
	GameServer()->ResetVotes();
	return true;
}

void CPlayer::TrySetClass(int WantedClass)
{	
	if (m_Class >= 0)
	{
		GameServer()->SendChatTarget(GetCID(), "Class already selected");
		return;
	}
	
	if (!GetCharacter())
	{
		GameServer()->SendChatTarget(GetCID(), "You must be alive to choose class");
		return;
	}
	
	
	switch (WantedClass)
	{
	case CLASS_COMMANDO:
		//GetCharacter()->GiveCustomWeapon(RIFLE_ASSAULTRIFLE);
		GameServer()->SendChatTarget(GetCID(), "You selected commando");
		GameServer()->Server()->SetClientClan(GetCID(), "Commando");
		m_Class = WantedClass;
		break;
		
	case CLASS_MEDIC:
		//GetCharacter()->GiveCustomWeapon(SHOTGUN_DOUBLEBARREL);
		GameServer()->SendChatTarget(GetCID(), "You selected medic");
		GameServer()->Server()->SetClientClan(GetCID(), "Medic");
		m_Class = WantedClass;
		break;
		
	case CLASS_ENGINEER:
		//GetCharacter()->GiveCustomWeapon(RIFLE_LASERRIFLE);
		GameServer()->SendChatTarget(GetCID(), "You selected engineer");
		GameServer()->Server()->SetClientClan(GetCID(), "Engineer");
		m_Class = WantedClass;
		break;
		
	case CLASS_PIONEER:
		//GetCharacter()->GiveCustomWeapon(GRENADE_GRENADELAUNCHER);
		GameServer()->SendChatTarget(GetCID(), "You selected pioneer");
		GameServer()->Server()->SetClientClan(GetCID(), "Pioneer");
		m_Class = WantedClass;
		break;
		
	case CLASS_BERSERKER:
		//GetCharacter()->GiveCustomWeapon(SWORD);
		GameServer()->SendChatTarget(GetCID(), "You selected berserker");
		GameServer()->Server()->SetClientClan(GetCID(), "Berserker");
		m_Class = WantedClass;
		break;
		
	default:
		break;
	};
	
	GetCharacter()->GiveClassWeapon();
	GameServer()->ResetVotes();
}


void CPlayer::DisableTip(int Tip)
{
	m_TipEnable[Tip] = 0;
}

void CPlayer::ShowTip(int Tip)
{
	/*
	if (Tip < 0 || Tip >= NUM_TIPS)
		return;
	
	if (m_TipEnable[Tip] <= 0)
		return;
	
	m_TipTimer[Tip] = TipTimer[Tip];
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Tip: %s", TipText[Tip]);
	GameServer()->SendChatTarget(GetCID(), aBuf);
	*/
}


void CPlayer::IDDQD()
{
	if (GetCharacter())
	{
		if (!GetCharacter()->m_GodMode)
		{
			GetCharacter()->m_GodMode = true;
			GameServer()->SendChatTarget(GetCID(), "God mode enabled");
		}
		else
		{
			GetCharacter()->m_GodMode = false;
			GameServer()->SendChatTarget(GetCID(), "God mode disabled");
		}
	}
}

void CPlayer::IDKFA()
{
	if (GetCharacter())
	{
		GameServer()->SendChatTarget(GetCID(), "Weapons, fuck yeah!");
		
		for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
		{
			if (!GotWeapon(i) && aCustomWeapon[i].m_Cost > 0)
			{
				m_pCharacter->GiveCustomWeapon(i);
			}
		}
	}
}


void CPlayer::GreedIsGood()
{
	m_Score = 999;
	GameServer()->SendChatTarget(GetCID(), "You greedy bastard");
}











