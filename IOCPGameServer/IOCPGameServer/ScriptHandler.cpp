#include "ScriptHandler.h"
#include "PacketHandler.h"
#include "ViewProcessing.h"
#include "AIHandler.h"

CScriptHandler::CScriptHandler()
{
}

CScriptHandler::~CScriptHandler()
{
}


int API_send_message(lua_State* L)
{
	int chatter = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	size_t isize = strlen((char*)L);
	char* message = (char*)lua_tolstring(L, -1, &isize);

	g_clients[chatter]->m_event.target_id = user_id;

	if (g_clients[chatter]->m_otype == O_NPC)
	{
		if (g_clients[chatter]->m_id == NPC_START_IDX)
		{
			if (!g_clients[user_id]->have_mon_quest && !g_clients[user_id]->clear_mon_quest)
			{
				g_clients[user_id]->have_mon_quest = true;
				WCHAR npcmessage[25] = L"plz Kill 3 Blue Slime";
				CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
			}
			else if (g_clients[user_id]->have_mon_quest && !g_clients[user_id]->clear_mon_quest)
			{
				if (g_clients[user_id]->killscore_blueslime >= 3)
				{
					CAIHandler::GetInst()->manage_exp(user_id, 200);
					g_clients[user_id]->clear_mon_quest = true;
					WCHAR npcmessage[30] = L"thx! I'll give you some exp!";
					CPacketHandler::GetInst()->send_stat_change(user_id);
					CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
				}
				else
				{
					WCHAR npcmessage[30] = L"not yet....?";
					CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
				}
			}
			else if (g_clients[user_id]->have_mon_quest && g_clients[user_id]->clear_mon_quest)
			{
				WCHAR npcmessage[30] = L"what a wonderful world!";
				CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
			}
		}
		else if (g_clients[chatter]->m_id == (NPC_START_IDX + 1))
		{
			if (!g_clients[user_id]->have_boss_quest && !g_clients[user_id]->clear_boss_quest)
			{
				g_clients[user_id]->have_boss_quest = true;
				WCHAR npcmessage[25] = L"plz Kill Boss Slime";
				CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
			}
			else if (g_clients[user_id]->have_boss_quest && !g_clients[user_id]->clear_boss_quest)
			{
				if (g_clients[user_id]->killscore_boss >= 1)
				{
					CAIHandler::GetInst()->manage_exp(user_id, 500);
					g_clients[user_id]->clear_boss_quest = true;
					WCHAR npcmessage[40] = L"thx! I'll give you some exp and damage!";
					CPacketHandler::GetInst()->send_stat_change(user_id);
					CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
				}
				else
				{
					WCHAR npcmessage[30] = L"not yet....?";
					CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
				}
			}
			else if (g_clients[user_id]->have_boss_quest && g_clients[user_id]->clear_boss_quest)
			{
				WCHAR npcmessage[30] = L"you saved our world!!";
				CPacketHandler::GetInst()->send_chat_packet(user_id, chatter, lstrlen(npcmessage), npcmessage);
			}
		}
	}

	lua_pop(L, 3);
	return 0;
}

int API_get_x(lua_State* L)
{
	int obj_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = g_clients[obj_id]->x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int obj_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = g_clients[obj_id]->y;
	lua_pushnumber(L, y);
	return 1;
}

int API_get_move_num(lua_State* L)
{
	int obj_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 1);
	lua_pushnumber(L, g_clients[obj_id]->m_movenum);
	return 1;
}

int API_set_move_num(lua_State* L)
{
	int obj_id = (int)lua_tointeger(L, -2);
	int move_num = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	g_clients[obj_id]->m_movenum = move_num;
	lua_pushnumber(L, move_num);
	return 1;
}

int API_random_move(lua_State* L)
{
	int obj_id = (int)lua_tointeger(L, -2);
	ENUMOP event_type = (ENUMOP)lua_tointeger(L, -1);
	lua_pop(L, 2);

	//if(g_clients[obj_id]->m_otype == O_NPC)
	CPacketHandler::GetInst()->npc_move(obj_id, event_type); // a*는 여기서 돌아간다. // 여기서 key는 npc id

	return 1;
}


int API_set_monster_stat(lua_State* L) // 좌표, 이름, obj_type, level 은 script로 관리
{
	int mon_id = (int)lua_tointeger(L, -6);
	g_clients[mon_id]->level = (short)lua_tointeger(L, -5);
	g_clients[mon_id]->x = (short)lua_tointeger(L, -4);
	g_clients[mon_id]->y = (short)lua_tointeger(L, -3);
	g_clients[mon_id]->hp = (short)lua_tointeger(L, -2);
	g_clients[mon_id]->m_otype = (char)lua_tointeger(L, -1);

	lua_pop(L, 6);
	return 1;
}


int API_boss_ai(lua_State* L) // 좌표, 이름, obj_type, level 은 script로 관리
{
	int pattern = (int)lua_tointeger(L, -2);
	int boss_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	CViewProcessing::GetInst()->create_nearlist_np(boss_id, (g_clients[boss_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
		(g_clients[boss_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[boss_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[boss_id]->x + VIEW_RADIUS / 2) / COL_GAP);

	unordered_set<unsigned int> near_vl = g_clients[boss_id]->m_nearlist.nearlist;
	for (auto& n_p : near_vl)
	{
		g_clients[n_p]->m_cl.lock();
		if (n_p == boss_id)
		{
			g_clients[n_p]->m_cl.unlock();
			continue;
		}
		g_clients[n_p]->m_cl.unlock();
		CPacketHandler::GetInst()->send_boss_attack(n_p, boss_id, pattern);
	}

	addtimer(boss_id, -1, OP_RANDOM_MOVE, 1000);

	return 1;
}

int API_boss_attack(lua_State* L)
{
	int boss_id = (int)lua_tointeger(L, -4);
	int user_id = (int)lua_tointeger(L, -3);
	int x = (int)lua_tointeger(L, -2);
	int y = (int)lua_tointeger(L, -1);
	lua_pop(L, 4);

	CViewProcessing::GetInst()->create_nearlist_np(boss_id, (g_clients[boss_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
		(g_clients[boss_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[boss_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[boss_id]->x + VIEW_RADIUS / 2) / COL_GAP);

	unordered_set<unsigned int> near_vl = g_clients[boss_id]->m_nearlist.nearlist;
	for (auto& n_p : near_vl)
	{
		g_clients[n_p]->m_cl.lock();
		if (n_p == boss_id)
		{
			g_clients[n_p]->m_cl.unlock();
			continue;
		}
		g_clients[n_p]->m_cl.unlock();
		if (x == g_clients[n_p]->x && y == g_clients[n_p]->y)
		{
			CAIHandler::GetInst()->manage_hp(n_p, 50 + g_clients[boss_id]->level * 4); // 보스 공격력은 임의로 줬음

			WCHAR bossstr[MAX_STR_LEN]{ L"" };
			wcscat_s(bossstr, g_clients[boss_id]->name); wcscat_s(bossstr, L" attacked ");	wcscat_s(bossstr, g_clients[n_p]->name);	wcscat_s(bossstr, L" ! ");
			wcscat_s(bossstr, L"dealing ");	WCHAR num[5]{ L"" }; _itow_s(50 + g_clients[boss_id]->level * 4, num, 10);	wcscat_s(bossstr, num);	wcscat_s(bossstr, L" damages !");
			CPacketHandler::GetInst()->send_chat_packet(n_p, boss_id, lstrlen(bossstr), bossstr);

			CPacketHandler::GetInst()->send_stat_change(n_p);
		}
	}

	return 1;
}
