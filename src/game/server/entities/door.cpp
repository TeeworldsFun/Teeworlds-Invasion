/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "door.h"

CDoor::CDoor(CGameWorld *pGameWorld, vec2 From, vec2 To, int Life)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DOOR)
{
	m_From = From;
	m_Pos = To;
	m_Life = Life;
	
	GameWorld()->InsertEntity(this);
}




void CDoor::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CDoor::Tick()
{
	if (m_Life-- <= 0)
		GameServer()->m_World.DestroyEntity(this);
}


void CDoor::TickPaused()
{
}

void CDoor::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_StartTick = Server()->Tick();
}
