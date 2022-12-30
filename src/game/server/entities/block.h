#ifndef GAME_SERVER_ENTITIES_BLOCK_H
#define GAME_SERVER_ENTITIES_BLOCK_H

#include <game/server/lasersnap_helper.h>
#include <game/server/entity.h>

#define Left 0
#define Down 1
#define Right 2
#define Up 3

#define xL -16.0f
#define xR 16.0f
#define yU -16.0f
#define yD 16.0f
#define Distance 32.0f

class CBlock : public CEntity
{
public:
    CBlock(CGameWorld *pGameWorld, int Type, int Owner, vec2 CenterPos);
    ~CBlock();

    virtual void Tick();
    virtual void Snap(int SnappingClient);
    virtual void Reset();

    bool Check(float x, float y);
    bool Check(vec2 Pos) { return Check(Pos.x, Pos.y); };

private:
    CLaserSnapHelper *m_Connect;
    int m_Type;
    void AddEdge(vec2 edge) { m_Connect->AddEdge(edge); }
};

#endif