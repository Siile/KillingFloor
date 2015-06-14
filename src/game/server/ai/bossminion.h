#ifndef GAME_SERVER_AI_BOSSMINION_H
#define GAME_SERVER_AI_BOSSMINION_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIBossminion : public CAI
{
public:
	CAIBossminion(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
};

#endif