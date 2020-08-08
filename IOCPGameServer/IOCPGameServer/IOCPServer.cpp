#pragma once;
//SQLConnect()
#include "extern.h"
#include "ViewProcessing.h"
#include "PacketHandler.h"
#include "DBHandler.h"
#include "AIHandler.h"
#include "Initializer.h"

#include <algorithm>
#include <random>

#include <thread>

int a = 0;
SECTOR g_sectors[MAX_ROW][MAX_COL];
CLIENT* g_clients[MAX_USER + MAX_NPC/* + MAX_TILE*/];
TILE* g_tile[MAX_TILE];
TILE_ALLOC tile_alloc[WORLD_HEIGHT][WORLD_WIDTH];

priority_queue<Event_Type*, vector<Event_Type*>, Compare> timer_queue;
mutex timer_lock;

HANDLE g_iocp;
SOCKET l_socket;

int g_curUser = 0;

SQLHENV henv;
SQLHDBC hdbc;
SQLHSTMT hstmt = 0;
SQLRETURN retcode;
SQLWCHAR dUser_name[MAX_NAME_LEN];
SQLINTEGER dUser_id, dUser_Level, dUser_posx, dUser_posy, dUser_hp, dUser_maxhp, dUser_exp, dUser_maxexp, dUser_monquest, dUser_monquestprocess, dUser_bossquest, dUser_bossquestprocess, dUser_normaldamage;
SQLLEN cbName = 0, cbID = 0, cbLevel = 0, cbPosx, cbPosy, cbHp, cbMaxHp, cbExp, cbMaxExp, cbMonQuest, cbMonQuestProcess, cbBossQuest, cbBossQuestProcess, cbUserNormalDamage;


void event_process(ENUMOP exover, int user_id, int player_id, int target_id);

void worker_thread();

void show_error() {
	printf("error\n");
}



void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void addtimer(int id, int target, ENUMOP op, unsigned int timer) // 이벤트 new delete 안하고 재사용 하는것이 얼마나 성능이 좋아지는지 확인해야함
{
	Event_Type* event_type = new Event_Type;
	event_type->obj_id = id;
	event_type->event_id = op;
	event_type->wakeup_time = high_resolution_clock::now() + milliseconds(timer);
	event_type->target_id = target;

	g_clients[id]->m_event.target_id = target;

	timer_lock.lock();
	timer_queue.emplace(event_type);
	timer_lock.unlock();
}

