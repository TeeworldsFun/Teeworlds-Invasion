#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "atk_ai.h"


CAlertAIATK::CAlertAIATK(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
}


void CAlertAIATK::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	
	if (g_Config.m_SvRandomWeapons)
		pChr->GiveRandomWeapon();
}


void CAlertAIATK::DoBehavior()
{
	// reset jump and attack
	m_Attack = 0;

	SeekClosestEnemyInSight();

	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ShootAtClosestEnemy();
		ReactToPlayer();
	}
	else
	{
		m_AttackTimer = 0;
		m_Jump = 0;
	}

	//if (SeekClosestEnemy())
	if (SeekClosestEnemy())
	{
		m_TargetPos = m_PlayerPos;

		// distance to the player
		if (m_PlayerPos.x < m_Pos.x)
			m_TargetPos.x = m_PlayerPos.x + WeaponShootRange()/2*(0.5f+frandom()*1.0f);
		else
			m_TargetPos.x = m_PlayerPos.x - WeaponShootRange()/2*(0.5f+frandom()*1.0f);

		m_Jump = 1;
	}
	else
		m_Jump = 0;

	
	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint(20);
		HookMove();
	}
	
	
	//DoJumping();
	//Unstuck();

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}