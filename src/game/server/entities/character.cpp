/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "character.h"
#include "laser.h"
#include "lightning.h"
#include "projectile.h"
#include "superexplosion.h"
#include "landmine.h"
#include "electromine.h"

#include <game/server/upgradelist.h>

#define RAD 0.017453292519943295769236907684886f

//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1)
			c.m_Presses++;
		else
			c.m_Releases++;
	}

	return c;
}


MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS)

// Character, "physical" player's part
CCharacter::CCharacter(CGameWorld *pWorld)
: CEntity(pWorld, CGameWorld::ENTTYPE_CHARACTER)
{
	m_ProximityRadius = ms_PhysSize;
	m_HiddenHealth = 10;
	m_MaxHealth = 10;
	m_Health = 0;
	m_Armor = 0;
	
	m_Bloodlust = 0;
	m_BloodlustSoundTimer = 0;
	
	m_Bottom = NULL;
	m_Top = NULL;
}

bool CCharacter::HookGrabbed()
{
	if (m_Core.m_HookState == HOOK_GRABBED)
		return true;
		
	return false;
}

void CCharacter::Reset()
{
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	m_Grenades = 2;
	m_ClassAbilityTimer = 0;

	m_GodMode = false;
	
	m_CryTimer = 0;
	m_CryState = 0;
	
	m_EmoteLockStop = 0;
	m_DeathTileTimer = 0;
	m_DelayedKill = false;
	m_WeaponPicked = false;
	m_IsBot = false;
	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_ActiveWeapon = WEAPON_GUN;
	m_LastWeapon = WEAPON_HAMMER;
	m_QueuedCustomWeapon = -1;

	m_pPlayer = pPlayer;
	m_Pos = Pos;

	m_Core.Reset();
	m_Core.Init(&GameServer()->m_World.m_Core, GameServer()->Collision());
	m_Core.m_Pos = m_Pos;

	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));

	GameServer()->m_World.InsertEntity(this);
	m_Alive = true;

	GameServer()->m_pController->OnCharacterSpawn(this);
	
	GiveCustomWeapon(HAMMER_BASIC);
	SetCustomWeapon(HAMMER_BASIC);
	
	if (pPlayer->m_pAI)
	{
		pPlayer->m_pAI->OnCharacterSpawn(this);
		m_IsBot = true;
	}
	else
		GiveClassWeapon();
	
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "votes", "Character spawn end");
	return true;
}

void CCharacter::ForceNogo()
{
	if (InNogo())
	{
		TakeDeathtileDamage();
		GameServer()->CreatePlayerSpawn(m_Pos);
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN);
		if (m_Core.m_Vel.x < 0)
		{
			Teleport(m_Pos + vec2(60, 0));
			m_Core.m_Vel.x = 16.0f;
		}
		else if (m_Core.m_Vel.x > 0)
		{
			Teleport(m_Pos - vec2(60, 0));
			m_Core.m_Vel.x = -16.0f;
		}
		GameServer()->CreatePlayerSpawn(m_Pos);
		
		if ((aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD || aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_FLYHAMMER) && m_Ninja.m_CurrentMoveTime > 0)
		{
			m_Ninja.m_CurrentMoveTime = 0;
			
			if (m_Ninja.m_ActivationDir.x < 0)
			{
				Teleport(m_Pos + vec2(60, 0));
				m_Core.m_Vel.x = 16.0f;
			}
			else if (m_Ninja.m_ActivationDir.x > 0)
			{
				Teleport(m_Pos - vec2(60, 0));
				m_Core.m_Vel.x = -16.0f;
			}
		}
	}
}

bool CCharacter::InNogo()
{
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y)&CCollision::COLFLAG_NOGO)
		return true;
	return false;
}



void CCharacter::ActivateBloodlust()
{
	m_Bloodlust = 180;
	m_BloodlustSoundTimer = 0;
}



	
bool CCharacter::SetLandmine()
{
	//int i = GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y-32);
	
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x-16, m_Pos.y+24)&CCollision::COLFLAG_SOLID && GameServer()->Collision()->GetCollisionAt(m_Pos.x+16, m_Pos.y+24)&CCollision::COLFLAG_SOLID)
	{
		new CLandmine(&GameServer()->m_World, m_Pos + vec2(0, 16), m_pPlayer->GetCID());
		GameServer()->CreateSound(m_Pos, SOUND_BODY_LAND);
		return true;
	}
	else
	{
		
	}
	return false;
}


bool CCharacter::SetElectromine()
{
	//int i = GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y-32);
	
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x-16, m_Pos.y+24)&CCollision::COLFLAG_SOLID && GameServer()->Collision()->GetCollisionAt(m_Pos.x+16, m_Pos.y+24)&CCollision::COLFLAG_SOLID)
	{
		new CElectromine(&GameServer()->m_World, m_Pos + vec2(0, 16), m_pPlayer->GetCID());
		GameServer()->CreateSound(m_Pos, SOUND_BODY_LAND);
		return true;
	}
	else
	{
		
	}
	return false;
}


void CCharacter::AddGrenades(int Num)
{
	if (Num < 1)
		return;
	
	int G = m_Grenades;
	
	m_Grenades += Num;
	if (m_Grenades > MAX_GRENADES)
		m_Grenades = MAX_GRENADES;
	
	
	char aBuf[256];
	
	if (G == m_Grenades)
	{
		str_format(aBuf, sizeof(aBuf), "Full grenades (%d)", m_Grenades);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "You gained %d grenades (%d/%d grenades)", G, m_Grenades, MAX_GRENADES);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
	}
}



void CCharacter::ThrowGrenade()
{
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	vec2 ProjStartPos = m_Pos+Direction*m_ProximityRadius*0.75f;

	CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_GRENADE,
		m_pPlayer->GetCID(),
		ProjStartPos,
		Direction,
		(int)(Server()->TickSpeed()*aCustomWeapon[GRENADE_GRENADE].m_BulletLife),
		aCustomWeapon[GRENADE_GRENADE].m_Damage, true, 0, SOUND_GRENADE_EXPLODE, aCustomWeapon[GRENADE_GRENADE].m_ParentWeapon, aCustomWeapon[GRENADE_GRENADE].m_Extra1);

	// pack the Projectile and send it to the client Directly
	CNetObj_Projectile p;
	pProj->FillInfo(&p);

	CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
	Msg.AddInt(1);
	for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
		Msg.AddInt(((int *)&p)[i]);
	Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
	
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_SKID);

	char aBuf[256];
	
	
	m_Grenades--;
	
	if (m_Grenades == 1)
	{
		str_format(aBuf, sizeof(aBuf), "%d grenade left", m_Grenades);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "%d grenades left", m_Grenades);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
	}
}

