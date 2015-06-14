#ifndef GAME_SERVER_AI_BOSSBODY_H
#define GAME_SERVER_AI_BOSSBODY_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIBossbody : public CAI
{
public:
	CAIBossbody(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
};

#endif