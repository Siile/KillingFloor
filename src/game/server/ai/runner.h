#ifndef GAME_SERVER_AI_RUNNER_H
#define GAME_SERVER_AI_RUNNER_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIRunner : public CAI
{
public:
	CAIRunner(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	virtual void ReceiveDamage();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif