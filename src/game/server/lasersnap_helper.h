#ifndef LASERSNAPHELPER
#define LASERSNAPHELPER

#include <cmath>
#include <base/vmath.h>
#include <base/tl/array.h>
#include "entity.h"
// #include "gamecontext.h"

class CLaserSnapHelper
{
private:
    array<vec2> m_Data;
    array<int> m_IDs;
    CGameContext *m_pGameServer;
    CGameContext *GameServer() { return m_pGameServer; };
    vec2 m_CenterPos;

public:
    CLaserSnapHelper(CGameContext *pGameServer, vec2 Pos)
    {
        m_pGameServer = pGameServer;
        m_CenterPos = Pos;
    }
    int SizeEdge() { return m_Data.size(); }
    int SizeID() { return m_IDs.size(); }
    int GetID(int at) { return m_IDs[at]; }

    bool TestSizeEdge() { return SizeEdge() ? true : false; }

    void AddEdge(vec2 pos)
    {
        m_Data.add(pos);
    };

    void AddID(int ID)
    {
        m_IDs.add(ID);
    };

    void Delete(int at)
    {
        if (at < SizeEdge() && at >= 0)
            m_Data.remove_index(at);
        else
            return;
    };

    void DeleteID(int ID)
    {
        if (ID < SizeID() && ID >= 0)
            m_IDs.remove_index(ID);
        else
            return;
    };

    void Edit(int at, vec2 value)
    {
        if (at < SizeEdge() && at >= 0)
            m_Data[at] = value;
        else
            return;
    }

    /* Example.
    void ExampleSnap(int SnapType)
    {
        for (int i = 0; i < SizeID(); i++)
        {
            CNetObj_DDNetLaser *pObj = static_cast<CNetObj_DDNetLaser *>(GameServer()->Server()->SnapNewItem(NETOBJTYPE_DDNETLASER, GetID(i), sizeof(CNetObj_DDNetLaser)));
            if (!pObj)
                return;

            pObj->m_FromX = (int)(At(i).x + m_CenterPos.x);
            pObj->m_FromY = (int)(At(i).y + m_CenterPos.y);
            pObj->m_ToX = (int)(Next(i).x + m_CenterPos.x);
            pObj->m_ToY = (int)(Next(i).y + m_CenterPos.y);
            pObj->m_Owner = -1;
            pObj->m_StartTick = GameServer()->Server()->Tick();
            pObj->m_Type = SnapType;
        }
    }*/

    vec2 At(int at) { return (SizeEdge() ? m_Data[at] : vec2(0, 0)); }
    vec2 Next(int Current) { return Current + 1 >= SizeEdge() ? At(0) : At(Current + 1); }
    vec2 Prev(int Current) { return Current - 1 <= 0 ? At(SizeEdge()-1) : At(Current - 1); }
};

#endif