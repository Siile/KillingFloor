#ifndef GAME_SERVER_ENTITIES_STATICLASER_H
#define GAME_SERVER_ENTITIES_STATICLASER_H

#include <game/server/entity.h>

class CStaticlaser : public CEntity
{
public:
	CStaticlaser(CGameWorld *pGameWorld, vec2 From, vec2 To);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	bool m_Hidden;

private:
	vec2 m_From;
	int m_EvalTick;
};

#endif