void CCharacter::Teleport(vec2 Pos)
{
	m_Pos = Pos;
	m_Core.m_Pos = m_Pos;

	m_Core.Reset();
	
	m_Pos = Pos;
	m_Core.m_Pos = m_Pos;
	
	if (GetPlayer()->m_pAI)
		GetPlayer()->m_pAI->StandStill(15);
}

void CCharacter::Destroy()
{
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	m_Alive = false;
}



void CCharacter::SetCustomWeapon(int CustomWeapon)
{
	if(CustomWeapon < 0 || CustomWeapon >= NUM_CUSTOMWEAPONS)
		return;
	
	if(CustomWeapon == m_ActiveCustomWeapon)
		return;

	m_LastWeapon = m_ActiveCustomWeapon;
	m_QueuedCustomWeapon = -1;
	m_ActiveCustomWeapon = CustomWeapon;
	GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
	
	// check if the weapon needs reloading
	if (m_aWeapon[CustomWeapon].m_Ammo <= 0 && m_aWeapon[CustomWeapon].m_AmmoReserved > 0)
	{
		//if (m_pPlayer->m_Class == CLASS_COMMANDO)
		//	m_ClipReloadTimer = aCustomWeapon[CustomWeapon].m_ClipReloadTime * Server()->TickSpeed() / 1000 * 0.85f;
		//else
			m_ClipReloadTimer = aCustomWeapon[CustomWeapon].m_ClipReloadTime * Server()->TickSpeed() / 1000;
	}
	else
		m_ClipReloadTimer = 0;

	if (GetPlayer()->GetCID() < FIRST_BOT_ID)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Using: %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), aBuf);
	}
}


	
bool CCharacter::IsGrounded()
{
	if(GameServer()->Collision()->CheckPoint(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	return false;
}


void CCharacter::HandleNinja()
{
	if(aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType != PROJTYPE_SWORD && aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType != PROJTYPE_FLYHAMMER)
		return;

	/*
	if ((Server()->Tick() - m_Ninja.m_ActivationTick) > (g_pData->m_Weapons.m_Ninja.m_Duration * Server()->TickSpeed() / 1000))
	{
		// time's up, return
		//m_aWeapons[WEAPON_NINJA].m_Got = false;
		m_ActiveWeapon = m_LastWeapon;

		//SetWeapon(m_ActiveWeapon);
		return;
	}
	*/

	// force ninja Weapon
	//SetWeapon(WEAPON_NINJA);

	m_Ninja.m_CurrentMoveTime--;

	if (m_Ninja.m_CurrentMoveTime == 0)
	{
		// reset velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir*m_Ninja.m_OldVelAmount;
		
		// special effect for mjölnir
		if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == ELECTRIC && aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_FLYHAMMER)
		{
			for (int i = 0; i < 9; i++)
			{
				float a = frandom()*360 * RAD;
				new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 50, 50, m_pPlayer->GetCID(), 2);
			}
		}
	}

	if (m_Ninja.m_CurrentMoveTime > 0)
	{
		// special effet for pikatanachu
		if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == ELECTRIC && aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD)
		{
			float a = frandom()*360 * RAD;
			new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 50, 50, m_pPlayer->GetCID(), 2);
		}
		
		// Set velocity
		if (aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD)
			m_Core.m_Vel = m_Ninja.m_ActivationDir * g_pData->m_Weapons.m_Ninja.m_Velocity;
		else
			m_Core.m_Vel = m_Ninja.m_ActivationDir * g_pData->m_Weapons.m_Ninja.m_Velocity * 0.4f;
		
		
		vec2 OldPos = m_Pos;
		GameServer()->Collision()->MoveBox(&m_Core.m_Pos, &m_Core.m_Vel, vec2(m_ProximityRadius, m_ProximityRadius), 0.f);

		// reset velocity so the client doesn't predict stuff
		m_Core.m_Vel = vec2(0.f, 0.f);

		// check if we Hit anything along the way
		{
			CCharacter *aEnts[MAX_CLIENTS];
			vec2 Dir = m_Pos - OldPos;
			float Radius = m_ProximityRadius * 2.0f;
			vec2 Center = OldPos + Dir * 0.5f;
			int Num = GameServer()->m_World.FindEntities(Center, Radius, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; ++i)
			{
				// skip self
				if (aEnts[i] == this || aEnts[i] == m_Top || aEnts[i] == m_Bottom)
					continue;

				// make sure we haven't Hit this object before
				bool bAlreadyHit = false;
				for (int j = 0; j < m_NumObjectsHit; j++)
				{
					if (m_apHitObjects[j] == aEnts[i])
						bAlreadyHit = true;
				}
				if (bAlreadyHit)
					continue;

				// check so we are sufficiently close
				if (distance(aEnts[i]->m_Pos, m_Pos) > (m_ProximityRadius * 2.0f))
					continue;

				// Hit a player, give him damage and stuffs...
				if (aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD)
					GameServer()->CreateSound(aEnts[i]->m_Pos, SOUND_NINJA_HIT);
				else
					GameServer()->CreateSound(aEnts[i]->m_Pos, SOUND_HAMMER_HIT);
				
				// set his velocity to fast upward (for now)
				if(m_NumObjectsHit < 10)
					m_apHitObjects[m_NumObjectsHit++] = aEnts[i];

				vec2 Force = vec2(0, -4.0f);
				
				if (aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_FLYHAMMER)
					Force = m_Ninja.m_ActivationDir * 20.0f;
				
				if (m_pPlayer->GetCID() < FIRST_BOT_ID)
					Force *= 1.6f;
				
				int Parent = WEAPON_NINJA;
				if (aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_FLYHAMMER)
					Parent = WEAPON_HAMMER;
				
				int Damage = aCustomWeapon[m_ActiveCustomWeapon].m_Damage;
				
				if (m_Bloodlust > 0 && aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD)
				{
					Damage += 1;
					IncreaseHealth(Damage);
				}
				
				aEnts[i]->TakeDamage(Force, aCustomWeapon[m_ActiveCustomWeapon].m_Damage, m_pPlayer->GetCID(), Parent);
			}
		}

		return;
	}

	return;
}


