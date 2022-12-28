/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "door.h"

CDoor::CDoor(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DOOR)
{
	m_Pos = Pos;
	
	GameWorld()->InsertEntity(this);

	for (int i = 0; i < NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}

	for (int i = 0; i < NUM_IDS; i++)
	{
		m_aIDs[i] = Server()->SnapNewID();
	}
}

CDoor::~CDoor()
{
	for (int i = 0; i < NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}

	for (int i = 0; i < NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_aIDs[i]);
	}
}

void CDoor::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CDoor::Tick()
{
	if (!m_Active)
		return;

	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 46.0f, 0);
			
	if(pChr && pChr->IsAlive() && !pChr->m_IsBot)
	{
		GameServer()->m_pController->NextLevel(pChr->GetPlayer()->GetCID());
	}
}


void CDoor::TickPaused()
{
}

void CDoor::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	if (!m_Active)
		return;

	float Radius = 46.0f;

	int NumSide = CDoor::NUM_SIDE;
	//if(Server()->GetClientAntiPing(SnappingClient))
	//	NumSide = std::min(6, NumSide);

	float AngleStep = 2.0f * pi / NumSide;

	for (int i = 0; i < NumSide; i++)
	{
		vec2 PartPosStart = m_Pos + vec2(Radius * cos(AngleStep * i), Radius * sin(AngleStep * i));
		vec2 PartPosEnd = m_Pos + vec2(Radius * cos(AngleStep * (i + 1)), Radius * sin(AngleStep * (i + 1)));

		CNetObj_Laser * pObj = static_cast<CNetObj_Laser*>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDs[i], sizeof(CNetObj_Laser)));
		if (!pObj)
			return;

		pObj->m_X = (int)PartPosStart.x;
		pObj->m_Y = (int)PartPosStart.y;
		pObj->m_FromX = (int)PartPosEnd.x;
		pObj->m_FromY = (int)PartPosEnd.y;
		pObj->m_StartTick = Server()->Tick();
	}

	//const float MAGIC_NUMBER = NUM_SIDE / 3.125f; // a magic number for NUM_SIDE = 24, NUM_HINT = 24, when dots stay
	const float MAGIC_NUMBER = NUM_SIDE / 3.0f;
	float AngleStart = (2.0f * pi * Server()->Tick() / static_cast<float>(Server()->TickSpeed())) / (MAGIC_NUMBER / 8);
	AngleStart = AngleStart * 2.0f;
	for (int i = 0; i < NUM_SIDE; i++)
	{
		vec2 PosStart = m_Pos + vec2(Radius * cos(AngleStart + AngleStep * i), Radius * sin(AngleStart + AngleStep * i));

		CNetObj_Projectile * pObj = static_cast<CNetObj_Projectile*>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_aIDs[i], sizeof(CNetObj_Projectile)));
		if (pObj)
		{
			pObj->m_X = (int)PosStart.x;
			pObj->m_Y = (int)PosStart.y;
			pObj->m_VelX = 0;
			pObj->m_VelY = 0;
			pObj->m_StartTick = Server()->Tick();
			pObj->m_Type = WEAPON_HAMMER;
		}
	}
}
