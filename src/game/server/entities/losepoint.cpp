#include <game/server/gamecontext.h>
#include "losepoint.h"

CLosepoint::CLosepoint(CGameWorld *pGameWorld)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LOSEPOINT)
{
	m_ProximityRadius = ms_PhysSize;

	Reset();
}

void CLosepoint::Reset()
{
	m_Range = vec4(0, 0, 0, 0);
}

void CLosepoint::UpdateRange()
{
	int TileSize = 8;
	
	for (int i = 0; i < 99; i++)
		if (GameServer()->Collision()->GetCollisionAt(m_Pos.x - i*TileSize, m_Pos.y) > 0)
		{
			m_Range.x = i*TileSize;
			break;
		}
			
	for (int i = 0; i < 99; i++)
		if (GameServer()->Collision()->GetCollisionAt(m_Pos.x + i*TileSize, m_Pos.y) > 0)
		{
			m_Range.y = i*TileSize;
			break;
		}
		
	for (int i = 0; i < 99; i++)
		if (GameServer()->Collision()->GetCollisionAt(m_Pos.x , m_Pos.y- i*TileSize) > 0)
		{
			m_Range.z = i*TileSize;
			break;
		}
			
	for (int i = 0; i < 99; i++)
		if (GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y + i*TileSize) > 0)
		{
			m_Range.w = i*TileSize;
			break;
		}
}

void CLosepoint::TickPaused()
{
	/*
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
	*/
}

void CLosepoint::Snap(int SnappingClient)
{
	
}
