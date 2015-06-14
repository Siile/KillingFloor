#ifndef GAME_SERVER_AI_ELECTRO_H
#define GAME_SERVER_AI_ELECTRO_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIElectro : public CAI
{
public:
	CAIElectro(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
	
	int m_LightningTimer;

private:
};

#endif