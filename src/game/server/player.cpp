
/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include "player.h"

#include <game/server/upgradelist.h>



MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS)

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, int Team)
{
	m_pGameServer = pGameServer;
	m_RespawnTick = Server()->Tick();
	m_DieTick = Server()->Tick();
	m_ScoreStartTick = Server()->Tick();
	m_pCharacter = 0;
	m_ClientID = ClientID;
	m_Team = GameServer()->m_pController->ClampTeam(Team);
	m_SpectatorID = SPEC_FREEVIEW;
	m_LastActionTick = Server()->Tick();
	m_TeamChangeTick = Server()->Tick();
	
	m_Score = 0;
	m_Money = g_Config.m_SvStartMoney;
	
	m_CanShop = false;
	GameServer()->ClearShopVotes(ClientID);
	
	m_InterestPoints = 0;
	m_BroadcastingCaptureStatus = false;
	
	m_EnableEmoticonGrenades = true;
	m_EnableWeaponInfo = 2;
	m_EnableAutoSpectating = true;
	
	m_IsBot = false;
	m_pAI = NULL;
	
	m_WantedTeam = m_Team;
	//m_Team = TEAM_SPECTATORS;
	
	if(str_comp(g_Config.m_SvGametype, "cstt") == 0)
		m_ForceToSpectators = true;
	else
		m_ForceToSpectators = false;
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
		m_aSavedWeapon[i] = false;
	
	// warm welcome awaits
	m_Welcomed = false;
}

CPlayer::~CPlayer()
{
	if (m_pAI)
		delete m_pAI;

	delete m_pCharacter;
	m_pCharacter = 0;
}


void CPlayer::NewRound()
{
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
		m_aSavedWeapon[i] = false;
	
	m_Score = 0;
	m_Money = g_Config.m_SvStartMoney;
	
	DisableShopping();
	m_InterestPoints = 0;
}

void CPlayer::DisableShopping()
{
	if (!m_CanShop)
		return;
	
	m_CanShop = false;
	GameServer()->ClearShopVotes(GetCID());
}
	
	
	
void CPlayer::Tick()
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	Server()->SetClientScore(m_ClientID, m_Score);
	
	// do latency stuff
	{
		IServer::CClientInfo Info;
		if(Server()->GetClientInfo(m_ClientID, &Info))
		{
			m_Latency.m_Accum += Info.m_Latency;
			m_Latency.m_AccumMax = max(m_Latency.m_AccumMax, Info.m_Latency);
			m_Latency.m_AccumMin = min(m_Latency.m_AccumMin, Info.m_Latency);
		}
		// each second
		if(Server()->Tick()%Server()->TickSpeed() == 0)
		{
			m_Latency.m_Avg = m_Latency.m_Accum/Server()->TickSpeed();
			m_Latency.m_Max = m_Latency.m_AccumMax;
			m_Latency.m_Min = m_Latency.m_AccumMin;
			m_Latency.m_Accum = 0;
			m_Latency.m_AccumMin = 1000;
			m_Latency.m_AccumMax = 0;
		}
	}

	if (m_ForceToSpectators)
		ForceToSpectators();

	
	if (m_IsBot)
		m_InterestPoints /= 1.025f;
	else
		m_InterestPoints /= 1.02f;
	
	/*
	if (m_InterestPoints > 0)
		m_InterestPoints--;
	else
	{
		//if (GetCharacter())
		//	m_InterestPoints = int(frandom()*10);
	}
	*/
	

	
	if(!GameServer()->m_World.m_Paused)
	{
		if(!m_pCharacter && m_Team == TEAM_SPECTATORS && m_SpectatorID == SPEC_FREEVIEW)
			m_ViewPos -= vec2(clamp(m_ViewPos.x-m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y-m_LatestActivity.m_TargetY, -400.0f, 400.0f));
		
		//if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*3 <= Server()->Tick())
		if(!m_pCharacter)
			m_Spawning = true;

		if(m_pCharacter)
		{
			if(m_pCharacter->IsAlive())
			{
				m_ViewPos = m_pCharacter->m_Pos;
			}
			else
			{
				delete m_pCharacter;
				m_pCharacter = 0;
			}
		}
		//else if(m_Spawning && m_RespawnTick <= Server()->Tick())
		else if(m_Spawning && (m_RespawnTick <= Server()->Tick() || str_comp(g_Config.m_SvGametype, "cstt") == 0))
			TryRespawn();
	}
	else
	{
		++m_RespawnTick;
		++m_DieTick;
		++m_ScoreStartTick;
		++m_LastActionTick;
		++m_TeamChangeTick;
 	}
}

