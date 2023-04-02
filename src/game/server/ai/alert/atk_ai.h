#ifndef GAME_SERVER_AI_ALERT_ATK_AI_H
#define GAME_SERVER_AI_ALERT_ATK_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAlertAIATK : public CAI
{
public:
	CAlertAIATK(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
};

#endif