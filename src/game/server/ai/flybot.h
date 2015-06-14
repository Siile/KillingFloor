#ifndef GAME_SERVER_AI_FLYBOT_H
#define GAME_SERVER_AI_FLYBOT_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIFlybot : public CAI
{
public:
	CAIFlybot(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif