#ifndef GAME_SERVER_AI_NINJA_H
#define GAME_SERVER_AI_NINJA_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAINinja : public CAI
{
public:
	CAINinja(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
};

#endif