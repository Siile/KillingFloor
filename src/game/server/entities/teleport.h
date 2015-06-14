#ifndef GAME_SERVER_ENTITIES_TELEPORT_H
#define GAME_SERVER_ENTITIES_TELEPORT_H

enum TeleportType
{
	TELEPORT_IN,
	TELEPORT_OUT
};

#include <game/server/entity.h>

class CTeleport : public CEntity
{
public:
	static const int ms_PhysSize = 24;

	int m_Type;
	
	
	CTeleport(CGameWorld *pGameWorld, int Type);

	virtual void Reset();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
};

#endif
