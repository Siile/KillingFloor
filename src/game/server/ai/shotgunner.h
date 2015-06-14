#ifndef GAME_SERVER_AI_SHOTGUNNER_H
#define GAME_SERVER_AI_SHOTGUNNER_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIShotgunner : public CAI
{
public:
	CAIShotgunner(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif