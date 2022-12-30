#include <game/server/gamecontext.h>
#include "block.h"

CBlock::CBlock(CGameWorld *pGameWorld, int Type, int Owner, vec2 CenterPos) : CEntity(pGameWorld, CGameWorld::ENTTYPE_BLOCK)
{
    m_Pos = GameServer()->Collision()->RoundPos(CenterPos);
    m_Type = Type;

    m_Connect = new CLaserSnapHelper(GameServer(), m_Pos);

    GameWorld()->InsertEntity(this);

    for (int i = 0; i < 4; i++)
        m_Connect->AddID(Server()->SnapNewID());

    AddEdge(vec2(-16, -16)); // Left and Up Corner
    AddEdge(vec2(-16, 16));  // Left and Down Corner
    AddEdge(vec2(16, 16));   // Right and Down Corner
    AddEdge(vec2(16, -16));  // Right and Up Corner
}

CBlock::~CBlock()
{
}

void CBlock::Tick()
{
}

void CBlock::Reset()
{
    for (int i = 0; i < 4; i++)
    {
        m_Connect->Delete(i);
        m_Connect->DeleteID(i);
    }
}

bool CBlock::Check(float x, float y)
{
    return (GameServer()->Collision()->GetCollisionAt(x, y) || GameWorld()->CheckBlock(x, y));
}

void CBlock::Snap(int SnappingClient)
{
    // m_Connect->ExampleSnap(LASERTYPE_FREEZE);

    for (int i = 0; i < m_Connect->SizeID(); i++)
    {
        CNetObj_DDNetLaser *pObj = static_cast<CNetObj_DDNetLaser *>(GameServer()->Server()->SnapNewItem(NETOBJTYPE_DDNETLASER, m_Connect->GetID(i), sizeof(CNetObj_DDNetLaser)));
        if (!pObj)
            return;

        bool Snap = true;
        float x = m_Pos.x;
        float y = m_Pos.y;
        switch (i)
        {
        case Left:
            if (Check(x - Distance, y))
                continue;
            break;
        case Down:
            if (Check(x, y + Distance))
                continue;
            break;
        case Right:
            if (Check(x + Distance, y))
                continue;
            break;
        case Up:
            if (Check(x, y - Distance))
                continue;
            break;

        default:
            break;
        }
        pObj->m_FromX = (int)(m_Connect->At(i).x + m_Pos.x);
        pObj->m_FromY = (int)(m_Connect->At(i).y + m_Pos.y);
        pObj->m_ToX = (int)(m_Connect->Next(i).x + m_Pos.x);
        pObj->m_ToY = (int)(m_Connect->Next(i).y + m_Pos.y);
        pObj->m_Owner = -1;
        pObj->m_StartTick = GameServer()->Server()->Tick();
        pObj->m_Type = LASERTYPE_DOOR;

        CNetObj_EntityEx *pEntData = 0;
        pEntData = static_cast<CNetObj_EntityEx *>(Server()->SnapNewItem(NETOBJTYPE_ENTITYEX, m_Connect->GetID(i), sizeof(CNetObj_EntityEx)));
        if (pEntData)
        {
            pEntData->m_SwitchNumber = 0;
            pEntData->m_Layer = 4;
            pEntData->m_EntityClass = ENTITYCLASS_DOOR;
        }
    }
}