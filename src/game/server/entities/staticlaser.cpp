/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "staticlaser.h"

CStaticlaser::CStaticlaser(CGameWorld *pGameWorld, vec2 From, vec2 To)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_From = From;
	m_Pos = To;
	m_EvalTick = 0;
	m_Hidden = false;
	
	GameWorld()->InsertEntity(this);
}




void CStaticlaser::Reset()
{
	//GameServer()->m_World.DestroyEntity(this);
}


void CStaticlaser::Tick()
{
	//if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/1000.0f)
	
	if (!m_Hidden)
		m_EvalTick = Server()->Tick();
}


void CStaticlaser::TickPaused()
{
	++m_EvalTick;
}

void CStaticlaser::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;
	
	//if (m_Hidden)
	if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/1000.0f)
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_StartTick = m_EvalTick;
}