void CPlayer::PostTick()
{
	// update latency value
	if(m_PlayerFlags&PLAYERFLAG_SCOREBOARD)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				m_aActLatency[i] = GameServer()->m_apPlayers[i]->m_Latency.m_Min;
		}
	}

	// update view pos for spectators
	if(m_Team == TEAM_SPECTATORS && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID])
		m_ViewPos = GameServer()->m_apPlayers[m_SpectatorID]->m_ViewPos;
}

void CPlayer::Snap(int SnappingClient)
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, m_ClientID, sizeof(CNetObj_ClientInfo)));
	if(!pClientInfo)
		return;

	StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
	StrToInts(&pClientInfo->m_Clan0, 3, Server()->ClientClan(m_ClientID));
	pClientInfo->m_Country = Server()->ClientCountry(m_ClientID);
	StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
	pClientInfo->m_UseCustomColor = m_TeeInfos.m_UseCustomColor;
	pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
	pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = SnappingClient == -1 ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = m_ClientID;
	pPlayerInfo->m_Score = m_Score;
	pPlayerInfo->m_Team = m_Team;

	if(m_ClientID == SnappingClient)
		pPlayerInfo->m_Local = 1;

	if(m_ClientID == SnappingClient && m_Team == TEAM_SPECTATORS)
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
		{
			// SPEC_FREEVIEW

			//pSpectatorInfo->m_X = 0;
			//pSpectatorInfo->m_Y = 0;
		}
		else
		{
			pSpectatorInfo->m_SpectatorID = m_SpectatorID;
			pSpectatorInfo->m_X = m_ViewPos.x;
			pSpectatorInfo->m_Y = m_ViewPos.y;
		}
	}
}

void CPlayer::OnDisconnect(const char *pReason)
{
	KillCharacter();

	if(Server()->ClientIngame(m_ClientID))
	{
		char aBuf[512];
		
		if (!m_IsBot)
		{
			if(pReason && *pReason)
				str_format(aBuf, sizeof(aBuf), "'%s' has left the game (%s)", Server()->ClientName(m_ClientID), pReason);
			else
				str_format(aBuf, sizeof(aBuf), "'%s' has left the game", Server()->ClientName(m_ClientID));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		}

		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", m_ClientID, Server()->ClientName(m_ClientID));
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
	}
}

void CPlayer::OnPredictedInput(CNetObj_PlayerInput *NewInput)
{
	// skip the input if chat is active
	if((m_PlayerFlags&PLAYERFLAG_CHATTING) && (NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING))
		return;

	if(m_pCharacter)
		m_pCharacter->OnPredictedInput(NewInput);
}

void CPlayer::OnDirectInput(CNetObj_PlayerInput *NewInput)
{
	if(NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING && !m_pAI)
	{
		// skip the input if chat is active
		if(m_PlayerFlags&PLAYERFLAG_CHATTING)
			return;

		// reset input
		if(m_pCharacter)
			m_pCharacter->ResetInput();

		m_PlayerFlags = NewInput->m_PlayerFlags;
 		return;
	}

	m_PlayerFlags = NewInput->m_PlayerFlags;

	if(m_pCharacter)
		m_pCharacter->OnDirectInput(NewInput);

	if(!m_pCharacter && m_Team != TEAM_SPECTATORS && (NewInput->m_Fire&1))
		m_Spawning = true;

	// check for activity
	if(NewInput->m_Direction || m_LatestActivity.m_TargetX != NewInput->m_TargetX ||
		m_LatestActivity.m_TargetY != NewInput->m_TargetY || NewInput->m_Jump ||
		NewInput->m_Fire&1 || NewInput->m_Hook)
	{
		m_LatestActivity.m_TargetX = NewInput->m_TargetX;
		m_LatestActivity.m_TargetY = NewInput->m_TargetY;
		m_LastActionTick = Server()->Tick();
	}
}

