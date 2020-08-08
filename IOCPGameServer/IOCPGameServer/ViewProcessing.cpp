#pragma once
#include "ViewProcessing.h"
#include "extern.h"
#include "PacketHandler.h"


CViewProcessing::CViewProcessing()
{
}

CViewProcessing::~CViewProcessing()
{
}

void CViewProcessing::create_nearlist(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	g_clients[user_id]->m_cl.lock(); // 둘
	unordered_set <unsigned int> near_vl;
	near_vl.clear();
	g_clients[user_id]->m_nearlist.nearlist.clear();
	g_clients[user_id]->m_cl.unlock();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;	int iRow = -1;

			iCol = i;	iRow = j;

			if (iCol < 0) iCol = 0;
			if (iRow < 0) iRow = 0;

			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) { // 아이디문제
				if (false == CPacketHandler::GetInst()->is_tile(playerlist))
				{
					if (is_near_check_pp(user_id, playerlist)) {
						if (CPacketHandler::GetInst()->is_npc(playerlist) || CPacketHandler::GetInst()->is_monster(playerlist)) { // 이거도 timer queue에 넣어야 1초에 한번 반응할듯
							EXOVER* over = new EXOVER;
							over->op = OP_PLAYER_MOVE;
							over->player_id = user_id;
							PostQueuedCompletionStatus(g_iocp, 1, g_clients[playerlist]->m_id, &over->over);
						}
						near_vl.emplace(playerlist);
					}
				}
				else
				{
					if (is_neartile_check_pt(user_id, playerlist))
						near_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}

	g_clients[user_id]->m_cl.lock(); // 넷
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}

void CViewProcessing::create_nearlist_p(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	g_clients[user_id]->m_cl.lock(); // 둘
	unordered_set <unsigned int> near_vl;
	near_vl.clear();
	g_clients[user_id]->m_nearlist.nearlist.clear();
	g_clients[user_id]->m_cl.unlock();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;	int iRow = -1;
			iCol = i; iRow = j;

			if (iCol < 0) iCol = 0;
			if (iRow < 0) iRow = 0;

			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (false == CPacketHandler::GetInst()->is_tile(playerlist)) {
					if (is_near_check_pp(user_id, playerlist)) {
						near_vl.emplace(playerlist);
					}
				}
				else
				{
					if (is_neartile_check_pt(user_id, playerlist)) {
						near_vl.emplace(playerlist);
					}
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}

	g_clients[user_id]->m_cl.lock(); // 넷
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}

void CViewProcessing::create_nearlist_pp(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status) return;

	g_clients[user_id]->m_cl.lock(); 
	unordered_set <unsigned int> near_vl;
	near_vl.clear();
	g_clients[user_id]->m_cl.unlock();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;	int iRow = -1;
			iCol = i; iRow = j;

			if (iCol < 0) iCol = 0;
			if (iRow < 0) iRow = 0;

			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock();
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (CPacketHandler::GetInst()->is_player(playerlist)) {
					if (is_near_check_pp(user_id, playerlist)) {
						near_vl.emplace(playerlist);
					}
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}

	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}

void CViewProcessing::create_nearlist_np(int npc_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[npc_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	unordered_set <unsigned int> near_vl;
	near_vl.clear();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1; int iRow = -1;
			iCol = i; iRow = j;

			if (iCol < 0) iCol = 0;
			if (iRow < 0) iRow = 0;

			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (CPacketHandler::GetInst()->is_player(playerlist)) {
					if (is_near_check_pp(npc_id, playerlist))
						near_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}

	g_clients[npc_id]->m_nearlist.nearlist = near_vl;
}

void CViewProcessing::create_nearlist_pn(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status)
		return;

	g_clients[user_id]->m_cl.lock();
	unordered_set <unsigned int> near_vl;
	near_vl.clear();
	g_clients[user_id]->m_cl.unlock();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1; int iRow = -1;
			iCol = i; iRow = j;

			if (iCol < 0) iCol = 0;	if (iRow < 0) iRow = 0;
			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (CPacketHandler::GetInst()->is_npc(playerlist) || CPacketHandler::GetInst()->is_monster(playerlist)) {
					if (is_attack_range_pn(user_id, playerlist))
						near_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}

	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}

