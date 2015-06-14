#ifndef GAME_SERVER_AI_PISTOLBOT_H
#define GAME_SERVER_AI_PISTOLBOT_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIPistolbot : public CAI
{
public:
	CAIPistolbot(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif