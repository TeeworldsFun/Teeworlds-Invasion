/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "radar.h"

CRadar::CRadar(CGameWorld *pGameWorld, int Type, int ObjectiveID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_RADAR)
{
	m_Active = false;
	GameWorld()->InsertEntity(this);
	m_Range = 0;

	m_Target = vec2(0, 0);
	m_Active = false;
	m_ActiveTick = 0;
	m_ObjectiveID = ObjectiveID;
}




void CRadar::Reset()
{

}


void CRadar::Tick()
{
	//m_Range += 20;
	//if (m_Range > 600)
	//	m_Range = 0;

	if (m_Type == RADAR_CHARACTER || m_Type == RADAR_HUMAN)
	{
		CCharacter *pCharacter = GameServer()->GetPlayerChar(m_ObjectiveID);
		
		if (pCharacter && (m_Type == RADAR_HUMAN && !pCharacter->m_IsBot))
		{
			m_Active = true;
		}
		else
			m_Active = false;
	}

	if (m_Type == RADAR_ENEMY)
	{
		if (m_ActiveTick > Server()->Tick())
			m_Active = true;
		else
			m_Active = false;
	}

	m_Range = 350;
	
	m_aTipPos.x += (m_aTipTargetPos.x-m_aTipPos.x)/4.0f;
	m_aTipPos.y += (m_aTipTargetPos.y-m_aTipPos.y)/4.0f;
	
	m_aBottomPos.x += (m_aBottomTargetPos.x-m_aBottomPos.x)/4.0f;
	m_aBottomPos.y += (m_aBottomTargetPos.y-m_aBottomPos.y)/4.0f;
}


void CRadar::TickPaused()
{

}

void CRadar::Snap(int SnappingClient)
{
	if (m_Type == RADAR_CHARACTER || m_Type == RADAR_HUMAN)
		if (SnappingClient == m_ObjectiveID)
			return;

	if (SnappingClient < 0 || SnappingClient >= MAX_CLIENTS)
		return;
	
	CPlayer *pPlayer = GameServer()->m_apPlayers[SnappingClient];
	if(!pPlayer)
		return;
	
	CCharacter *pCharacter = pPlayer->GetCharacter();
	if (!pCharacter)
		return;

	if (pCharacter)
		m_Pos = pCharacter->m_Pos;
	else
		return;
	
	if (pCharacter && pCharacter->m_IsBot)
		return;
		
	m_Target = pCharacter->GetPosition();

	float Angle = atan2(m_Target.x-pCharacter->m_Pos.x, m_Target.y-pCharacter->m_Pos.y);
	
	float Range = min((int)distance(m_Target, pCharacter->m_Pos), m_Range);
	
	vec2 Pos = pCharacter->m_Pos;
	vec2 TargetPos = vec2(pCharacter->m_Pos.x + sin(Angle) * Range, pCharacter->m_Pos.y + cos(Angle) * Range);

	m_aTipTargetPos = Pos;

	if(NetworkClipped(SnappingClient) || !m_Active)
		return;

	if (m_Type == RADAR_CHARACTER || m_Type == RADAR_HUMAN)
	{
		CNetObj_Pickup *pRadar = (CNetObj_Pickup *)Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup));
		if(!pRadar)
			return;

		pRadar->m_X = (int)m_aTipPos.x;
		pRadar->m_Y = (int)m_aTipPos.y;
		pRadar->m_Type = POWERUP_WEAPON;
		pRadar->m_Subtype = pCharacter->GetActiveWeapon();
	}
	if (m_Type == RADAR_DOOR)
	{
		CNetObj_Pickup *pRadar = (CNetObj_Pickup *)Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup));
		if(!pRadar)
			return;

		pRadar->m_X = (int)m_aTipPos.x;
		pRadar->m_Y = (int)m_aTipPos.y;
		pRadar->m_Type = POWERUP_HEALTH;
	}
	if (m_Type == RADAR_ENEMY)
	{
		CNetObj_Pickup *pRadar = (CNetObj_Pickup *)Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup));
		if(!pRadar)
			return;

		pRadar->m_X = (int)m_aTipPos.x;
		pRadar->m_Y = (int)m_aTipPos.y;
		pRadar->m_Type = POWERUP_ARMOR;
	}

}