CCharacter *CPlayer::GetCharacter()
{
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return 0;
}

void CPlayer::KillCharacter(int Weapon)
{
	if(m_pCharacter)
	{
		m_pCharacter->Die(m_ClientID, Weapon);
		delete m_pCharacter;
		m_pCharacter = 0;
	}
}

void CPlayer::Respawn()
{
	if(m_Team != TEAM_SPECTATORS || m_WantedTeam != TEAM_SPECTATORS)
		m_Spawning = true;
}



void CPlayer::ForceToSpectators()
{
	m_ForceToSpectators = false;
	SetTeam(TEAM_SPECTATORS, false);
	m_TeamChangeTick = Server()->Tick();
}
	
	
void CPlayer::JoinTeam()
{
	if (m_WantedTeam != m_Team)
	{
		SetTeam(m_WantedTeam, false);
		m_TeamChangeTick = Server()->Tick();
	}
}
	


void CPlayer::SetWantedTeam(int Team, bool DoChatMsg)
{
	if (Team == m_WantedTeam)
		return;
	
	char aBuf[512];
	if(DoChatMsg)
	{
		str_format(aBuf, sizeof(aBuf), "'%s' is joining the %s", Server()->ClientName(m_ClientID), GameServer()->m_pController->GetTeamName(Team));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
	
	m_WantedTeam = Team;
}
	
	
void CPlayer::SetTeam(int Team, bool DoChatMsg)
{
	// clamp the team
	Team = GameServer()->m_pController->ClampTeam(Team);
	if(m_Team == Team)
		return;

	char aBuf[512];
	
	/* skip this
	if(DoChatMsg)
	{
		str_format(aBuf, sizeof(aBuf), "'%s' joined the %s", Server()->ClientName(m_ClientID), GameServer()->m_pController->GetTeamName(Team));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
	*/

	KillCharacter();

	m_Team = Team;
	m_LastActionTick = Server()->Tick();
	m_SpectatorID = SPEC_FREEVIEW;
	// we got to wait 0.5 secs before respawning
	m_RespawnTick = Server()->Tick();
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", m_ClientID, Server()->ClientName(m_ClientID), m_Team);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	GameServer()->m_pController->OnPlayerInfoChange(GameServer()->m_apPlayers[m_ClientID]);

	if(Team == TEAM_SPECTATORS)
	{
		// update spectator modes
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_SpectatorID == m_ClientID)
				GameServer()->m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
		}
	}
}

void CPlayer::TryRespawn()
{
	vec2 SpawnPos;
	
	/*
	if (m_WantedTeam != m_Team && m_WantedTeam != TEAM_SPECTATORS && GameServer()->m_pController->CanCharacterSpawn(GetCID()))
	{
		m_Team = m_WantedTeam;
	}*/
	
	if(!GameServer()->m_pController->CanCharacterSpawn(GetCID()))
		return;
	
	if(str_comp(g_Config.m_SvGametype, "cstt") == 0)
		JoinTeam();
	
	if(!GameServer()->m_pController->CanSpawn(m_Team, &SpawnPos))
		return;
	
	m_ForceToSpectators = false;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "joining the %s", GameServer()->m_pController->GetTeamName(m_WantedTeam));
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "cstt", aBuf);
	
	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GameServer()->CreatePlayerSpawn(SpawnPos);
	
	GiveSavedWeapons();
	
	//GameServer()->ResetVotes();
}




void CPlayer::AITick()
{
	if (m_pAI)
		m_pAI->Tick();
}
	

bool CPlayer::AIInputChanged()
{
	if (m_pAI)
		return m_pAI->m_InputChanged;

	return false;
}



void CPlayer::GiveSavedWeapons()
{
	if (!GetCharacter())
		return;
	
	if (!GetCharacter()->IsAlive())
		return;
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		if (m_aSavedWeapon[i])
			GetCharacter()->GiveCustomWeapon(i);
	}
}

