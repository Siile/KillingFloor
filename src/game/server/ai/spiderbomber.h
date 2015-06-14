#ifndef GAME_SERVER_AI_SPIDERBOMBER_H
#define GAME_SERVER_AI_SPIDERBOMBER_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAISpiderbomber : public CAI
{
public:
	CAISpiderbomber(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
};

#endif