#ifndef GAME_SERVER_AI_MADBOMBER_H
#define GAME_SERVER_AI_MADBOMBER_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIMadbomber : public CAI
{
public:
	CAIMadbomber(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
};

#endif