#ifndef GAME_SERVER_AI_GRABBER_H
#define GAME_SERVER_AI_GRABBER_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIGrabber : public CAI
{
public:
	CAIGrabber(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif