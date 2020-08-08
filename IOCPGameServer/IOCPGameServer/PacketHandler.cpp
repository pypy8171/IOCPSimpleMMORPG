#pragma once;
#include "PacketHandler.h"
#include "ViewProcessing.h"
#include "DBHandler.h"
#include "AIHandler.h"

CPacketHandler::CPacketHandler()
{
}

CPacketHandler::~CPacketHandler()
{
}

void CPacketHandler::recv_packet_construct(int user_id, int io_byte) // io_byte는 dword이긴함
{
	//CLIENT& cu = g_clients[user_id];
	EXOVER& recv_overlapped = g_clients[user_id]->m_recv_over;

	int rest_byte = io_byte;
	char* p = recv_overlapped.io_buf;

	int packet_size = 0;

	if (0 != g_clients[user_id]->m_prev_size) packet_size = g_clients[user_id]->m_packet_buf[0];

	while (rest_byte > 0)
	{
		if (0 == packet_size) packet_size = *p;
		if (packet_size <= rest_byte + g_clients[user_id]->m_prev_size)
		{
			memcpy(g_clients[user_id]->m_packet_buf + g_clients[user_id]->m_prev_size, p, packet_size - g_clients[user_id]->m_prev_size);
			p += packet_size - g_clients[user_id]->m_prev_size;
			rest_byte -= packet_size - g_clients[user_id]->m_prev_size;
			packet_size = 0;
			process_packet(user_id, g_clients[user_id]->m_packet_buf);
			g_clients[user_id]->m_prev_size = 0;
		}
		else
		{

			memcpy(g_clients[user_id]->m_packet_buf + g_clients[user_id]->m_prev_size, p, rest_byte); // cu.m_packet_buf 이미 prev_size가 0이 아니라 그냥 쓰면 안됨
			g_clients[user_id]->m_prev_size += rest_byte;
			rest_byte = 0;
			p += rest_byte; // rest_byte가 0인데 += 의미가 잇나 위하고 바꿔야 할거같음
		}
	}
}

void CPacketHandler::process_packet(int user_id, char buf[])
{
	switch (buf[1])
	{
	case C2S_LOGIN:
	{
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buf);
		CDBHandler::GetInst()->DB_login(user_id, packet->name); // 여기서 enter
		
		addtimer(user_id, -1, OP_HEAL, 5000);
		addtimer(user_id, -1, OP_DATA_SAVE, 300000); // 5분
	}
	break;
	case C2S_MOVE:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buf);
		g_clients[user_id]->m_move_time = packet->move_time;
		
		if (high_resolution_clock::now() > g_clients[user_id]->move_delay)
		{
			g_clients[user_id]->move_delay = high_resolution_clock::now() + 100ms;
			do_move(user_id, packet->direction);
		}
	}
	break;
	case C2S_ATTACK:
	{
		cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(buf);

		if (high_resolution_clock::now() > g_clients[user_id]->attack_delay)
		{
			g_clients[user_id]->attack_delay = high_resolution_clock::now() + 1000ms;
			CAIHandler::GetInst()->do_attack(user_id);
		}
	}
	break;
	case C2S_CHAT:
	{
		cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(buf);
		send_chat_packet(-1, user_id, lstrlen(packet->message), packet->message);
	}
	break;
	case C2S_LOGOUT: // 이미 sendbyte 가 0이면 disconnect하고 있음.
	{
		cs_packet_logout* packet = reinterpret_cast<cs_packet_logout*>(buf);
		disconnect(user_id);
	}
	break;
	case C2S_TELEPORT:
	{
		//cs_packet_teleport* packet = reinterpret_cast<cs_packet_teleport*>(buf);
		g_clients[user_id]->x = 400;
		g_clients[user_id]->y = 400;
		send_move_packet(user_id, user_id);
	}
	break;
	default:
	{
		cout << "Unkown packet type error!\n";
		DebugBreak(); // 비쥬얼 스튜디오 상에서 멈추고 상태를 표시하라
		exit(-1);
	}
	}
}

