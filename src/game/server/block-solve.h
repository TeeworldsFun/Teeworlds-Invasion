#ifndef GAME_SERVER_BUILDSOLVE_H
#define GAME_SERVER_BUILDSOLVE_H

#include <game/server/gamecontext.h>
#include <vector>
#include <game/collision.h>

class CBlock;

class CBlockSolve
{
private:
    CGameContext *m_pGameServer;
    CGameContext *GameServer() { return m_pGameServer; }

public:
    CBlockSolve(CGameContext *pGameServer);

    bool Build(int Owner, vec2 Pos);
    bool CanPlace(float x, float y);
    bool CanPlace(vec2 Pos) { return CanPlace(Pos.x, Pos.y); };

    std::vector<CBlock> *m_pBlock;
};

#endif