void CCharacter::DoWeaponSwitch()
{
	// make sure we can switch
	if(m_ReloadTimer > 0 || m_QueuedCustomWeapon == -1)
		return;

	// switch Weapon
	SetCustomWeapon(m_QueuedCustomWeapon);
	m_ReloadTimer = 0;
	m_aNextWeapon[aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon] = m_ActiveCustomWeapon;
}



void CCharacter::HandleWeaponSwitch()
{
	int WantedWeapon = m_ActiveCustomWeapon;
	if(m_QueuedCustomWeapon != -1)
		WantedWeapon = m_QueuedCustomWeapon;

	// select Weapon
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	if(Next < 128) // make sure we only try sane stuff
	{
		while(Next) // Next Weapon selection
		{
			WantedWeapon = (WantedWeapon+1)%NUM_CUSTOMWEAPONS;
			if(m_aWeapon[WantedWeapon].m_Got && !m_aWeapon[WantedWeapon].m_Disabled)
				Next--;
			
			//WantedWeapon = GetNextWeapon(aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
			//Next--;
		}
	}

	if(Prev < 128) // make sure we only try sane stuff
	{
		while(Prev) // Prev Weapon selection
		{
			WantedWeapon = (WantedWeapon-1)<0?NUM_CUSTOMWEAPONS-1:WantedWeapon-1;
			if(m_aWeapon[WantedWeapon].m_Got && !m_aWeapon[WantedWeapon].m_Disabled)
				Prev--;
			
			//WantedWeapon = GetPrevWeapon(aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
			//Prev--;
		}
	}

	// Direct Weapon selection
	if(m_LatestInput.m_WantedWeapon)
	{
		ScanWeapons();
		int w = GetWeapon(m_Input.m_WantedWeapon-1);
		if (w != -1)
			WantedWeapon = w;
	}

	// check for insane values
	if(WantedWeapon >= 0 && WantedWeapon < NUM_CUSTOMWEAPONS && WantedWeapon != m_ActiveCustomWeapon && m_aWeapon[WantedWeapon].m_Got && !m_aWeapon[WantedWeapon].m_Disabled)
		m_QueuedCustomWeapon = WantedWeapon;

	DoWeaponSwitch();
}





int CCharacter::GetWeapon(int ParentType)
{
	if (m_aNextWeapon[ParentType] < 0 || m_aNextWeapon[ParentType] > NUM_CUSTOMWEAPONS)
	{
		m_aNextWeapon[ParentType] = 0;
		return GetNextWeapon(ParentType);
	}
	
	if (aCustomWeapon[m_aNextWeapon[ParentType]].m_ParentWeapon == ParentType)
		return m_aNextWeapon[ParentType];
		
	else
		return GetNextWeapon(ParentType);
}

int CCharacter::GetPrevWeapon(int ParentType)
{
	int w = m_aNextWeapon[ParentType];
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS+1; i++)
	{
		if (--w < 0)
			w = NUM_CUSTOMWEAPONS;
			
		if (aCustomWeapon[w].m_ParentWeapon == ParentType && m_aWeapon[w].m_Got && !m_aWeapon[w].m_Disabled)
		{
			m_aNextWeapon[ParentType] = w;
			return w;
		}
	}
	
	return -1;
}

int CCharacter::GetNextWeapon(int ParentType)
{
	int w = m_aNextWeapon[ParentType];
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS+1; i++)
	{
		if (++w >= NUM_CUSTOMWEAPONS)
			w = 0;
			
		if (aCustomWeapon[w].m_ParentWeapon == ParentType && m_aWeapon[w].m_Got && !m_aWeapon[w].m_Disabled)
		{
			m_aNextWeapon[ParentType] = w;
			return w;
		}
	}
	
	return -1;
}


int CCharacter::GetFirstWeapon(int ParentType)
{
	for (int i = 0; i < NUM_CUSTOMWEAPONS+1; i++)
	{
		if (aCustomWeapon[i].m_ParentWeapon == ParentType && m_aWeapon[i].m_Got && !m_aWeapon[i].m_Disabled)
		{
			return i;
		}
	}
	
	return -1;
}


void CCharacter::ScanWeapons()
{
	for (int i = 0; i < NUM_WEAPONS; i++)
	{
		int w = m_aNextWeapon[i];
		if (w < 0 || w >= NUM_CUSTOMWEAPONS)
		{
			m_aNextWeapon[i] = GetFirstWeapon(i);
			continue;
		}
		
		if (m_aWeapon[m_aNextWeapon[i]].m_Got && !m_aWeapon[m_aNextWeapon[i]].m_Disabled)
		{
			continue;
		}
		else
		{
			m_aNextWeapon[i] = GetFirstWeapon(i);
			continue;
		}
	}
}



