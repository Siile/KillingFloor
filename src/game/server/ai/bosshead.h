#ifndef GAME_SERVER_AI_BOSSHEAD_H
#define GAME_SERVER_AI_BOSSHEAD_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIBosshead : public CAI
{
public:
	CAIBosshead(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
	
	int m_State;
	int m_StateHealth;
};

#endif