void CViewProcessing::create_viewlist_pn(int npc_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[npc_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	unordered_set <unsigned int> view_vl;
	view_vl.clear();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;	int iRow = -1;
			iCol = i;	iRow = j;

			if (iCol < 0) iCol = 0;
			if (iRow < 0) iRow = 0;

			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (CPacketHandler::GetInst()->is_player(playerlist)) {
					if (is_near_check_pp(npc_id, playerlist))
						view_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}
	g_clients[npc_id]->m_viewlist.viewlist = view_vl;
}

void CViewProcessing::create_tilelist_pt(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	unordered_set <unsigned int> near_vl;
	near_vl.clear();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;
			int iRow = -1;

			iCol = i;
			iRow = j;

			if (iCol < 0) iCol = 0;
			if (iRow < 0) iRow = 0;

			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;
			if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (is_obstacle_check(user_id, playerlist)) {
					near_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}

	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}

void CViewProcessing::create_tilelist_nt(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	unordered_set <unsigned int> near_vl;
	near_vl.clear();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;	int iRow = -1;
			iCol = i; iRow = j;

			if (iCol < 0) iCol = 0;	if (iRow < 0) iRow = 0;
			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;	if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (is_obstacle_check(user_id, playerlist)) { // create_obstacle_nt와 같음 지금
					near_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}
	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}

void CViewProcessing::create_obstacle_nt(int user_id, int row_start, int col_start, int row_end, int col_end)
{
	if (ST_ACTIVE != g_clients[user_id]->m_status) // 시작시 npc띄우는것때메  // 이거 안하면 터짐
		return;

	unordered_set <unsigned int> near_vl;
	near_vl.clear();

	for (int i = col_start; i <= col_end; ++i) {
		for (int j = row_start; j <= row_end; ++j) {
			int iCol = -1;	int iRow = -1;
			iCol = i; iRow = j;

			if (iCol < 0) iCol = 0;	if (iRow < 0) iRow = 0;
			if (iCol > MAX_COL - 1)	iCol = MAX_COL - 1;	if (iRow > MAX_ROW - 1) iRow = MAX_ROW - 1;

			g_sectors[iRow][iCol].sector_lock.lock(); // 섹터락
			for (auto& playerlist : g_sectors[iRow][iCol].m_setPlayerList) {
				if (is_obstacle_check(user_id, playerlist))
				{
					near_vl.emplace(playerlist);
				}
			}
			g_sectors[iRow][iCol].sector_lock.unlock();
		}
	}
	g_clients[user_id]->m_cl.lock();
	g_clients[user_id]->m_nearlist.nearlist = near_vl;
	g_clients[user_id]->m_cl.unlock();
}



void CViewProcessing::check_near_view(int user_id)
{
	g_clients[user_id]->m_cl.lock();
	unordered_set <unsigned int> old_vl = g_clients[user_id]->m_viewlist.viewlist;
	unordered_set <unsigned int> new_vl = g_clients[user_id]->m_nearlist.nearlist;
	g_clients[user_id]->m_cl.unlock();

	for (auto np : new_vl)
	{
		if (0 == old_vl.count(np)) 
		{
			CPacketHandler::GetInst()->send_enter_packet(user_id, np);
			if (false == CPacketHandler::GetInst()->is_player(np)) {
				if (!CPacketHandler::GetInst()->is_tile(np))
					CPacketHandler::GetInst()->activate_npc(np);
				else 
				{
					CPacketHandler::GetInst()->activate_tile(np);
					continue;
				}
				continue;
			}

			g_clients[np]->m_cl.lock();
			if (0 == g_clients[np]->m_viewlist.viewlist.count(user_id)) {
				g_clients[np]->m_cl.unlock();
				CPacketHandler::GetInst()->send_enter_packet(np, user_id);
			}
			else {
				g_clients[np]->m_cl.unlock();
				CPacketHandler::GetInst()->send_move_packet(np, user_id);
			}
		}
		else
		{
			if (false == CPacketHandler::GetInst()->is_player(np)) continue;

			g_clients[np]->m_cl.lock();
			if (0 != g_clients[np]->m_viewlist.viewlist.count(user_id)) {
				g_clients[np]->m_cl.unlock();
				CPacketHandler::GetInst()->send_move_packet(np, user_id);
			}
			else {
				g_clients[np]->m_cl.unlock();
				CPacketHandler::GetInst()->send_enter_packet(np, user_id);
			}
		}
	}

	for (auto old_p : old_vl)
	{
		if (0 == new_vl.count(old_p))
		{
			CPacketHandler::GetInst()->send_leave_packet(user_id, old_p);
			if (false == CPacketHandler::GetInst()->is_player(old_p)) continue;

			g_clients[old_p]->m_cl.lock();
			if (0 != g_clients[old_p]->m_viewlist.viewlist.count(user_id)) {
				g_clients[old_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_leave_packet(old_p, user_id);
			}
			else {
				g_clients[old_p]->m_cl.unlock();
			}
		}
	}
}

void CViewProcessing::check_near_view_pn(int npc_id)
{
	g_clients[npc_id]->m_cl.lock();
	unordered_set <unsigned int> old_vl = g_clients[npc_id]->m_viewlist.viewlist;
	unordered_set <unsigned int> new_vl = g_clients[npc_id]->m_nearlist.nearlist;
	g_clients[npc_id]->m_cl.unlock();

	for (auto np : new_vl)
	{
		g_clients[npc_id]->m_cl.lock();
		if (0 != g_clients[npc_id]->m_viewlist.viewlist.count(np)) {
			g_clients[npc_id]->m_cl.unlock();
			CPacketHandler::GetInst()->send_move_packet(np, npc_id);
		}
		else {
			g_clients[npc_id]->m_cl.unlock();
			CPacketHandler::GetInst()->send_enter_packet(np, npc_id);
		}
	}

	for (auto old_p : old_vl)
	{
		g_clients[npc_id]->m_cl.lock();
		if (0 != g_clients[npc_id]->m_viewlist.viewlist.count(old_p)) {
			g_clients[npc_id]->m_cl.unlock();
			CPacketHandler::GetInst()->send_leave_packet(old_p, npc_id);
		}
		else {
			g_clients[npc_id]->m_cl.unlock();
		}
	}
}

void CViewProcessing::change_sector(int user_id, int x, int y, int col, int row) // 지역변수 사용하게 되면 파라미터 달라야 함.
{
	if (x / COL_GAP != col || y / ROW_GAP != row)
	{
		int iPrevCol = col; int iPrevRow = row;

		int iCol = x / COL_GAP; int iRow = y / ROW_GAP;

		if (iCol > MAX_COL - 1)	iCol = iPrevCol;
		if (iRow > MAX_ROW - 1)	iRow = iPrevRow;

		g_sectors[iPrevRow][iPrevCol].sector_lock.lock();
		g_sectors[iPrevRow][iPrevCol].m_setPlayerList.erase(user_id);
		g_sectors[iPrevRow][iPrevCol].sector_lock.unlock();

		g_sectors[iRow][iCol].sector_lock.lock();
		g_sectors[iRow][iCol].m_setPlayerList.emplace(user_id);
		g_clients[user_id]->row = iRow;	g_clients[user_id]->col = iCol;
		g_sectors[iRow][iCol].sector_lock.unlock();
	}
}

bool CViewProcessing::is_near_check_pp(int id1, int id2)
{
	if (VIEW_RADIUS / 2 < abs(g_clients[id1]->x - g_clients[id2]->x))
		return false;
	if (VIEW_RADIUS / 2 < abs(g_clients[id1]->y - g_clients[id2]->y))
		return false;

	// 타겟에서 벗어나면 -1로 세팅
	char type = g_clients[id1]->m_otype;
	if (type == O_BLUES || (type == O_REDS && g_clients[id1]->is_move)) {

		if (is_under_ten(id1, id2))
		{
			g_clients[id1]->target_id = id2;
			return true;
		}
		else
			g_clients[id1]->target_id = -1;
	}
	else if (type == O_BOSSS)
	{
		g_clients[id1]->target_id = id2;
	}

	return true;
}

bool CViewProcessing::is_neartile_check_pt(int id1, int id2)
{
	if (VIEW_RADIUS / 2 < abs(g_clients[id1]->x - g_tile[id2 - (TILE_START_IDX)]->x))
		return false;
	if (VIEW_RADIUS / 2 < abs(g_clients[id1]->y - g_tile[id2 - (TILE_START_IDX)]->y))
		return false;

	return true;
}


bool CViewProcessing::is_obstacle_check(int id1, int id2)
{
	if (!CPacketHandler::GetInst()->is_tile(id2))
	{
		if (g_clients[id2]->is_dead)
			return false;

		if (5 < abs(g_clients[id1]->x - g_clients[id2]->x))
			return false;
		if (5 < abs(g_clients[id1]->y - g_clients[id2]->y))
			return false;

		return true;
	}
	else  if (CPacketHandler::GetInst()->is_tile(id2))
	{
		if (5 < abs(g_clients[id1]->x - g_tile[id2 - (TILE_START_IDX)]->x))
			return false;
		if (5 < abs(g_clients[id1]->y - g_tile[id2 - (TILE_START_IDX)]->y))
			return false;

		return true;
	}
}


bool CViewProcessing::is_neartile_check_nt(int id1, int id2)
{
	if (5 < abs(g_clients[id1]->x - g_tile[id2 - (TILE_START_IDX)]->x))
		return false;
	if (5 < abs(g_clients[id1]->y - g_tile[id2 - (TILE_START_IDX)]->y))
		return false;

	return true;
}

bool CViewProcessing::is_under_ten(int id1, int id2)
{
	if (abs(g_clients[id1]->x - g_clients[id2]->x) >= 5)
		return false;

	if (abs(g_clients[id1]->y - g_clients[id2]->y) >= 5)
		return false;

	return true;
}

bool CViewProcessing::is_attack_range_pn(int user_id, int mon_id)
{
	if (1 < abs(g_clients[user_id]->x - g_clients[mon_id]->x) + abs(g_clients[user_id]->y - g_clients[mon_id]->y) && !g_clients[mon_id]->is_dead)
		return false;

	return true;
}

//CLIENT& cl = g_clients[playerlist];
//ZeroMemory(&cl.m_recv_over.over, sizeof(cl.m_recv_over.op));
//cl.m_recv_over.op = OP_PLAYER_MOVE;
//cl.m_recv_over.player_id = playerlist;
//PostQueuedCompletionStatus(g_iocp, 1, cl.m_recv_over.player_id, &cl.m_recv_over.over);