void CPlayer::SaveWeapons()
{
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
		m_aSavedWeapon[i] = false;
	
	if (!GetCharacter())
		return;
	
	if (!GetCharacter()->IsAlive())
		return;
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
		m_aSavedWeapon[i] = m_pCharacter->GotWeapon(i);
}


bool CPlayer::GotWeapon(int CustomWeapon)
{
	if (CustomWeapon == HAMMER_BASIC || CustomWeapon == GUN_PISTOL)
		return true;
	if (m_aSavedWeapon[CustomWeapon])
		return true;
	
	if (!GetCharacter())
		return false;
	
	return m_pCharacter->GotWeapon(CustomWeapon);
}



bool CPlayer::BuyableWeapon(int i)
{
	// check if we already have better version of the weapon
	for (int w = 0; w < NUM_CUSTOMWEAPONS; w++)
	{
		if (aCustomWeapon[w].m_Require == i && GotWeapon(w))
			return false;
	}
	
	if (!GotWeapon(i) && aCustomWeapon[i].m_Cost > 0 && (GotWeapon(aCustomWeapon[i].m_Require) || aCustomWeapon[i].m_Require == -1))
	{
		if (aCustomWeapon[i].m_Require >= 0 && WeaponDisabled(aCustomWeapon[i].m_Require))
			return false;
		
		return true;
	}
	
	return false;
}


void CPlayer::ListBuyableWeapons()
{
	/*
	if (!GetCharacter())
	{
		GameServer()->SendChatTarget(GetCID(), "You must be alive to do shopping");
		return;
	}
	*/
	
	GameServer()->SendChatTarget(GetCID(), "Available weapons to buy:");
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		if (!GotWeapon(i) && aCustomWeapon[i].m_Cost > 0 && (GotWeapon(aCustomWeapon[i].m_Require) || aCustomWeapon[i].m_Require == -1))
		{
			if (aCustomWeapon[i].m_Require >= 0 && WeaponDisabled(aCustomWeapon[i].m_Require))
				continue;
			
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s - %s - %d points", aCustomWeapon[i].m_BuyCmd, aCustomWeapon[i].m_Name, aCustomWeapon[i].m_Cost);
			GameServer()->SendChatTarget(GetCID(), aBuf);
		}
	}
}




bool CPlayer::BuyWeapon(int CustomWeapon)
{
	if (CustomWeapon < 0 || CustomWeapon >= NUM_CUSTOMWEAPONS)
		return false;

	if (!GetCharacter())
	{
		GameServer()->SendChatTarget(GetCID(), "You must be alive to do shopping");
		return false;
	}
	
	if (aCustomWeapon[CustomWeapon].m_Cost == 0 || (!GotWeapon(aCustomWeapon[CustomWeapon].m_Require) && aCustomWeapon[CustomWeapon].m_Require != -1))
		return false;
	
	if (aCustomWeapon[CustomWeapon].m_Require >= 0 && WeaponDisabled(aCustomWeapon[CustomWeapon].m_Require))
		return false;
	
	if (GotWeapon(CustomWeapon))
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "You already have  %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetCID(), aBuf);
		return false;
	}
	
	if (m_Money < aCustomWeapon[CustomWeapon].m_Cost)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Not enough money for %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetCID(), aBuf);
		return false;
	}
	
	m_pCharacter->GiveCustomWeapon(CustomWeapon);
	//m_pCharacter->SetCustomWeapon(CustomWeapon);
	m_Money -= aCustomWeapon[CustomWeapon].m_Cost;

	if (CustomWeapon != HAMMER_BASIC)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s ready for action", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetCID(), aBuf);
	}
	
	GameServer()->ResetVotes();
	
	if (GetCharacter())
	{
		if (aCustomWeapon[CustomWeapon].m_ParentWeapon == WEAPON_GRENADE)
			GameServer()->CreateSound(GetCharacter()->m_Pos, SOUND_PICKUP_GRENADE);
		else
			GameServer()->CreateSound(GetCharacter()->m_Pos, SOUND_PICKUP_SHOTGUN);
	}
	
	return true;
}








