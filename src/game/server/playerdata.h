#ifndef GAME_SERVER_PLAYERDATA_H
#define GAME_SERVER_PLAYERDATA_H

// stored player data for switching between levels
class CPlayerData
{
private:
	CPlayerData *m_pChild1;
	CPlayerData *m_pChild2;

public:
	CPlayerData(const char *pName, const char *TimeoutID);
	void Die();
	void Reset();
	
	int m_aWeaponType[99];
	int m_aWeaponAmmo[99];
	int m_aWeaponAmmoReserved[99];
	
	int m_aAmmo[99];
	int m_Weapon;
	int m_Armor;
	int m_Kits;
	int m_Score;
	int m_Gold;
	
	int m_HighestLevel;
	int m_HighestLevelSeed;
	
	char m_TimeoutID[256];
	
	char m_aName[16];
	
	void Add(CPlayerData *pPlayerData);
	CPlayerData *Get(const char *pName, const char *TimeoutID);
	
	int GetHighScore(int Score);
	int GetPlayerCount(int Score);
};

#endif
