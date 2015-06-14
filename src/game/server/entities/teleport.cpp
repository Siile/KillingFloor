#include <game/server/gamecontext.h>
#include "teleport.h"

CTeleport::CTeleport(CGameWorld *pGameWorld, int Type)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_TELEPORT)
{
	m_Type = Type;
	m_ProximityRadius = ms_PhysSize;

	Reset();
}

void CTeleport::Reset()
{

}

void CTeleport::TickPaused()
{
	/*
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
	*/
}

void CTeleport::Snap(int SnappingClient)
{
	
}
