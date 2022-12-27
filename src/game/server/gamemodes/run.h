/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_COOP_H
#define GAME_SERVER_GAMEMODES_COOP_H
#include <game/server/gamecontroller.h>

class CGameControllerCoop : public IGameController
{
private:
	void SpawnNewGroup(bool AddBots = true);

	vec2 GetBotSpawnPos();
	void RandomGroupSpawnPos();
	int m_BotSpawnTick;

	// hordes of enemies
	int m_EnemyCount;
	int m_EnemiesLeft;

	int m_BossesLeft;

	int m_NumEnemySpawnPos;
	int m_SpawnPosRotation;

	int m_TriggerLevel;
	int m_TriggerTick;

	bool m_AutoRestart;

	void Trigger(bool IncreaseLevel);

	class CRadar *m_pDoor;
	class CRadar *m_pEnemySpawn;
public:
	CGameControllerCoop(class CGameContext *pGameServer);

	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();

	void DisplayExit(vec2 Pos);
};
#endif
