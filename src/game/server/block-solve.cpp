#include "entities/block.h"
#include "block-solve.h"

CBlockSolve::CBlockSolve(CGameContext *pGameServer)
{
    m_pGameServer = pGameServer;
}

bool CBlockSolve::Build(int Owner, vec2 Pos)
{
    if (CanPlace(Pos))
    {
        new CBlock(&GameServer()->m_World, BLOCKTYPE_SOLID, Owner, Pos);
        GameServer()->Collision()->CreateBlock(Pos, BLOCKTYPE_SOLID);
    }
    else
        return false;
    return true;
}

bool CBlockSolve::CanPlace(float x, float y)
{
    vec2 RoundPos = GameServer()->Collision()->RoundPos(vec2(x, y));
    if (GameServer()->Collision()->GetCollisionAt(RoundPos.x, RoundPos.y) || GameServer()->m_World.CheckBlock(RoundPos.x, RoundPos.y))
        return false;

    if (GameServer()->m_World.ClosestCharacter(vec2(RoundPos.x, RoundPos.y), 48.0f, NULL))
        return false;

    return true;
}