void CCharacter::FireWeapon()
{
	DoWeaponSwitch();
	
	if(m_ReloadTimer > 0 || m_ClipReloadTimer > 0)
		return;
	
	
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = aCustomWeapon[m_ActiveCustomWeapon].m_FullAuto;
	
	if (m_IsBot)
		FullAuto = true;
	
	//bool UseAmmo = true;
	//if (aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo <= 0)
	//		UseAmmo = false;

	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1))
		WillFire = true;

	if(!WillFire)
		return;

		
	// check for ammo
	if(m_aWeapon[m_ActiveCustomWeapon].m_Ammo <= 0 && aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize > 0)
	{
		// 125ms is a magical limit of how fast a human can click
		m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
		if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
		{
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
			m_LastNoAmmoSound = Server()->Tick();
		}

		return;
	}
	

	vec2 ProjStartPos = m_Pos+Direction*m_ProximityRadius*0.75f;

	// play sound
	if (aCustomWeapon[m_ActiveCustomWeapon].m_Sound >= 0)
		GameServer()->CreateSound(m_Pos, aCustomWeapon[m_ActiveCustomWeapon].m_Sound);
	
	// create the projectile
	switch(aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType)
	{
		case PROJTYPE_SWORD:
		{
			// reset Hit objects
			m_NumObjectsHit = 0;

			m_Ninja.m_ActivationDir = Direction;
			//m_Ninja.m_CurrentMoveTime = aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife * Server()->TickSpeed() / 1000;
			m_Ninja.m_CurrentMoveTime = g_pData->m_Weapons.m_Ninja.m_Movetime * aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife * Server()->TickSpeed() / 1000;
			m_Ninja.m_OldVelAmount = length(m_Core.m_Vel);

			GameServer()->CreateSound(m_Pos, SOUND_NINJA_FIRE);
		} break;		
		
		case PROJTYPE_FLYHAMMER:
		{
			// reset Hit objects
			m_NumObjectsHit = 0;

			m_Ninja.m_ActivationDir = Direction;
			//m_Ninja.m_CurrentMoveTime = aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife * Server()->TickSpeed() / 1000;
			m_Ninja.m_CurrentMoveTime = g_pData->m_Weapons.m_Ninja.m_Movetime * aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife * Server()->TickSpeed() / 1000;
			m_Ninja.m_OldVelAmount = length(m_Core.m_Vel);

			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_SKID);
		} break;
		
		
		case PROJTYPE_HAMMER:
		{
			// reset objects Hit
			m_NumObjectsHit = 0;

			CCharacter *apEnts[MAX_CLIENTS];
			int Hits = 0;
			int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*0.5f, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; ++i)
			{
				CCharacter *pTarget = apEnts[i];

				if ((pTarget == this) || (pTarget == m_Top) || (pTarget == m_Bottom) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
					continue;

				// set his velocity to fast upward (for now)
				if(length(pTarget->m_Pos-ProjStartPos) > 0.0f)
					GameServer()->CreateHammerHit(pTarget->m_Pos-normalize(pTarget->m_Pos-ProjStartPos)*m_ProximityRadius*0.5f);
				else
					GameServer()->CreateHammerHit(ProjStartPos);

				vec2 Dir;
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);
				else
					Dir = vec2(0.f, -1.f);

				// apply sleep effect
				if (pTarget->GetPlayer()->m_pAI && aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == SLEEPEFFECT)
				{
					if (!pTarget->m_Bottom && !pTarget->m_Top)
						pTarget->GetPlayer()->m_pAI->Zzz(100);
				}
				
				pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f * aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, aCustomWeapon[m_ActiveCustomWeapon].m_Damage,
					m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
				Hits++;
			}

			// if we Hit anything, we have to wait for the reload
			//if(Hits)
			//	m_ReloadTimer = Server()->TickSpeed()/3;

		} break;
		
		case PROJTYPE_BULLET:
		{
			float a = GetAngle(Direction);
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == BULLETSPREAD)
				a += frandom()*0.08f - frandom()*0.08f;
			
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == BIGBULLETSPREAD)
				a += frandom()*0.14f - frandom()*0.14f;
				
			
			CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_GUN,
				m_pPlayer->GetCID(),
				ProjStartPos,
				vec2(cosf(a), sinf(a)),
				(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
				aCustomWeapon[m_ActiveCustomWeapon].m_Damage, 0, aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, -1, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);

			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(1);
			for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);

			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
		} break;

		case PROJTYPE_PELLET:
		{
			// int ShotSpread = 2;

			// CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			// Msg.AddInt(ShotSpread*2+1);

			
			int ShotSpread = aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread / 2;
			
			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread);
				
			if (aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread%2)
			{
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+2];
					float v = 1-(absolute(i)/(float)ShotSpread);
					float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
					CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_SHOTGUN,
						m_pPlayer->GetCID(),
						ProjStartPos,
						vec2(cosf(a), sinf(a))*Speed,
						//(int)(Server()->TickSpeed()*GameServer()->Tuning()->m_ShotgunLifetime), // 0.2f
						(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
						aCustomWeapon[m_ActiveCustomWeapon].m_Damage, 0, aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, -1, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
					
					// pack the Projectile and send it to the client Directly
					CNetObj_Projectile p;
					pProj->FillInfo(&p);

					for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
						Msg.AddInt(((int *)&p)[i]);
				}
			}
			else
			{
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.130f, -0.050f, 0.050f, 0.130f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+3];
					float v = 1-(absolute(i)/(float)ShotSpread);
					float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
					CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_SHOTGUN,
						m_pPlayer->GetCID(),
						ProjStartPos,
						vec2(cosf(a), sinf(a))*Speed,
						(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
						aCustomWeapon[m_ActiveCustomWeapon].m_Damage, 0, aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, -1, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
					
					// pack the Projectile and send it to the client Directly
					CNetObj_Projectile p;
					pProj->FillInfo(&p);

					for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
						Msg.AddInt(((int *)&p)[i]);
				}
			}

			Server()->SendMsg(&Msg, 0,m_pPlayer->GetCID());
		} break;

		case PROJTYPE_GRENADE:
		{
			CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_GRENADE,
				m_pPlayer->GetCID(),
				ProjStartPos,
				Direction,
				(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
				aCustomWeapon[m_ActiveCustomWeapon].m_Damage, true, 0, SOUND_GRENADE_EXPLODE, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);

			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(1);
			for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);
			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
		} break;

		case PROJTYPE_LASER:
		{
			float a = GetAngle(Direction);
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == BULLETSPREAD)
				a += frandom()*0.08f - frandom()*0.08f;
			
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == BIGBULLETSPREAD)
				a += frandom()*0.14f - frandom()*0.14f;
			
			new CLaser(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), GameServer()->Tuning()->m_LaserReach, m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_Damage, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);
		} break;
		
		case PROJTYPE_LIGHTNING:
		{
		/*
			float a = GetAngle(Direction);
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == BULLETSPREAD)
				a += frandom()*0.08f - frandom()*0.08f;
			
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == BIGBULLETSPREAD)
				a += frandom()*0.14f - frandom()*0.14f;

			new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 200, 100, m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_Damage);
			//new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_Damage);
		*/
		
			int ShotSpread = aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread / 2;

			if (aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread%2)
			{
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+2];

					new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 200, 100, m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_Damage);
				}
			}
			else
			{
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.130f, -0.050f, 0.050f, 0.130f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+3];
					
					new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 200, 100, m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_Damage);
				}
			}
		} break;
	};
	
	
	/*
		case WEAPON_NINJA:
		{
			// reset Hit objects
			m_NumObjectsHit = 0;

			m_Ninja.m_ActivationDir = Direction;
			m_Ninja.m_CurrentMoveTime = g_pData->m_Weapons.m_Ninja.m_Movetime * Server()->TickSpeed() / 1000;
			m_Ninja.m_OldVelAmount = length(m_Core.m_Vel);

			GameServer()->CreateSound(m_Pos, SOUND_NINJA_FIRE);
		} break;

	}
	*/

	m_AttackTick = Server()->Tick();

	if(m_aWeapon[m_ActiveCustomWeapon].m_Ammo > 0 && aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize)
		m_aWeapon[m_ActiveCustomWeapon].m_Ammo--;

	if(m_ReloadTimer <= 0)
	{
		m_ReloadTimer = aCustomWeapon[m_ActiveCustomWeapon].m_BulletReloadTime * Server()->TickSpeed() / 1000;
	}
}


