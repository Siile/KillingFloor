#ifndef GAME_SERVER_ENTITIES_LOSEPOINT_H
#define GAME_SERVER_ENTITIES_LOSEPOINT_H


#include <game/server/entity.h>

class CLosepoint : public CEntity
{
public:
	static const int ms_PhysSize = 24;

	// -x, x, -y, y
	vec4 m_Range;
	
	void UpdateRange();
	
	
	CLosepoint(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
};

#endif