void CPacketHandler::send_packet(int user_id, void* packet)
{
	unsigned char* buf = reinterpret_cast<unsigned char*>(packet);

	CLIENT* user = g_clients[user_id];

	EXOVER* exover = new EXOVER; // recv에서 사용하고 있으므로 새로 할당해서 사용
	exover->op = OP_SEND;
	ZeroMemory(&exover->over, sizeof(exover->over));
	exover->wsabuf.buf = exover->io_buf;
	exover->wsabuf.len = buf[0];

	memcpy(exover->io_buf, buf, buf[0]);

	WSASend(g_clients[user_id]->m_socket, &exover->wsabuf, 1, NULL, 0, &exover->over, NULL);
}

void CPacketHandler::enter_game(int user_id, WCHAR name[])
{
	g_clients[user_id]->m_cl.lock();
	wcscpy_s(g_clients[user_id]->name, name);
	g_clients[user_id]->name[MAX_ID_LEN] = NULL;
	send_login_ok_packet(user_id);
	g_clients[user_id]->m_status = ST_ACTIVE; // deadlock 때문에 위치 바꿈
	g_clients[user_id]->m_cl.unlock();

	CViewProcessing::GetInst()->create_nearlist_p(user_id, (g_clients[user_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
		(g_clients[user_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[user_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[user_id]->x + VIEW_RADIUS / 2) / COL_GAP);

	unordered_set <unsigned int> n_vl = g_clients[user_id]->m_nearlist.nearlist;

	for (auto& near_vl : n_vl)
	{
		if (false == is_tile(near_vl))
		{
			if (ST_SLEEP == g_clients[near_vl]->m_status)
				activate_npc(near_vl);

			if (ST_ACTIVE == g_clients[near_vl]->m_status) // 다른 스레드들이 종료하고 active를 건드려야 한다. 문제가 생기면 수정한다고 하심. 필요한 것임.
			{
				send_enter_packet(user_id, near_vl);

				if (true == is_player(near_vl))
					send_enter_packet(near_vl, user_id);
			}
		}
		else
		{
			if (ST_SLEEP == g_tile[near_vl - TILE_START_IDX]->m_status)
				activate_tile(near_vl);

			if (ST_ACTIVE == g_tile[near_vl - TILE_START_IDX]->m_status)
				send_enter_packet(user_id, near_vl);
		}
	}

	// 교수님 코드
	//for (auto &cl : g_clients) // 섹터로 가능한 부분 // 이부분도 성능에 문제 있을것.
	//{
	//	if (cl == NULL)
	//		continue;
	//
	//	int i = cl->m_id;
	//	if (user_id == i) continue;
	//	if (true == CViewProcessing::GetInst()->is_near_check_pp(user_id, i))
	//	{
	//		//g_clients[i]->m_cl.lock();
	//		if (ST_SLEEP == g_clients[i]->m_status){
	//			activate_npc(i);
	//		}
	//
	//		if (ST_ACTIVE == g_clients[i]->m_status) // 다른 스레드들이 종료하고 active를 건드려야 한다. 문제가 생기면 수정한다고 하심. 필요한 것임.
	//		{
	//			send_enter_packet(user_id, i);
	//			
	//			if(true == is_player(i))
	//				send_enter_packet(i, user_id);
	//		}
	//		//g_clients[i]->m_cl.unlock();
	//	}
	//}
}

void CPacketHandler::send_login_ok_packet(int user_id)
{
	sc_packet_login_ok packet;
	//packet.exp = 0;// g_clients[user_id]->exp;
	packet.hp = g_clients[user_id]->hp;
	packet.id = user_id;
	packet.level = g_clients[user_id]->level;
	packet.size = sizeof(packet);
	packet.type = S2C_LOGIN_OK;
	packet.x = g_clients[user_id]->x;
	packet.y = g_clients[user_id]->y;
	packet.exp = g_clients[user_id]->exp;


	send_packet(user_id, &packet);
}
static int a = 0;
void CPacketHandler::send_enter_packet(int user_id, int o_id)
{
	sc_packet_enter packet;
	packet.id = o_id;
	packet.size = sizeof(packet);
	packet.type = S2C_ENTER;

	if (!is_tile(o_id))
	{
		packet.x = g_clients[o_id]->x;
		packet.y = g_clients[o_id]->y;
		wcscpy_s(packet.name, g_clients[o_id]->name);
		packet.o_type = g_clients[o_id]->m_otype;
	}
	else
	{
		packet.x = g_tile[o_id - (TILE_START_IDX)]->x;
		packet.y = g_tile[o_id - (TILE_START_IDX)]->y;
		wcscpy_s(packet.name, g_tile[o_id - (TILE_START_IDX)]->name);
		packet.o_type = g_tile[o_id - (TILE_START_IDX)]->m_otype;
	}
	// 교수님 코드
	g_clients[user_id]->m_cl.lock();
	//int before = g_clients[user_id]->m_viewlist.viewlist.bucket_count();
	g_clients[user_id]->m_viewlist.viewlist.emplace(o_id);
	//if (g_clients[user_id]->m_viewlist.viewlist.bucket_count() == before*8)
	//{
	//	int bucketcount = g_clients[user_id]->m_viewlist.viewlist.bucket_count();

	//	a += (4 * bucketcount)*40; // 40은 set 크기
	//	cout << a << " *8 " << endl;
	//}
	//else if (g_clients[user_id]->m_viewlist.viewlist.bucket_count() == before * 2)
	//{
	//	int bucketcount = g_clients[user_id]->m_viewlist.viewlist.bucket_count();

	//	a += (4 * bucketcount) * 40;
	//	cout << a <<" *2 "<< endl;
	//}
	g_clients[user_id]->m_cl.unlock();
	send_packet(user_id, &packet);
}

void CPacketHandler::send_chat_packet(int user_id, int chatter, char len, WCHAR mess[])
{
	sc_packet_chat packet;
	packet.id = chatter;
	packet.size = len * 2 + 8;// sizeof(sc_packet_chat);
	packet.type = S2C_CHAT;
	//ZeroMemory(packet.chat, lstrlen(packet.chat));
	wcsncpy_s(packet.chat, mess, len);

	if (user_id == -1)
	{
		CViewProcessing::GetInst()->create_nearlist(chatter, (g_clients[chatter]->y - VIEW_RADIUS / 2) / ROW_GAP,
			(g_clients[chatter]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[chatter]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[chatter]->x + VIEW_RADIUS / 2) / COL_GAP);
		unordered_set <unsigned int> near_vl = g_clients[chatter]->m_nearlist.nearlist;
		for (auto& n_vl : near_vl)
		{
			if (is_player(n_vl))
				send_packet(n_vl, reinterpret_cast<char*>(&packet));
		}
	}
	else
		send_packet(user_id, reinterpret_cast<char*>(&packet)); // ? 
}

void CPacketHandler::send_move_packet(int user_id, int mover)
{
	sc_packet_move packet;
	packet.id = mover;
	packet.size = sizeof(packet);
	packet.type = S2C_MOVE;

	packet.x = g_clients[mover]->x;
	packet.y = g_clients[mover]->y;
	packet.move_time = g_clients[user_id]->m_move_time;

	send_packet(user_id, &packet);
}

void CPacketHandler::send_leave_packet(int user_id, int o_id)
{
	sc_packet_leave packet;
	packet.id = o_id;
	packet.size = sizeof(packet);
	packet.type = S2C_LEAVE;

	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_viewlist.viewlist.erase(o_id);
	g_clients[user_id]->m_cl.unlock();

	send_packet(user_id, &packet);
}

void CPacketHandler::send_login_fail_packet(int user_id)
{
	sc_packet_login_fail packet;
	packet.size = sizeof(packet);
	packet.type = S2C_LOGIN_FAIL;

	send_packet(user_id, &packet);
}

void CPacketHandler::send_stat_change(int user_id)
{
	sc_packet_stat_change packet;
	packet.id = user_id; // id 없어도 상관없음 파라미터로 들어온 곳으로 send_packet하면 되니까
	packet.size = sizeof(packet);
	packet.type = S2C_STAT_CHANGE;
	packet.hp = g_clients[user_id]->hp;
	packet.exp = g_clients[user_id]->exp;
	packet.level = g_clients[user_id]->level;
	packet.x = g_clients[user_id]->x;
	packet.y = g_clients[user_id]->y;

	send_packet(user_id, &packet);
}

void CPacketHandler::send_boss_attack(int user_id, int boss_id, char pattern) // 이미지 보여주기 패킷
{
	sc_packet_monster_attack packet;
	packet.size = sizeof(packet);
	packet.type = S2C_MONSTER_ATTACK;
	packet.boss_id = boss_id;
	packet.x = g_clients[boss_id]->x;
	packet.y = g_clients[boss_id]->y;
	packet.pattern = pattern;

	send_packet(user_id, &packet);
}

void CPacketHandler::disconnect(int user_id)
{
	send_leave_packet(user_id, user_id);
	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_status = ST_ALLOC;
	//if(!g_clients[user_id]->m_bConnected) --g_curr_user_id;
	closesocket(g_clients[user_id]->m_socket);

	for (int i = 0; i < NPC_START_IDX; ++i)
	{
		//CLIENT& cl = g_clients[i];

		if (user_id == g_clients[i]->m_id) continue; // 그래도 send_leave_packet은 보내야 함

		//cl.m_cl.lock(); // 주석
		if (ST_ACTIVE == g_clients[i]->m_status)
			send_leave_packet(g_clients[i]->m_id, user_id);
		//cl.m_cl.unlock(); // 주석
	}

	CDBHandler::GetInst()->DB_logout(user_id); // stresstest  // db_test
	g_clients[user_id]->m_status = ST_FREE;
	g_clients[user_id]->m_cl.unlock();
}

void CPacketHandler::do_move(int user_id, int direction)
{
	CLIENT& user = *g_clients[user_id];
	int x = user.x;	int y = user.y;

	switch (direction)
	{
	case D_UP: if (y > 0)	y--; break;// 0
	case D_DOWN: if (y < WORLD_HEIGHT - 1) y++; break; //		
	case D_LEFT: if (x > 0) x--; break;// 		
	case D_RIGHT: if (x < WORLD_WIDTH - 1) x++; break;// 
	default: cout << "Unkown Direction from Client move packet!\n"; 	DebugBreak(); exit(-1);
	}

	CViewProcessing::GetInst()->create_tilelist_pt(user_id, (user.y - VIEW_RADIUS / 2) / ROW_GAP,
		(user.x - VIEW_RADIUS / 2) / COL_GAP, (user.y + VIEW_RADIUS / 2) / ROW_GAP, (user.x + VIEW_RADIUS / 2) / COL_GAP);

	unordered_set <unsigned int> near_vl = user.m_nearlist.nearlist;
	for (auto& n_vl : near_vl) {
		if (is_player(n_vl))
			continue;

		if (col_check_ct(x, y, n_vl))
			return;
	}

	user.x = x;	user.y = y;

	// user가 이전 섹터를 벗어나면 이전 섹터에서 지우고 새 섹터에 집어넣는다.
	CViewProcessing::GetInst()->change_sector(user_id, user.x, user.y, user.col, user.row);

	CViewProcessing::GetInst()->create_nearlist(user_id, (user.y - VIEW_RADIUS / 2) / ROW_GAP,
		(user.x - VIEW_RADIUS / 2) / COL_GAP, (user.y + VIEW_RADIUS / 2) / ROW_GAP, (user.x + VIEW_RADIUS / 2) / COL_GAP);

	CViewProcessing::GetInst()->check_near_view(user_id);
}

void CPacketHandler::npc_move(int id, ENUMOP op) // a*알고리즘 사용시 각 타일마다 possess되어있는지 확인한다.  
{												// 길 찾는 비용이 어느정도인지 아직 모르니

	if (ENUMOP::OP_ASTAR_MOVE == op)
		return;

	if (true == g_clients[id]->is_dead)
	{
		addtimer(id, g_clients[id]->target_id, OP_RESURRECT, 30000); // target_id 필요없을듯
		return;
	}

	CAIHandler::GetInst()->reset_astar(id); // 시야 멀어져서 random_move 하면 astar reset 이게 소용 있을지는 확인

	CLIENT& user = *g_clients[id];
	int x = g_clients[id]->x;	int y = g_clients[id]->y;

	g_clients[id]->m_event.target_id = -1;

	CViewProcessing::GetInst()->create_viewlist_pn(id, (user.y - VIEW_RADIUS / 2) / ROW_GAP,
		(user.x - VIEW_RADIUS / 2) / COL_GAP, (user.y + VIEW_RADIUS / 2) / ROW_GAP, (user.x + VIEW_RADIUS / 2) / COL_GAP);

	switch (op)
	{
	case ENUMOP::OP_RANDOM_MOVE:
	{
		switch (rand() % 4)
		{
		case D_UP: if (y > 0) y--; break;
		case D_DOWN:if (y < WORLD_HEIGHT - 1) y++; break;
		case D_LEFT: if (x > 0) x--; break;
		case D_RIGHT: if (x < WORLD_WIDTH - 1) x++;	break;
		}
	}
	break;
	}

	// movenum 햇

	CViewProcessing::GetInst()->create_tilelist_nt(id, (user.y - VIEW_RADIUS / 2) / ROW_GAP,
		(user.x - VIEW_RADIUS / 2) / COL_GAP, (user.y + VIEW_RADIUS / 2) / ROW_GAP, (user.x + VIEW_RADIUS / 2) / COL_GAP);

	g_clients[id]->m_cl.lock();
	unordered_set <unsigned int> near_vl_nt = user.m_nearlist.nearlist;
	g_clients[id]->m_cl.unlock();

	for (auto& n_vl : near_vl_nt) {
		if (col_check_ct(x, y, n_vl)) {
			//if (user.m_otype == O_REDS && is_player(n_vl))
			//{
			//	WCHAR npcname[MAX_STR_LEN]{ L"" };
			//	wcscat_s(npcname, user.name); wcscat_s(npcname, L" attacked ");	wcscat_s(npcname, g_clients[n_vl]->name);	wcscat_s(npcname, L" ! ");
			//	wcscat_s(npcname, L"dealing ");	WCHAR num[5]{ L"" }; _itow_s(user.level * 3, num, 10);	wcscat_s(npcname, num);	wcscat_s(npcname, L" damages !");

			//	CPacketHandler::GetInst()->send_chat_packet(n_vl, user.m_id, lstrlen(npcname), npcname);

			//	CAIHandler::GetInst()->manage_hp(n_vl, user.level * 3);
			//	CPacketHandler::GetInst()->send_stat_change(n_vl); // hp 에서 stat change
			//	break;
			//}
			addtimer(id, -1, OP_RANDOM_MOVE, 1000); // addtimer(id, g_clients[id]->target_id, OP_RANDOM_MOVE, 1000);
			return;
		}
	}

	user.x = x;	user.y = y;

	CViewProcessing::GetInst()->change_sector(id, user.x, user.y, user.col, user.row);

	CViewProcessing::GetInst()->create_nearlist_np(id, (user.y - VIEW_RADIUS / 2) / ROW_GAP,
		(user.x - VIEW_RADIUS / 2) / COL_GAP, (user.y + VIEW_RADIUS / 2) / ROW_GAP, (user.x + VIEW_RADIUS / 2) / COL_GAP);

	unordered_set<unsigned int> near_vl = user.m_nearlist.nearlist;
	unordered_set<unsigned int> view_vl = user.m_viewlist.viewlist;

	// 시작하자마자 밖으로 나가면 viewlist nearlist 둘다 없어서 지우지를 못함.
	bool keep_alive = false;

	for (auto& n_p : near_vl)
	{
		if (ST_ACTIVE != g_clients[n_p]->m_status) continue;
		if (true == CViewProcessing::GetInst()->is_near_check_pp(n_p, id))
		{
			keep_alive = true;
			g_clients[n_p]->m_cl.lock();
			if (0 != g_clients[n_p]->m_viewlist.viewlist.count(id)) {
				g_clients[n_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_move_packet(n_p, id);
			}
			else {
				g_clients[n_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_enter_packet(n_p, id);
			}
		}
	}

	for (auto& v_p : view_vl)
	{
		if (true != CViewProcessing::GetInst()->is_near_check_pp(v_p, id)) // 근처에 없다.
		{
			g_clients[v_p]->m_cl.lock();
			if (0 != g_clients[v_p]->m_viewlist.viewlist.count(id)) { // 플레이어 viewlist에 있다
				g_clients[v_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_leave_packet(v_p, id);
			}
			else {
				g_clients[v_p]->m_cl.unlock();
			}
		}
	}

	// 근처에서 가장 가까운 애가 타겟
	if (keep_alive && g_clients[id]->m_otype == O_BOSSS && g_clients[id]->target_id != -1)
		addtimer(id, g_clients[id]->target_id, OP_BOSS_ATTACK, 2000);
	else if (keep_alive && g_clients[id]->m_otype == O_BLUES && g_clients[id]->target_id != -1)
		addtimer(id, g_clients[id]->target_id, OP_ASTAR_MOVE, 1000);
	else if (keep_alive && g_clients[id]->m_otype == O_BLUES && g_clients[id]->target_id == -1)
		addtimer(id, g_clients[id]->target_id, OP_RANDOM_MOVE, 1000);
	else if (keep_alive && g_clients[id]->m_otype == O_REDS && g_clients[id]->target_id != -1)
		addtimer(id, g_clients[id]->target_id, OP_ASTAR_MOVE, 1000);
	else if (keep_alive && g_clients[id]->m_otype == O_REDS && g_clients[id]->target_id == -1)
		g_clients[id]->is_move = false;

	else g_clients[id]->m_status = ST_SLEEP;
}

void CPacketHandler::activate_npc(int id)
{
	C_STATUS old_state = C_STATUS::ST_SLEEP;
	if (true == atomic_compare_exchange_strong(&g_clients[id]->m_status, &old_state, ST_ACTIVE)) // 한번만 addtimer진행 
	{
		if (g_clients[id]->m_otype != O_REDS && g_clients[id]->m_otype != O_NPC)
			addtimer(id, -1, OP_RANDOM_MOVE, 1000);
		g_clients[id]->is_move = false;
	}
}

void CPacketHandler::activate_tile(int id)
{
	C_STATUS old_state = C_STATUS::ST_SLEEP;
	atomic_compare_exchange_strong(&g_tile[id - (TILE_START_IDX)]->m_status, &old_state, ST_ACTIVE);
}

bool CPacketHandler::col_check_ct(int user_x, int user_y, int tile_id)
{
	if (is_tile(tile_id))
	{
		if (user_x == g_tile[tile_id - (TILE_START_IDX)]->x && user_y == g_tile[tile_id - (TILE_START_IDX)]->y)
			return true;
	}
	else
	{
		if (g_clients[tile_id]->is_dead)
			return false;

		if (user_x == g_clients[tile_id]->x && user_y == g_clients[tile_id]->y)
			return true;
	}

	return false;
}


// 객체가 담고있는걸로해도 성능이 차이가 없음
//CLIENT& cl = user;
//ZeroMemory(&cl.m_recv_over.over, sizeof(cl.m_recv_over.op));
//cl.m_recv_over.op = OP_RANDOM_MOVE_FINISH;
//cl.m_recv_over.player_id = id;
//PostQueuedCompletionStatus(g_iocp, 1, cl.m_recv_over.player_id, &cl.m_recv_over.over);

// player
//if (user.x / COL_GAP != user.col || user.y / ROW_GAP != user.row) {
//	int iPrevCol = user.col; int iPrevRow = user.row;
//
//	int iCol = user.x / COL_GAP; int iRow = user.y / ROW_GAP;
//
//	if (iCol > MAX_COL - 1)	iCol = iPrevCol;
//	if (iRow > MAX_ROW - 1)	iRow = iPrevRow;
//
//	g_sectors[iPrevRow][iPrevCol].sector_lock.lock();
//	g_sectors[iPrevRow][iPrevCol].m_setPlayerList.erase(user.m_id);
//	g_sectors[iPrevRow][iPrevCol].sector_lock.unlock();
//
//	g_sectors[iRow][iCol].sector_lock.lock();
//	g_sectors[iRow][iCol].m_setPlayerList.emplace(user.m_id);
//	user.row = iRow;	user.col = iCol; // 포인터이기에 직접 값 바꿈
//	g_sectors[iRow][iCol].sector_lock.unlock();
//}


	//if(user.m_movenum>-1)	user.m_movenum -= 1;

	//if (0 == user.m_movenum)
	//{
	//	g_clients[id]->m_cl.lock();
	//	if (false == CPacketHandler::GetInst()->is_player(id)) {
	//		EXOVER* over = new EXOVER;
	//		over->op = OP_RANDOM_MOVE_FINISH;
	//		over->player_id = id;
	//		over->target_id = g_clients[id]->m_event.target_id;
	//		PostQueuedCompletionStatus(g_iocp, 1, user.m_id, &over->over);
	//	}
	//	g_clients[id]->m_cl.unlock();
	//}