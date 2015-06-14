#ifndef GAME_SERVER_AI_LIGHTNINGBOT_H
#define GAME_SERVER_AI_LIGHTNINGBOT_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAILightningbot : public CAI
{
public:
	CAILightningbot(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif