/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "alert.h"

#include <game/server/ai.h>
#include <game/server/ai/alert/atk_ai.h>



CGameControllerAlert::CGameControllerAlert(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "Alert";
}

void CGameControllerAlert::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);

	// init AI
	if (RequestAI)
		pChr->GetPlayer()->m_pAI = new CAlertAIATK(GameServer(), pChr->GetPlayer());

	GameServer()->ResetVotes();
}



void CGameControllerAlert::Tick()
{
	IGameController::Tick();
	AutoBalance();

	GameServer()->UpdateAI();
}
