#ifndef GAME_SERVER_ENTITIES_STATICLASER_H
#define GAME_SERVER_ENTITIES_STATICLASER_H

#include <game/server/entity.h>

class CDoor : public CEntity
{
public:
	enum
	{
		NUM_SIDE = 12,
		NUM_PARTICLES = 12,
		NUM_IDS = NUM_SIDE + NUM_PARTICLES,
	};

public:
	CDoor(CGameWorld *pGameWorld, vec2 Pos);
	~CDoor();
	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	bool m_Active;

private:
	int m_IDs[NUM_IDS];
	int m_IDs2[NUM_IDS];
	int m_IDs3[NUM_IDS];
	int m_aIDs[NUM_IDS];
};

#endif
