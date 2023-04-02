/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_ALERT_H
#define GAME_SERVER_GAMEMODES_ALERT_H
#include <game/server/gamecontroller.h>

class CGameControllerAlert : public IGameController
{
public:
	CGameControllerAlert(class CGameContext *pGameServer);
	
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	virtual void Tick();
};
#endif
