/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PICKUP_H
#define GAME_SERVER_ENTITIES_PICKUP_H

#include <game/server/entity.h>

const int PickupPhysSize = 14;

class CPickup : public CEntity
{
public:
	CPickup(CGameWorld *pGameWorld, int Type, int SubType = 0);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	void Respawn()
	{
		if (!m_Dropable)
		{
			m_SpawnTick = -1;
			m_Flashing = false;
			m_FlashTimer = 0;
		}
	}
	
	void RespawnDropable()
	{
		m_SpawnTick = -1;
		m_Life = 480;
		m_Dropable = true;
		m_Flashing = false;
		m_FlashTimer = 0;
	}
	
	void Hide()
	{
		m_SpawnTick = 1;
		m_Life = 0;
		m_Flashing = false;
		m_FlashTimer = 0;
	}
	
	bool m_SkipAutoRespawn;
	
	bool m_Dropable;
	int m_Life;
	int m_SpawnTick;
	
	bool m_Flashing;
	int m_FlashTimer;
	
	int GetType(){ return m_Type; }
	int GetSubtype(){ return m_Type; }
	
	void SetSubtype(int Type){ m_Subtype = Type; }
	
private:
	int m_Type;
	int m_Subtype;
};

#endif