void CCharacter::DoReloading(bool SkipAmmoCheck)
{
	if (aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize == 0 || m_aWeapon[m_ActiveCustomWeapon].m_Ammo >= aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize)
		return;
	
	// check if we need to reload the weapon
	if ((m_aWeapon[m_ActiveCustomWeapon].m_Ammo <= 0 || SkipAmmoCheck) && m_ClipReloadTimer <= 0 && (m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved > 0 || aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo <= 0))
	{
		// check if the weapon uses clips
		if (aCustomWeapon[m_ActiveCustomWeapon].m_ClipReloadTime > 0)
		{
			//if (m_pPlayer->m_Class == CLASS_COMMANDO)
			//	m_ClipReloadTimer = aCustomWeapon[m_ActiveCustomWeapon].m_ClipReloadTime * Server()->TickSpeed() / 1000 * 0.85f;
			//else
				m_ClipReloadTimer = aCustomWeapon[m_ActiveCustomWeapon].m_ClipReloadTime * Server()->TickSpeed() / 1000;
		}
	}
	
	
	if(m_ClipReloadTimer > 0)
	{
		// reloading...
		m_ClipReloadTimer--;
		
		// play annoying sound
		if(!m_IsBot && m_LastReloadingSound <= Server()->Tick())
		{
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
			m_LastReloadingSound = Server()->Tick() + 10;
		}

		// ...done!
		if (m_ClipReloadTimer <= 0)
		{
			if (aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo)
			{
				int AmmoWanted = aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize - m_aWeapon[m_ActiveCustomWeapon].m_Ammo;
				
				if (AmmoWanted > m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved)
				{
					m_aWeapon[m_ActiveCustomWeapon].m_Ammo += m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved;
					m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved = 0;
				}
				else
				{
					m_aWeapon[m_ActiveCustomWeapon].m_Ammo += AmmoWanted;
					m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved -= AmmoWanted;
				}
			}
			else
			{
				// infinite bullets, no ammo limit set
				m_aWeapon[m_ActiveCustomWeapon].m_Ammo = aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize;
			}
		}
	}
}

	
	
void CCharacter::HandleWeapons()
{
	HandleNinja();
	DoReloading();
	ShowArmor();

	
	if(m_ReloadTimer > 0)
	{
		m_ReloadTimer--;
		return;
	}
	
	// fire Weapon, if wanted
	FireWeapon();

	return;
}



void CCharacter::ShowArmor()
{
	// showing "clips" as armor points
	if (aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo <= 0 && aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize <= 0)
		m_Armor = 0;
	else
	{
		m_Armor = m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved / float(aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo)*10.0f;
		if (m_Armor == 0 && m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved > 0)
			m_Armor = 1;
	}
}




void CCharacter::Explode(int Size)
{
	if (Size > 0)
	{
		CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, m_Pos, m_pPlayer->GetCID(), WEAPON_HAMMER, Size);
		GameServer()->m_World.InsertEntity(S);
	}
	m_DelayedKill = true;
}




void CCharacter::GiveClassWeapon()
{
	switch (m_pPlayer->m_Class)
	{
	case CLASS_COMMANDO:
		GiveCustomWeapon(RIFLE_ASSAULTRIFLE);
		break;
		
	case CLASS_MEDIC:
		GiveCustomWeapon(SHOTGUN_DOUBLEBARREL);
		break;
		
	case CLASS_ENGINEER:
		GiveCustomWeapon(RIFLE_LASERRIFLE);
		break;
		
	case CLASS_PIONEER:
		GiveCustomWeapon(GRENADE_GRENADELAUNCHER);
		break;
		
	case CLASS_BERSERKER:
		GiveCustomWeapon(SWORD_KATANA);
		break;
		
	default:
		break;
	};
}

bool CCharacter::GiveCustomWeapon(int CustomWeapon)
{
	if(!m_aWeapon[CustomWeapon].m_Got)
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chr", "GiveCustomWeapon start");
	
		m_aWeapon[CustomWeapon].m_Got = true;
		m_aWeapon[CustomWeapon].m_Disabled = false;
		m_aWeapon[CustomWeapon].m_Ready = false;
		m_aWeapon[CustomWeapon].m_Ammo = aCustomWeapon[CustomWeapon].m_ClipSize;
		
		bool SkipAmmoFill = false;
		
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chr", "weapon requirements");
		if (aCustomWeapon[CustomWeapon].m_Require >= 0)
		{
			m_aWeapon[aCustomWeapon[CustomWeapon].m_Require].m_Disabled = true;
			
			// ammo
			if (m_aWeapon[aCustomWeapon[CustomWeapon].m_Require].m_Got)
			{
				SkipAmmoFill = true;
				m_aWeapon[CustomWeapon].m_AmmoReserved = m_aWeapon[aCustomWeapon[CustomWeapon].m_Require].m_AmmoReserved + m_aWeapon[aCustomWeapon[CustomWeapon].m_Require].m_Ammo;
				
				if (m_aWeapon[CustomWeapon].m_AmmoReserved > aCustomWeapon[CustomWeapon].m_MaxAmmo)
					m_aWeapon[CustomWeapon].m_AmmoReserved = aCustomWeapon[CustomWeapon].m_MaxAmmo;
				
				m_aWeapon[CustomWeapon].m_Ammo = 0;
			}
		}
		
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chr", "ammo fill");
		if (!SkipAmmoFill)
		{
			switch (GameServer()->Difficulty())
			{
				case NORMAL:
				m_aWeapon[CustomWeapon].m_AmmoReserved = aCustomWeapon[CustomWeapon].m_MaxAmmo;
				break;
							
				case HARD:
				m_aWeapon[CustomWeapon].m_AmmoReserved = aCustomWeapon[CustomWeapon].m_MaxAmmo * 0.7f;
				break;
					
				case SUICIDAL:
				m_aWeapon[CustomWeapon].m_AmmoReserved = aCustomWeapon[CustomWeapon].m_MaxAmmo * 0.3f;
				break;
			};
		}
		
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chr", "ScanWeapons");
		ScanWeapons();
		
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chr", "DoWeaponSwitch");
		if (m_ActiveCustomWeapon == aCustomWeapon[CustomWeapon].m_Require)
		{
			m_QueuedCustomWeapon = CustomWeapon;
			DoWeaponSwitch();
		}
		
		return true;
	}
	return false;
}


