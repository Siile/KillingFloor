#ifndef GAME_SERVER_AI_BOMBER_H
#define GAME_SERVER_AI_BOMBER_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIBomber : public CAI
{
public:
	CAIBomber(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
};

#endif