void event_process(ENUMOP event_type, int user_id, int player_id, int target_id)
{

	switch (event_type) // 플레이어 move, 플레이어 attack도 여기 들어와야할듯
	{
	case OP_RANDOM_MOVE_FINISH:
	{
		g_clients[user_id]->lua_l.lock();
		lua_State* L = g_clients[user_id]->L;
		lua_getglobal(L, "event_ai_finish");
		lua_pushnumber(L, user_id); // exover->player_id
		lua_pushnumber(L, target_id);
		int error = lua_pcall(L, 2, 0, 0);
		if (error) cout << lua_tostring(L, -1) << endl;
		g_clients[user_id]->lua_l.unlock();
	}
	break;
	case OP_RANDOM_MOVE:
	{
		//g_clients[user_id]->lua_l.lock();
		//lua_State* L = g_clients[user_id]->L;
		//lua_getglobal(L, "event_ai_move");
		//lua_pushnumber(L, player_id);
		//lua_pushnumber(L, event_type);
		//int error = lua_pcall(L, 2, 0, 0);
		//if (error) cout << lua_tostring(L, -1) << endl;
		//g_clients[user_id]->lua_l.unlock();
		CPacketHandler::GetInst()->npc_move(user_id, event_type); // a*는 여기서 돌아간다. // 여기서 key는 npc id

	}
	break;
	case OP_ASTAR_MOVE: // 몬스터 astar
	{
		CAIHandler::GetInst()->astar_finding(user_id, g_clients[user_id]->target_id, event_type); // user_id는 npc, target은 client
	}
	break;
	case OP_ENEMY_ATTACK: // 몬스터 일반공격
	{
		if (g_clients[user_id]->m_event.target_id == -1) 
			break;

		if (!g_clients[g_clients[user_id]->m_event.target_id]->is_dead && !g_clients[user_id]->is_dead)
		{
			int exp = g_clients[user_id]->level * 5;
			if (g_clients[user_id]->m_otype == O_BLUES)
				exp *= 2;

			WCHAR npcname[MAX_STR_LEN]{ L"" };
			wcscat_s(npcname, g_clients[user_id]->name); wcscat_s(npcname, L" attacked ");	wcscat_s(npcname, g_clients[g_clients[user_id]->m_event.target_id]->name);	wcscat_s(npcname, L" ! ");
			wcscat_s(npcname, L"dealing ");	WCHAR num[5]{ L"" }; _itow_s(exp * 0.4, num, 10);	wcscat_s(npcname, num);	wcscat_s(npcname, L" damages !");

			CPacketHandler::GetInst()->send_chat_packet(g_clients[user_id]->m_event.target_id, user_id, lstrlen(npcname), npcname);


			CAIHandler::GetInst()->manage_hp(g_clients[user_id]->m_event.target_id, exp * 0.4);
			CPacketHandler::GetInst()->send_stat_change(g_clients[user_id]->m_event.target_id); // hp 에서 stat change

			char dist = CAIHandler::GetInst()->get_dist(user_id, g_clients[user_id]->m_event.target_id);

			// active라면
			if (dist == 1)	addtimer(user_id, g_clients[user_id]->m_event.target_id, OP_ENEMY_ATTACK, 1000);
			else if (CAIHandler::GetInst()->is_range(user_id, g_clients[user_id]->m_event.target_id)) addtimer(user_id, g_clients[user_id]->m_event.target_id, OP_ASTAR_MOVE, 1000);
			else if (g_clients[user_id]->m_otype == O_REDS) g_clients[user_id]->is_move = false;
			else addtimer(user_id, -1, OP_RANDOM_MOVE, 1000);
		}
		else if (g_clients[g_clients[user_id]->m_event.target_id]->is_dead && !g_clients[user_id]->is_dead)
		{
			g_clients[user_id]->m_event.target_id = -1;
			if (g_clients[user_id]->m_otype == O_REDS) g_clients[user_id]->is_move = false;
			else if(g_clients[user_id]->m_otype == O_REDS)  addtimer(user_id, -1, OP_RANDOM_MOVE, 1000);
		}
	}
	break;
	case OP_BOSS_ATTACK:
	{
		g_clients[user_id]->lua_l.lock();
		lua_State* L = g_clients[user_id]->L;
		lua_getglobal(L, "boss_ai");
		lua_pushnumber(L, target_id);
		int error = lua_pcall(L, 1, 0, 0);
		if (error) cout << lua_tostring(L, -1) << endl;
		g_clients[user_id]->lua_l.unlock();
	}
	break;
	case OP_RESURRECT: 
	{
		g_clients[user_id]->hp = g_clients[user_id]->maxhp;

		g_clients[user_id]->m_cl.lock();
		if (g_clients[user_id]->m_otype == O_PLAYER)
		{											
			g_clients[user_id]->is_dead = false;
			g_clients[user_id]->m_cl.unlock();

			g_clients[user_id]->exp *= 0.5;
			g_clients[user_id]->x = START_POINT_X;
			g_clients[user_id]->y = START_POINT_Y;
			
			CViewProcessing::GetInst()->change_sector(user_id, g_clients[user_id]->x, g_clients[user_id]->y, g_clients[user_id]->col, g_clients[user_id]->row);

			CViewProcessing::GetInst()->create_nearlist(user_id, (g_clients[user_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
				(g_clients[user_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[user_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[user_id]->x + VIEW_RADIUS / 2) / COL_GAP);

			CViewProcessing::GetInst()->check_near_view(user_id);
			
			CPacketHandler::GetInst()->send_stat_change(user_id); // hp 에서 stat change

			WCHAR str[10] = { L" alive" };
			WCHAR num[15]{ L"" };
			_itow_s(user_id, num, 10);
			wcscat_s(num, str);

			CPacketHandler::GetInst()->send_chat_packet(user_id, SERVER_CHATTER, lstrlen(num), num);
		}
		else
		{
			//g_clients[user_id]->m_event.target_id = -1;
			g_clients[user_id]->is_dead = false;
			g_clients[user_id]->m_cl.unlock();

			CViewProcessing::GetInst()->create_nearlist_np(user_id, (g_clients[user_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
				(g_clients[user_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[user_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[user_id]->x + VIEW_RADIUS / 2) / COL_GAP);

			unordered_set<unsigned int> near_vl = g_clients[user_id]->m_nearlist.nearlist;
			for (auto& n_p : near_vl)
			{
				g_clients[n_p]->m_cl.lock();
				if (n_p == user_id)
				{
					g_clients[n_p]->m_cl.unlock();
					continue;
				}
				g_clients[n_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_leave_packet(n_p, user_id);
			}

			g_clients[user_id]->lua_l.lock();
			lua_getglobal(g_clients[user_id]->L, "set_monster_stat");
			lua_pcall(g_clients[user_id]->L, 0, 0, 0);
			g_clients[user_id]->lua_l.unlock();

			CViewProcessing::GetInst()->change_sector(user_id, g_clients[user_id]->x, g_clients[user_id]->y, g_clients[user_id]->col, g_clients[user_id]->row);

			CViewProcessing::GetInst()->create_nearlist_np(user_id, (g_clients[user_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
				(g_clients[user_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[user_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[user_id]->x + VIEW_RADIUS / 2) / COL_GAP);

			unordered_set<unsigned int> near_vl2 = g_clients[user_id]->m_nearlist.nearlist;

			if (near_vl2.size() == 0)
				g_clients[user_id]->m_status = ST_SLEEP;
			else
			{
				for (auto& n_p : near_vl2)
				{
					g_clients[n_p]->m_cl.lock();
					if (n_p == user_id)
					{
						g_clients[n_p]->m_cl.unlock();
						continue;
					}
					g_clients[n_p]->m_cl.unlock();
					CPacketHandler::GetInst()->send_enter_packet(n_p, user_id);
				}
			}

			if (g_clients[user_id]->m_status != ST_SLEEP)
			{
				if (g_clients[user_id]->m_otype == O_BLUES)
					addtimer(user_id, -1, OP_RANDOM_MOVE, 1000);
				else if (g_clients[user_id]->m_otype == O_BOSSS)
					addtimer(user_id, -1, OP_RANDOM_MOVE, 1000);
				else if (g_clients[user_id]->m_otype == O_REDS)
					g_clients[user_id]->is_move = false;
			}
		}
	}
	break;
	}
}

void timer() // 스레드 
{
	while (true)
	{
		this_thread::sleep_for(1ms);// Sleep 은 윈도우에서만 가능한거임.
		while (true) {
			timer_lock.lock();
			if (timer_queue.empty())
			{
				timer_lock.unlock();
				break;
			}

			if (timer_queue.top()->wakeup_time > high_resolution_clock::now())
			{
				timer_lock.unlock();
				break;
			}

			Event_Type* tEvent = timer_queue.top();
			timer_queue.pop();
			timer_lock.unlock();

			EXOVER* exover = new EXOVER;
			exover->op = tEvent->event_id;

			exover->player_id = tEvent->obj_id;
			exover->target_id = tEvent->target_id; // 이거하면 안움직임
			PostQueuedCompletionStatus(g_iocp, 1, exover->player_id, &exover->over);

			delete tEvent;
		}
	}
}

void worker_thread()
{
	while (true)
	{
		DWORD io_byte;
		ULONG_PTR key = 0;
		WSAOVERLAPPED* over = nullptr;
		GetQueuedCompletionStatus(g_iocp, &io_byte, &key, &over, INFINITE); // accept와 같이 블로킹 -> acceptex사용
		// acceptex하면 달라지는게 뭐지 -> 강의 다시 보기

		EXOVER* exover = reinterpret_cast<EXOVER*>(over);
		int user_id = static_cast<int>(key);

		//CLIENT& cl = g_clients[user_id];

		switch (exover->op)
		{
		case OP_PLAYER_MOVE: // 이런 이벤트만 처리하게 묶기 // send recv accept eventprocess 이렇게만.
		{
			//g_clients[user_id]->m_cl.unlock();
			g_clients[user_id]->lua_l.lock(); // npc id
			lua_State* L = g_clients[user_id]->L;
			lua_getglobal(L, "event_player_move");
			lua_pushnumber(L, exover->player_id);  // client id
			int error = lua_pcall(L, 1, 0, 0);
			if (error) cout << lua_tostring(L, -1) << endl;
			g_clients[user_id]->lua_l.unlock();

			delete exover;
		}
		break;
		case OP_DATA_SAVE:
		{
			CDBHandler::GetInst()->DB_save_userdata(user_id); // db_test
			addtimer(user_id, -1, OP_DATA_SAVE, 300000);
			delete exover;
		}
		break;
		case OP_HEAL:
		{
			CAIHandler::GetInst()->heal(user_id);
			addtimer(user_id, -1, OP_HEAL, 5000);
			delete exover;
		}
		break;
		case OP_RANDOM_MOVE_FINISH:
		case OP_RANDOM_MOVE:
		case OP_ASTAR_MOVE:
		case OP_ENEMY_ATTACK:
		case OP_RESURRECT:
		case OP_BOSS_ATTACK:
		{
			if (user_id != -1)
			{
				event_process(exover->op, user_id, exover->player_id, exover->target_id);
			}
			delete exover;
		}
		break;

		case OP_RECV:
		{
			if (0 == io_byte)
				CPacketHandler::GetInst()->disconnect(user_id);
			else
			{
				CPacketHandler::GetInst()->recv_packet_construct(user_id, io_byte);
				//process_packet(user_id, exover->io_buf);
				ZeroMemory(&g_clients[user_id]->m_recv_over.over, sizeof(g_clients[user_id]->m_recv_over.op));
				DWORD flags = 0;
				WSARecv(g_clients[user_id]->m_socket, &g_clients[user_id]->m_recv_over.wsabuf, 1, NULL, &flags, &g_clients[user_id]->m_recv_over.over, NULL);
			}
		}
		break;
		case OP_SEND:
			if (0 == io_byte)
			{
				CPacketHandler::GetInst()->disconnect(user_id);
			}
			else
			{
				delete exover;
			}
			break;
		case OP_ACCEPT: // 비동기 accept 하나만 실행. main에서 비동기 accept 하고 worker thread에서 완료를 하고 // 4.28 강의 두번째 3분
						// 완료한 thread에서 accept할때까지 다른 thread는 완료 안됨. 따라서 싱글스레드로 진행이됨.
		{
			int user_idd = -1;
			for (int i = 0; i < MAX_USER; ++i)
			{
				lock_guard<mutex> gl{ g_clients[i]->m_cl }; // unlcok 이 필요 없다. 블록에서 빠져나갈때 unlock 을 해줌, 루프를 매번 해줄때 unlock을 해주고 lock을 건다.
				if (ST_FREE == g_clients[i]->m_status) //  template 객체. 생성자 소멸자 호출할때 lock unlock 함. 함수 or 블록 부분에 사용하면 유용하다
				{
					int cur = i;

					g_clients[i]->m_status = ST_ALLOC;
					user_idd = i;

					if (g_curUser <= cur)
						++g_curUser;
					break;
				}
			}

			SOCKET	c_socket = exover->c_socket;

			if (-1 == user_idd) //send_login_fail_packet;
				closesocket(exover->c_socket);
			else
			{
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_iocp, user_idd, 0);

				//CLIENT& NewClient = g_clients[user_id];

				wcscpy_s(g_clients[user_idd]->name, L"USER");
				g_clients[user_idd]->m_id = user_idd;
				g_clients[user_idd]->m_prev_size = 0;
				g_clients[user_idd]->m_recv_over.op = OP_RECV;

				ZeroMemory(&g_clients[user_idd]->m_recv_over.over, sizeof(g_clients[user_idd]->m_recv_over.op));
				g_clients[user_idd]->m_recv_over.wsabuf.buf = g_clients[user_idd]->m_recv_over.io_buf;
				g_clients[user_idd]->m_recv_over.wsabuf.len = MAX_BUF_SIZE;
				g_clients[user_idd]->m_socket = c_socket;
				g_clients[user_idd]->m_viewlist.viewlist.clear();
				g_clients[user_idd]->x = rand() % WORLD_WIDTH;
				g_clients[user_idd]->y = rand() % WORLD_WIDTH;
				g_clients[user_idd]->level = 1;
				g_clients[user_idd]->hp = 30000;
				g_clients[user_idd]->maxhp = 30000;
				g_clients[user_idd]->exp = 0;
				g_clients[user_idd]->maxexp = 0;
				g_clients[user_idd]->clear_mon_quest = false;
				g_clients[user_idd]->killscore_blueslime = 0;
				g_clients[user_idd]->clear_boss_quest = false;
				g_clients[user_idd]->killscore_boss = 0;
				g_clients[user_idd]->damage = 10;

				g_clients[user_idd]->m_otype = O_PLAYER;

				DWORD flags = 0;
				WSARecv(c_socket, &g_clients[user_idd]->m_recv_over.wsabuf, 1, NULL, &flags, &g_clients[user_idd]->m_recv_over.over, NULL);

				//// sector에 집어 넣어준다.
				short col = g_clients[user_idd]->x / COL_GAP;
				short row = g_clients[user_idd]->y / ROW_GAP;
				g_sectors[row][col].m_setPlayerList.emplace(g_clients[user_idd]->m_id);
				g_clients[user_idd]->row = row;
				g_clients[user_idd]->col = col;

			}

			c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

			exover->c_socket = c_socket;
			ZeroMemory(&exover->over, sizeof(exover->over));
			AcceptEx(l_socket, exover->c_socket, exover->io_buf, NULL,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &exover->over);

		}
		break;
		}
	}
}

int main()
{
	wcout.imbue(locale("korean"));

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	l_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN s_address; // 프로토콜, 인터넷 포트, 클라 서버 주소체계
	memset(&s_address, 0, sizeof(s_address));
	//fill_n(&s_address, sizeof(s_address), 0);
	s_address.sin_family = AF_INET; // ipv4 인터넷 주소체계
	s_address.sin_port = htons(SERVER_PORT); // htons사용 이유 // 16비트 포트정보
	s_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // 32비트 ip정보
	::bind(l_socket, reinterpret_cast<sockaddr*>(&s_address), sizeof(s_address)); // error +
	// c++11 bind가 아닌 bind를 사용하기 위해 ::

	listen(l_socket, SOMAXCONN); // error + 

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	CInitializer::GetInst()->initialize_sectors();
	CInitializer::GetInst()->initialize_clients();
	CInitializer::GetInst()->initialize_tiles();
	CInitializer::GetInst()->initialize_npcs();
	CInitializer::GetInst()->initialize_db();  // db_test

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(l_socket), g_iocp, 999, 0); // 등록

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	EXOVER accept_over;

	ZeroMemory(&accept_over.over, sizeof(accept_over.over));
	accept_over.op = OP_ACCEPT;
	accept_over.c_socket = c_socket;
	AcceptEx(l_socket, accept_over.c_socket, accept_over.io_buf, NULL,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &accept_over.over);
	// 반환이 아니라 초기화된 소켓을 넣음

	vector<thread> worker_threads;
	for (int i = 0; i < 4; ++i) worker_threads.emplace_back(worker_thread);
	thread timer_thread(timer);

	timer_thread.join();
	for (auto& threads : worker_threads) threads.join();


	for (int i = 0; i < MAX_USER + MAX_NPC /*+ MAX_TILE*/; ++i)
	{
		if (g_clients[i] != nullptr)
		{
			delete g_clients[i];
			g_clients[i] = nullptr;
		}
	}

	CDBHandler::GetInst()->DB_finish(); // db_test

	CloseHandle(g_iocp); // 추가
}

//g_clients[user_id]->m_viewlist.viewlist.clear();
//CViewProcessing::GetInst()->create_nearlist_p(user_id, (g_clients[user_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
//	(g_clients[user_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[user_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[user_id]->x + VIEW_RADIUS / 2) / COL_GAP);

//unordered_set<unsigned int> near_vl = g_clients[user_id]->m_nearlist.nearlist;
//for (auto& n_p : near_vl)
//{
//	CPacketHandler::GetInst()->send_leave_packet(user_id, n_p); // CPacketHandler::GetInst()->send_leave_packet(n_p, user_id);
//	if (n_p == user_id || CPacketHandler::GetInst()->is_tile(n_p))
//	{
//		continue;
//	}

//	if (g_clients[n_p]->m_otype != O_PLAYER)
//	{
//		CPacketHandler::GetInst()->send_leave_packet(n_p, user_id);
//		CAIHandler::GetInst()->reset_astar(n_p);
//		g_clients[n_p]->m_status = ST_SLEEP;
//		g_clients[n_p]->is_move = false;
//	}
//}
	//CViewProcessing::GetInst()->change_sector(user_id, g_clients[user_id]->x, g_clients[user_id]->y, g_clients[user_id]->col, g_clients[user_id]->row);

	//CViewProcessing::GetInst()->create_nearlist_p(user_id, (g_clients[user_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
	//	(g_clients[user_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[user_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[user_id]->x + VIEW_RADIUS / 2) / COL_GAP);

	//unordered_set<int> near_vl2 = g_clients[user_id]->m_nearlist.nearlist;

	//for (auto& near_vl : near_vl2)
	//{
	//	if (ST_SLEEP == g_clients[near_vl]->m_status) {
	//		CPacketHandler::GetInst()->activate_npc(near_vl); // 일케 하면 연속으로 움직임
	//		//g_clients[near_vl]->m_status = ST_ACTIVE; // 일케하면 죽고 시야에 보이는 애가 안움직임
	//		//CAIHandler::GetInst()->reset_astar(near_vl);
	//		//addtimer(near_vl, -1, OP_RANDOM_MOVE, 1000);
	//	}

	//	if (ST_ACTIVE == g_clients[near_vl]->m_status) // 다른 스레드들이 종료하고 active를 건드려야 한다. 문제가 생기면 수정한다고 하심. 필요한 것임.
	//	{
	//		CPacketHandler::GetInst()->send_enter_packet(user_id, near_vl);

	//		if (true == CPacketHandler::GetInst()->is_player(near_vl))
	//			CPacketHandler::GetInst()->send_enter_packet(near_vl, user_id);
	//	}
	//}