void CCharacter::GiveNinja()
{
	/*
	m_Ninja.m_ActivationTick = Server()->Tick();
	m_aWeapons[WEAPON_NINJA].m_Got = true;
	m_aWeapons[WEAPON_NINJA].m_Ammo = -1;
	if (m_ActiveWeapon != WEAPON_NINJA)
		m_LastWeapon = m_ActiveWeapon;
	m_ActiveWeapon = WEAPON_NINJA;

	GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA);
	*/
}

void CCharacter::SetEmote(int Emote, int Tick)
{
	//if (m_EmoteLockStop > Tick)
	//	return;
	
	if (m_EmoteLockStop > Server()->Tick())
		return;

	m_EmoteType = Emote;
	m_EmoteStop = Tick;
}

void CCharacter::SetEmoteFor(int Emote, int Ticks, int LockEmote, bool UseTime)
{
	if (m_EmoteLockStop > Server()->Tick() && LockEmote == 0)
		return;

	m_EmoteType = Emote;
	
	if (UseTime)
	{
		m_EmoteStop = Server()->Tick() + Ticks * Server()->TickSpeed() / 1000;
		if (LockEmote > 0)
			m_EmoteLockStop = Server()->Tick() + LockEmote * Server()->TickSpeed() / 1000;
	}
	else
	{
		m_EmoteStop = Server()->Tick() + Ticks * Server()->TickSpeed();
		if (LockEmote > 0)
			m_EmoteLockStop = Server()->Tick() + LockEmote * Server()->TickSpeed();
	}
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
	// check for changes
	if(mem_comp(&m_Input, pNewInput, sizeof(CNetObj_PlayerInput)) != 0)
		m_LastAction = Server()->Tick();

	// copy new input
	mem_copy(&m_Input, pNewInput, sizeof(m_Input));
	m_NumInputs++;

	// it is not allowed to aim in the center
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
	mem_copy(&m_LatestInput, pNewInput, sizeof(m_LatestInput));

	// it is not allowed to aim in the center
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;

	if(m_NumInputs > 2 && m_pPlayer->GetTeam() != TEAM_SPECTATORS)
	{
		HandleWeaponSwitch();
		FireWeapon();
	}

	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
}

void CCharacter::ResetInput()
{
	m_Input.m_Direction = 0;
	m_Input.m_Hook = 0;
	// simulate releasing the fire button
	if((m_Input.m_Fire&1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}


void CCharacter::TopTick()
{
	int D = m_Core.m_Direction;
	int A = m_Core.m_Angle;
	
	m_Core = m_Bottom->GetCore();
	
	m_Core.m_Direction = D;
	m_Core.m_Angle = A;
	
	m_Pos = m_Bottom->GetPosition() + vec2(0, -40);

	m_Core.m_Pos = m_Pos;
	
	/*
	int Distance = distance(m_Pos, m_Head->GetPosition());
	
	if (Distance > 700)
	{
		GameServer()->CreatePlayerSpawn(m_Pos);
		
		m_Pos = m_Head->GetPosition() + vec2(frandom()*30 - frandom()*30, -20);
		m_Core.m_Pos = m_Pos;
		GameServer()->CreatePlayerSpawn(m_Pos);
	}
	
	if (Distance > 150)
		//m_Core.m_Vel.x = (m_Head->GetPosition().x - m_Pos.x) / 10.0f;
		m_Core.m_Vel = vec2((m_Head->GetPosition().x - m_Pos.x) / 20.0f, (m_Head->GetPosition().y - m_Pos.y) / 20.0f);
	*/
	
	/*
	if (GetPlayer()->m_pAI->GetMove() < 0 && m_Pos.x > m_Head->GetPosition().x)
	{
		m_Child = m_Head;
		m_Head = NULL;
		m_Child->m_Head = this;
		m_Child->m_Child = NULL;
	}
	*/
	
	/*
	CCharacter *Head = GameServer()->m_pController->GetHead();
	
	if (!Head)
	{
		m_DelayedKill = true;
		return;
	}
		
	m_Pos = Head->GetPosition() + vec2(0, -50);
	m_Core = Head->GetCore();

	m_Core.m_Pos = m_Pos;
	*/
}


void CCharacter::Tick()
{
	if(m_pPlayer->m_ForceBalanced)
	{
		char Buf[128];
		str_format(Buf, sizeof(Buf), "You were moved to %s due to team balancing", GameServer()->m_pController->GetTeamName(m_pPlayer->GetTeam()));
		GameServer()->SendBroadcast(Buf, m_pPlayer->GetCID());

		m_pPlayer->m_ForceBalanced = false;
	}

	m_Core.m_Input = m_Input;
	m_Core.Tick(true);
	
	if (m_GodMode)
		SetHealth(30);
	
	if (m_CryTimer > 0)
		m_CryTimer--;
	
	if (m_Bottom)
		TopTick();

	// handle death-tiles and leaving gamelayer
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameLayerClipped(m_Pos))
	{
		if (m_DeathTileTimer <= 0)
		{
			m_DeathTileTimer = 10;
			TakeDeathtileDamage();
			//Die(m_pPlayer->GetCID(), WEAPON_WORLD);
		}
	}
	
	if (m_DelayedKill)
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);

	if (m_DeathTileTimer > 0)
		m_DeathTileTimer--;
	
	
	if (m_ClassAbilityTimer > 0)
	{
		m_ClassAbilityTimer--;
		if (m_ClassAbilityTimer <= 0 && m_pPlayer->m_Class == CLASS_MEDIC)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Heal loaded");
		}
		
		if (m_ClassAbilityTimer <= 0 && m_pPlayer->m_Class == CLASS_COMMANDO)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Create clips loaded");
		}	
		
		if (m_ClassAbilityTimer <= 0 && m_pPlayer->m_Class == CLASS_ENGINEER)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Electro mine loaded");
		}		
		
		if (m_ClassAbilityTimer <= 0 && m_pPlayer->m_Class == CLASS_PIONEER)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Land mine loaded");
		}
		
		if (m_ClassAbilityTimer <= 0 && m_pPlayer->m_Class == CLASS_BERSERKER)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Bloodlust loaded");
		}
	}
	
	
	if (m_Bloodlust > 0)
	{
		m_Bloodlust--;
		
		if (m_BloodlustSoundTimer-- <= 0)
		{
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			m_BloodlustSoundTimer = 10;
		}
	}
	
	
	// handle Weapons
	HandleWeapons();

	// Previnput
	m_PrevInput = m_Input;
	return;
}

