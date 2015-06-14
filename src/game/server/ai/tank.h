#ifndef GAME_SERVER_AI_TANK_H
#define GAME_SERVER_AI_TANK_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAITank : public CAI
{
public:
	CAITank(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif