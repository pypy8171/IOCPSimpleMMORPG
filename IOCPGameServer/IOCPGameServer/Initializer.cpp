#include "Initializer.h"
#include "DBHandler.h"
#include "AIHandler.h"
#include "ScriptHandler.h"

#include <algorithm>
#include <random>

vector<int> v1;
vector<int> v2;

static int idxV1 = 0;
static int idxV2 = 0;

CInitializer::CInitializer()
{
}

CInitializer::~CInitializer()
{
}

void CInitializer::initialize_db()
{
	CDBHandler::GetInst()->init();
}

void CInitializer::initialize_sectors()
{
	cout << "start sectors initialize" << endl;
	for (int i = 0; i < MAX_COL; ++i)
	{
		for (int j = 0; j < MAX_ROW; ++j)
		{
			g_sectors[j][i].m_StartX = i * COL_GAP;
			g_sectors[j][i].m_StartY = j * ROW_GAP;
			g_sectors[j][i].m_EndX = (i + 1) * COL_GAP;
			g_sectors[j][i].m_EndY = (j + 1) * ROW_GAP;

			//g_sectors[j][i]->m_setPlayerList.reserve(400);
		}
	}
	cout << "end sectors initialize" << endl << endl;
}
//POINT pt[50000];
void CInitializer::initialize_tiles() // 파일에는 0 ~ 40000 까지는 1로 ? or 좌표로?
{
	cout << "start tiles initialize" << endl;

	for (int i = 0; i < WORLD_WIDTH; ++i)
	{
		for (int j = 0; j < WORLD_HEIGHT; ++j)
		{
			tile_alloc[j][i].m_alloc = TA_FREE;
		}
	}

	for (int i = 0; i < WORLD_WIDTH; ++i)
	{
		//if (i > 390 && i < 410)
		//	continue;

		v1.emplace_back(i);
		v2.emplace_back(i);
	}

	for (int i = 0; i < MAX_TILE_COL * MAX_TILE_ROW; ++i) // 4만개 / 200
	{
		TILE* tile = new TILE;

		tile->m_id = i;
		tile->m_status = ST_FREE;

		tile->x = 0;
		tile->y = 0;

		tile->row = tile->y / ROW_GAP;
		tile->col = tile->x / COL_GAP;

		g_tile[i] = tile;

		g_tile[i] = tile;
	}

	random_device rd;
	mt19937 g(rd());
	shuffle(v1.begin(), v1.end(), g);
	shuffle(v2.begin(), v2.end(), g);

	static int iIdx = 0;
	for (int i = 0; i < MAX_TILE_COL; ++i) // 4만개 / 200
	{
		for (int j = 0; j < MAX_TILE_ROW; ++j) // 200
		{
			if (idxV2 >= 800)
				idxV2 = 0;

			if (v1[idxV1] > 390 && v1[idxV1] < 410 && v2[idxV2] > 390 && v2[idxV2] < 410)
			{
				++idxV2;

				if (j > 0)	--j;
				else j = 0;
				continue;
			}
			if ((v1[idxV1] > 375 && v1[idxV1] < 385 && v2[idxV2] > 395 && v2[idxV2] < 405) || (v1[idxV1] > 415 && v1[idxV1] < 425 && v2[idxV2] > 395 && v2[idxV2] < 405))
			{
				++idxV2;

				if (j > 0)	--j;
				else j = 0;
				continue;
			}



			iIdx = TILE_START_IDX + i * MAX_TILE_COL + j;
			g_tile[iIdx - TILE_START_IDX]->m_status = ST_SLEEP;
			g_tile[iIdx - TILE_START_IDX]->m_id = iIdx;

			g_tile[iIdx - TILE_START_IDX]->x = v1[idxV1];
			g_tile[iIdx - TILE_START_IDX]->y = v2[idxV2];

			++idxV2;

			g_tile[iIdx - TILE_START_IDX]->m_otype = O_TILE;

			g_tile[iIdx - TILE_START_IDX]->row = g_tile[iIdx - TILE_START_IDX]->y / ROW_GAP;
			g_tile[iIdx - TILE_START_IDX]->col = g_tile[iIdx - TILE_START_IDX]->x / COL_GAP;

			short iRow = g_tile[iIdx - TILE_START_IDX]->row;
			short iCol = g_tile[iIdx - TILE_START_IDX]->col;

			g_sectors[iRow][iCol].m_setPlayerList.emplace(iIdx); // tile 삽입
		}
		//idxV2 = 0;
		++idxV1;
	}
	cout << "finish tiles initialize" << endl << endl;

	for (int i = 0; i < MAX_TILE; ++i)
	{
		if (g_tile[i]->m_id < 0)
			cout << "문제있음" << endl;
	}
}

void CInitializer::initialize_clients() // 멀티 스레드 이전 싱글 스레드에서 돌아감. 뮤텍스 필요 없음
{
	cout << "start clients initialize" << endl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		CLIENT* client = new CLIENT;

		client->m_id = i;
		client->m_status = ST_FREE;
		client->m_socket = 0;

		ZeroMemory(&client->name, sizeof(client->name));
		ZeroMemory(&client->m_packet_buf, sizeof(client->m_packet_buf));
		ZeroMemory(&client->m_recv_over.over, 255);

		client->m_viewlist.viewlist.clear();
		client->m_nearlist.nearlist.clear();

		g_clients[i] = client;
		//InitializeSRWLock(&g_clients[i]->m_viewlist.rwlock);
	}
	cout << "end clients initialize" << endl << endl;
}

void CInitializer::initialize_npcs()
{

	cout << "start npc initialize" << endl;
	for (int i = NPC_START_IDX; i < MAX_NPC + NPC_START_IDX; ++i) // 2~3만 총 1만개
	{
		CLIENT* client = new CLIENT;

		client->m_id = i;
		client->m_status = ST_FREE;
		client->m_socket = 0;

		ZeroMemory(&client->name, sizeof(client->name));
		ZeroMemory(&client->m_packet_buf, sizeof(client->m_packet_buf));
		ZeroMemory(&client->m_recv_over.over, 255);

		client->m_viewlist.viewlist.clear();
		client->m_nearlist.nearlist.clear();

		client->x = 0;
		client->y = 0;

		client->row = client->y / ROW_GAP;
		client->col = client->x / COL_GAP;

		g_clients[i] = client;
	}

	for (int i = NPC_START_IDX; i < MAX_NPC + NPC_START_IDX; ++i) // 2~3만 총 1만개
	{
		if (idxV2 >= 800)
			idxV2 = 0;

		if (v1[idxV1] > 390 && v1[idxV1] < 410 && v2[idxV2] > 390 && v2[idxV2] < 410)
		{
			++idxV2;

			if (i > NPC_START_IDX)	--i;
			else i = NPC_START_IDX;
			continue;
		}
		if ((v1[idxV1] > 375 && v1[idxV1] < 385 && v2[idxV2] > 395 && v2[idxV2] < 405) || (v1[idxV1] > 415 && v1[idxV1] < 425 && v2[idxV2] > 395 && v2[idxV2] < 405))
		{
			++idxV2;

			if (i > NPC_START_IDX)	--i;
			else i = NPC_START_IDX;
			continue;
		}

		ZeroMemory(&g_clients[i]->name, sizeof(g_clients[i]->name));
		ZeroMemory(&g_clients[i]->m_packet_buf, sizeof(g_clients[i]->m_packet_buf));
		ZeroMemory(&g_clients[i]->m_recv_over.over, 255);
		g_clients[i]->m_recv_over.wsabuf.buf = g_clients[i]->m_recv_over.io_buf;
		g_clients[i]->m_recv_over.wsabuf.len = MAX_BUF_SIZE;

		g_clients[i]->level = rand() % 10 + 1; // 레벨 다르게
		g_clients[i]->maxhp = g_clients[i]->level * 40; // 레벨마다 체력 다르게
		g_clients[i]->hp = g_clients[i]->maxhp;

		g_clients[i]->m_viewlist.viewlist.clear();
		g_clients[i]->m_nearlist.nearlist.clear();

		g_clients[i]->m_socket = 0;
		g_clients[i]->m_id = i;
		g_clients[i]->target_id = -1;

		g_clients[i]->m_status = ST_SLEEP;

		g_clients[i]->x = v1[idxV1];
		g_clients[i]->y = v2[idxV2];

		g_clients[i]->row = g_clients[i]->y / ROW_GAP;
		g_clients[i]->col = g_clients[i]->x / COL_GAP;

		++idxV2;

		if (idxV2 % 100 == 0)
		{
			++idxV1;
		}

		if (i < NPC_START_IDX + MAX_QUEST_NPC) // 고정 
		{
			if (i - NPC_START_IDX == 0)
			{
				g_clients[i]->x = 395;
				g_clients[i]->y = 395;
			}
			else if (i - NPC_START_IDX == 1)
			{
				g_clients[i]->x = 405;
				g_clients[i]->y = 405;
			}
			g_clients[i]->row = g_clients[i]->y / ROW_GAP;
			g_clients[i]->col = g_clients[i]->x / COL_GAP;
			g_clients[i]->m_otype = O_NPC; // 1
		}
		else if (i >= MAX_NPC + NPC_START_IDX - MAX_BOSS)
		{
			g_clients[i]->maxhp = 300 + g_clients[i]->level * 40;
			g_clients[i]->hp = 300 + g_clients[i]->maxhp;

			if (i == MAX_NPC + NPC_START_IDX - MAX_BOSS)
			{
				g_clients[i]->x = 380;
				g_clients[i]->y = 400;
			}
			else if (i == MAX_NPC + NPC_START_IDX - MAX_BOSS + 1)
			{
				g_clients[i]->x = 420;
				g_clients[i]->y = 400;
			}
			g_clients[i]->row = g_clients[i]->y / ROW_GAP;
			g_clients[i]->col = g_clients[i]->x / COL_GAP;
			g_clients[i]->m_otype = O_BOSSS; // 2
		}
		else
			g_clients[i]->m_otype = rand() % 2 + 2; //  // 2, 3

		WCHAR npcobj[MAX_ID_LEN]{};
		WCHAR idx[MAX_ID_LEN] = {};
		_itow_s(g_clients[i]->level, npcobj, 10);

		if (g_clients[i]->m_otype == O_NPC) {
			wprintf_s(g_clients[i]->name, "%NPC%d", i);
			WCHAR npcname[MAX_ID_LEN]{ L"LV NPC" }; // lv 없어도 됨
			wcscat_s(npcobj, npcname); wcscpy_s(g_clients[i]->name, npcobj);
		}
		else if (g_clients[i]->m_otype == O_BLUES) // 자유 분방 몬스터 , 추적
		{
			wprintf_s(g_clients[i]->name, "BLUE%d", i);
			WCHAR npcname[MAX_ID_LEN]{ L"LV BLUE" };
			wcscat_s(npcobj, npcname);	wcscpy_s(g_clients[i]->name, npcobj);
		}
		else if (g_clients[i]->m_otype == O_REDS) // 평화 몬스터, 추적
		{
			wprintf_s(g_clients[i]->name, "RED%d", i);
			WCHAR npcname[MAX_ID_LEN]{ L"LV RED" };
			wcscat_s(npcobj, npcname);	wcscpy_s(g_clients[i]->name, npcobj);
		}
		else if (g_clients[i]->m_otype == O_BOSSS)
		{
			wprintf_s(g_clients[i]->name, "BOSS%d", i);
			WCHAR npcname[MAX_ID_LEN]{ L"LV BOSS" }; // lv없어도 됨
			wcscat_s(npcobj, npcname);	wcscpy_s(g_clients[i]->name, npcobj);
		}

		short iRow = g_clients[i]->row;
		short iCol = g_clients[i]->col;

		g_sectors[iRow][iCol].m_setPlayerList.emplace(i); // npc 삽입


		int error;

		lua_State* L = g_clients[i]->L = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadfile(L, "NPC.LUA");
		lua_pcall(L, 0, 0, 0);
		lua_getglobal(L, "set_uid");
		lua_pushnumber(L, i);
		lua_pcall(L, 1, 0, 0); // L, 파라미터 개수, 리턴 개수, ,
		//lua_pop(L, 1); // getglobal한거 날려줌

		lua_getglobal(L, "save_monster_stat");
		lua_pushnumber(L, g_clients[i]->m_id);
		lua_pushnumber(L, g_clients[i]->level);
		lua_pushnumber(L, g_clients[i]->x);
		lua_pushnumber(L, g_clients[i]->y);
		lua_pushnumber(L, g_clients[i]->hp);
		lua_pushnumber(L, g_clients[i]->m_otype);
		error = lua_pcall(L, 6, 0, 0);

		if (g_clients[i]->m_otype == O_BOSSS)
		{
			lua_register(L, "API_boss_ai", API_boss_ai);
			lua_register(L, "API_boss_attack", API_boss_attack);
		}
		lua_register(L, "API_send_message", API_send_message);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);
		lua_register(L, "API_set_move_num", API_set_move_num);
		lua_register(L, "API_get_move_num", API_get_move_num);
		lua_register(L, "API_random_move", API_random_move);
		lua_register(L, "API_set_monster_stat", API_set_monster_stat);

		//lua_pushstring(L, (char*)client->name); // lua_pushstring 에 wchar가 없음

		CAIHandler::GetInst()->reset_astar(i);
	}
	cout << "finish npc initialize" << endl << endl;

	for (int i = NPC_START_IDX; i < NPC_START_IDX + MAX_NPC; ++i)
	{
		if (g_clients[i]->m_id < 0)
			cout << "문제있음" << endl;
	}
}