void CCharacter::TickDefered()
{
	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GameServer()->Collision());
		m_ReckoningCore.Tick(false);
		m_ReckoningCore.Move();
		m_ReckoningCore.Quantize();
	}

	//lastsentcore
	vec2 StartPos = m_Core.m_Pos;
	vec2 StartVel = m_Core.m_Vel;
	bool StuckBefore = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.Move();
	bool StuckAfterMove = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	bool StuckAfterQuant = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

	if(!StuckBefore && (StuckAfterMove || StuckAfterQuant))
	{
		// Hackish solution to get rid of strict-aliasing warning
		union
		{
			float f;
			unsigned u;
		}StartPosX, StartPosY, StartVelX, StartVelY;

		StartPosX.f = StartPos.x;
		StartPosY.f = StartPos.y;
		StartVelX.f = StartVel.x;
		StartVelY.f = StartVel.y;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "STUCK!!! %d %d %d %f %f %f %f %x %x %x %x",
			StuckBefore,
			StuckAfterMove,
			StuckAfterQuant,
			StartPos.x, StartPos.y,
			StartVel.x, StartVel.y,
			StartPosX.u, StartPosY.u,
			StartVelX.u, StartVelY.u);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}

	int Events = m_Core.m_TriggeredEvents;
	int Mask = CmaskAllExceptOne(m_pPlayer->GetCID());

	if(Events&COREEVENT_GROUND_JUMP) GameServer()->CreateSound(m_Pos, SOUND_PLAYER_JUMP, Mask);

	if(Events&COREEVENT_HOOK_ATTACH_PLAYER) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_PLAYER, CmaskAll());
	if(Events&COREEVENT_HOOK_ATTACH_GROUND) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_GROUND, Mask);
	if(Events&COREEVENT_HOOK_HIT_NOHOOK) GameServer()->CreateSound(m_Pos, SOUND_HOOK_NOATTACH, Mask);


	if(m_pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		m_Pos.x = m_Input.m_TargetX;
		m_Pos.y = m_Input.m_TargetY;
	}

	// update the m_SendCore if needed
	{
		CNetObj_Character Predicted;
		CNetObj_Character Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		// only allow dead reackoning for a top of 3 seconds
		if(m_ReckoningTick+Server()->TickSpeed()*3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
}

void CCharacter::TickPaused()
{
	++m_AttackTick;
	++m_DamageTakenTick;
	++m_Ninja.m_ActivationTick;
	++m_ReckoningTick;
	if(m_LastAction != -1)
		++m_LastAction;
	if(m_EmoteStop > -1)
		++m_EmoteStop;
}

void CCharacter::SetHealth(int Health)
{
	m_MaxHealth = Health;
	m_HiddenHealth = Health;
}

bool CCharacter::IncreaseHealth(int Amount)
{
	if(m_HiddenHealth >= m_MaxHealth)
		return false;
	m_HiddenHealth = clamp(m_HiddenHealth+Amount, 0, m_MaxHealth);
	return true;
}


// use armor points as clips
bool CCharacter::AddClip()
{
	if (aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize <= 0)
		return false;

	if (m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved <= aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo - aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize)
	{
		m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved += aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize;
		return true;
	}
	
	return false;
}


// use armor points as clips
bool CCharacter::IncreaseArmor(int Amount)
{
	if (aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize <= 0)
		return false;

	if (m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved >= aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo)
		return false;
		
	if (m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved + aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize >= aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo)
	{
		m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved = aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo;
	}
	else
	{
		m_aWeapon[m_ActiveCustomWeapon].m_AmmoReserved += aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize;
	}
	
	return true;
}



void CCharacter::Die(int Killer, int Weapon, bool SkipKillMessage)
{
	if (Weapon < 0)
		Weapon = 0;
	// we got to wait 0.5 secs before respawning
	m_pPlayer->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
	
	if (!SkipKillMessage)
	{
		int ModeSpecial = GameServer()->m_pController->OnCharacterDeath(this, GameServer()->m_apPlayers[Killer], Weapon);

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "kill killer='%d:%s' victim='%d:%s' weapon=%d special=%d",
			Killer, Server()->ClientName(Killer),
			m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()), Weapon, ModeSpecial);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

		// send the kill message
		CNetMsg_Sv_KillMsg Msg;
		Msg.m_Killer = Killer;
		Msg.m_Victim = m_pPlayer->GetCID();
		Msg.m_Weapon = Weapon;
		Msg.m_ModeSpecial = ModeSpecial;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
	}

	// a nice sound
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_DIE);

	// this is for auto respawn after 3 secs
	m_pPlayer->m_DieTick = Server()->Tick();

	m_Alive = false;
	GameServer()->m_World.RemoveEntity(this);
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID());
	
	// only bosses upper part can trigger death
	if (m_Bottom)
		m_Bottom->Die(Killer, Weapon, true);
	
	if (m_Top)
		m_Top->m_Bottom = NULL;
}


void CCharacter::Cry()
{
	if (m_CryTimer <= 0)
	{
		m_CryTimer = 50;
		if (m_CryState == 0 || m_CryState == 2)
		{
			GameServer()->CreateSound(m_Pos, SOUND_TEE_CRY);
			m_CryState++;
		}
		else if (m_CryState == 1)
		{
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);
			m_CryState++;
			m_CryTimer = 30;
		}
		else if (m_CryState == 3)
		{
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			m_CryState = 0;
		}
	}
	
}



bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{	
	if (m_Top)
	{
		Force /= 2.0f;
		m_Core.m_Vel += Force;
		return m_Top->TakeDamage(vec2(0, 0), Dmg, From, Weapon);;
	}
	
	m_Core.m_Vel += Force;

	//if(GameServer()->m_pController->IsFriendlyFire(m_pPlayer->GetCID(), From) && !g_Config.m_SvTeamdamage)
	//	return false;


	// skip player damage
	if(GameServer()->Difficulty() != SUICIDAL && From < FIRST_BOT_ID && m_pPlayer->GetCID() < FIRST_BOT_ID)
		return false;
	
	// no self damage to boss
	if (m_Bottom && (m_Bottom->GetPlayer()->GetCID() == From || GetPlayer()->GetCID() == From))
		return false;
		
	// m_pPlayer only inflicts half damage on self
	if(From == m_pPlayer->GetCID())
		Dmg = max(1, Dmg/2);

	m_DamageTaken++;


		// create healthmod indicator
		if(Server()->Tick() < m_DamageTakenTick+25)
		{
			// make sure that the damage indicators doesn't group together
			GameServer()->CreateDamageInd(m_Pos, m_DamageTaken*0.25f, Dmg);
		}
		else
		{
			m_DamageTaken = 0;
			GameServer()->CreateDamageInd(m_Pos, 0, Dmg);
		}

	// bottom part of boss doesn't take damage
	if(Dmg && !m_Top)
	{
		m_HiddenHealth -= Dmg;
	}
	
	// make sure that boss doesn't just die like normal tees
	if (m_Bottom && m_HiddenHealth <= 0)
	{
		GetPlayer()->m_pAI->m_BombTimer = 200;
		m_HiddenHealth = 9000;
		GameServer()->SendEmoticon(GetPlayer()->GetCID(), EMOTICON_DEVILTEE);
	}
	

	m_DamageTakenTick = Server()->Tick();

	if (!m_Bottom)
	{
		// do damage Hit sound
		if(From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
		{
			int Mask = CmaskOne(From);
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
					Mask |= CmaskOne(i);
			}
			GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
		}
	}

	// check for death
	if(m_HiddenHealth <= 0)
	{
		Die(From, Weapon);

		// set attacker's face to happy (taunt!)
		if (From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
		{
			CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
			if (pChr)
			{
				SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
				//pChr->m_EmoteType = EMOTE_HAPPY;
				//pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
			}
		}

		return false;
	}

	if (Dmg > 2)
		GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
	else
		GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	SetEmote(EMOTE_PAIN, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
	//m_EmoteType = EMOTE_PAIN;
	//m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;

	if (Dmg > 0 && GetPlayer()->m_pAI && Weapon >= 0)
	{
		GetPlayer()->m_pAI->ReceiveDamage();
	}
	
	return true;
}


void CCharacter::TakeDeathtileDamage()
{
	m_DamageTaken++;
	
	int i = GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y-32);
	
	if(!(i&CCollision::COLFLAG_SOLID) && !(i&CCollision::COLFLAG_DEATH) && !(i&CCollision::COLFLAG_NOHOOK))
	{
		m_Core.m_Vel.y = -5.0f;
		m_Core.m_Jumped = 0;
		
		// teleport boss out of deathtiles
		/*
		if (m_Top)
		{
			GameServer()->CreatePlayerSpawn(m_Pos);
			Teleport(m_Pos + vec2(0, -120));
			GameServer()->CreatePlayerSpawn(m_Pos);
		}
		*/
	}
	/*
	else
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x+32, m_Pos.y) == 0)
	{
		m_Core.m_Vel.x = 4.0f;
	}
	else
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x-32, m_Pos.y) == 0)
	{
		m_Core.m_Vel.x = -4.0f;
	}
	else
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y+32) == 0)
	{
		m_Core.m_Vel.y = 4.0f;
	}
	*/
	

	// create healthmod indicator
	if(Server()->Tick() < m_DamageTakenTick+25)
	{
		// make sure that the damage indicators doesn't group together
		GameServer()->CreateDamageInd(m_Pos, 1*0.25f, 3);
	}
	else
	{
		GameServer()->CreateDamageInd(m_Pos, 0, 3);
	}

	m_HiddenHealth -= 3;

	m_DamageTakenTick = Server()->Tick();
	
	
	// check for death
	if(m_HiddenHealth <= 0)
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);
		return;
	}

	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	SetEmote(EMOTE_PAIN, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);

	if (GetPlayer()->m_pAI)
		GetPlayer()->m_pAI->ReceiveDamage();
}

void CCharacter::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, m_pPlayer->GetCID(), sizeof(CNetObj_Character)));
	if(!pCharacter)
		return;

	// write down the m_Core
	if(!m_ReckoningTick || GameServer()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pCharacter->m_Tick = 0;
		m_Core.Write(pCharacter);
	}
	else
	{
		pCharacter->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pCharacter);
	}

	// set emote
	if (m_EmoteStop < Server()->Tick())
	{
		m_EmoteType = EMOTE_NORMAL;
		m_EmoteStop = -1;
	}

	pCharacter->m_Emote = m_EmoteType;

	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;

	if (aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD)
		pCharacter->m_Weapon = WEAPON_NINJA;
	else
		pCharacter->m_Weapon = aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon;
	
	pCharacter->m_AttackTick = m_AttackTick;

	pCharacter->m_Direction = m_Input.m_Direction;

	if(m_pPlayer->GetCID() == SnappingClient || SnappingClient == -1 ||
		(!g_Config.m_SvStrictSpectateMode && m_pPlayer->GetCID() == GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID))
	{
		pCharacter->m_Health = int(float(m_HiddenHealth)/float(m_MaxHealth) * 10.0f);
		if (pCharacter->m_Health < 1)
			pCharacter->m_Health = 1;

		pCharacter->m_Armor = m_Armor;
			
		if(m_aWeapon[m_ActiveCustomWeapon].m_Ammo > 0)
		{
			if (aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize < 10)
			{
				pCharacter->m_AmmoCount = m_aWeapon[m_ActiveCustomWeapon].m_Ammo;
			}
			else
			{
				pCharacter->m_AmmoCount = int(float(m_aWeapon[m_ActiveCustomWeapon].m_Ammo)/float(aCustomWeapon[m_ActiveCustomWeapon].m_ClipSize) * 10.0f);
				if (pCharacter->m_AmmoCount == 0 && m_aWeapon[m_ActiveCustomWeapon].m_Ammo > 0)
					pCharacter->m_AmmoCount = 1;
			}
		}
			
	}

	if(pCharacter->m_Emote == EMOTE_NORMAL)
	{
		if(250 - ((Server()->Tick() - m_LastAction)%(250)) < 5)
			pCharacter->m_Emote = EMOTE_BLINK;
	}

	pCharacter->m_PlayerFlags = GetPlayer()->m_PlayerFlags;
}

