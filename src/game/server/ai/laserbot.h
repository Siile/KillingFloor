#ifndef GAME_SERVER_AI_LASERBOT_H
#define GAME_SERVER_AI_LASERBOT_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAILaserbot : public CAI
{
public:
	CAILaserbot(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif