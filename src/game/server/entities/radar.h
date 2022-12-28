#ifndef GAME_SERVER_ENTITIES_RADAR_H
#define GAME_SERVER_ENTITIES_RADAR_H

#include <game/server/entity.h>

enum
{
	RADAR_CHARACTER=0,
	RADAR_HUMAN,
	RADAR_ENEMY,
	RADAR_DOOR,
	NUM_RADARS
};

class CRadar : public CEntity
{
public:
	CRadar(CGameWorld *pGameWorld, int Type, int ObjectiveID = 0);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	vec2 m_Target;

	void Activate(vec2 Pos) { m_Active = true; m_Target = Pos;}
	void Activate(vec2 Pos, int ActiveTick) { m_Active = true; m_Target = Pos; m_ActiveTick = ActiveTick; }

private:
	int m_Range;
	vec2 m_aTipTargetPos, m_aBottomTargetPos;
	vec2 m_aTipPos, m_aBottomPos;

	int m_Type;
	int m_ObjectiveID;
	bool m_Active;
	int m_ActiveTick;
	
};

#endif

