#include "AIHandler.h"
#include "ViewProcessing.h"
#include "PacketHandler.h"

CAIHandler::CAIHandler()
{
}

CAIHandler::~CAIHandler()
{
}

void CAIHandler::astar_finding(int npc_id, int target_id, ENUMOP event_type) // if(플레이어가 사망하면 종료) 추가하는거 고려
{
	reset_astar(npc_id);

	if (g_clients[npc_id]->m_status != ST_ACTIVE)
		return;

	if (target_id == -1)
	{
		addtimer(npc_id, target_id, OP_RANDOM_MOVE, 1000); // 추가
		return;
	}

	if (true == g_clients[npc_id]->is_dead)
	{
		return;
	}

	init_node(npc_id, target_id);
	find_path(npc_id, target_id);



	if (g_clients[npc_id]->m_listPath.size() > 10 || g_clients[npc_id]->m_listPath.size() == 0)
	{
		reset_astar(npc_id);  // g_clients[npc_id]->m_listPath.clear();
		addtimer(npc_id, -1, OP_RANDOM_MOVE, 1000);
	}
	else if (g_clients[npc_id]->m_listPath.size() > 0 && g_clients[npc_id]->m_listPath.size() < 10)
	{
		astar_move(npc_id, g_clients[npc_id]->target_id);
	}
	//}
		//addtimer(npc_id, target_id, OP_ASTAR_MOVE, 500);
}

void CAIHandler::init_node(int npc_id, int target_id)
{
	g_clients[npc_id]->start_x = g_clients[npc_id]->x;
	g_clients[npc_id]->start_y = g_clients[npc_id]->y;
	g_clients[npc_id]->end_x = g_clients[target_id]->x;
	g_clients[npc_id]->end_y = g_clients[target_id]->y;

	int iStartX = g_clients[npc_id]->start_x;
	int iStartY = g_clients[npc_id]->start_y;
	int iEndX = g_clients[npc_id]->end_x;
	int iEndY = g_clients[npc_id]->end_y;

	for (int i = 0; i < NPC_RADIUS + 1; ++i)
	{
		for (int j = 0; j < NPC_RADIUS + 1; ++j)
		{
			if (g_clients[npc_id]->start_x - NPC_RADIUS * 0.5 + j < 0)
				g_clients[npc_id]->m_arrNode[i][j].iCurIdxX = 0;
			else if (g_clients[npc_id]->start_x - NPC_RADIUS * 0.5 + j > WORLD_WIDTH)
				g_clients[npc_id]->m_arrNode[i][j].iCurIdxX = WORLD_WIDTH - 1;
			else
				g_clients[npc_id]->m_arrNode[i][j].iCurIdxX = g_clients[npc_id]->start_x - 5 + j;

			if (g_clients[npc_id]->start_y - NPC_RADIUS * 0.5 + i < 0)
				g_clients[npc_id]->m_arrNode[i][j].iCurIdxY = 0;
			else if (g_clients[npc_id]->start_y - NPC_RADIUS * 0.5 + j > WORLD_HEIGHT)
				g_clients[npc_id]->m_arrNode[i][j].iCurIdxY = WORLD_HEIGHT - 1;
			else
				g_clients[npc_id]->m_arrNode[i][j].iCurIdxY = g_clients[npc_id]->start_y - 5 + i;
		}
	}

	CViewProcessing::GetInst()->create_obstacle_nt(npc_id, ((int)(iStartY - NPC_RADIUS * 0.5) / ROW_GAP),
		(int)((iStartX - NPC_RADIUS * 0.5) / COL_GAP), (int)((iStartY + NPC_RADIUS * 0.5) / ROW_GAP),
		(int)((iStartX + NPC_RADIUS * 0.5) / COL_GAP));

	g_clients[npc_id]->m_cl.lock();
	unordered_set <unsigned int> near_vl_nt = g_clients[npc_id]->m_nearlist.nearlist;
	g_clients[npc_id]->m_cl.unlock();

	if (tile_alloc[iStartY - 1][iStartX].m_alloc != T_ALLOC::TA_FREE) 	g_clients[npc_id]->m_arrNode[4][5].bObstacle = true;
	if (tile_alloc[iStartY][iStartX-1].m_alloc != T_ALLOC::TA_FREE) 	g_clients[npc_id]->m_arrNode[5][4].bObstacle = true;
	if (tile_alloc[iStartY + 1][iStartX].m_alloc != T_ALLOC::TA_FREE) 	g_clients[npc_id]->m_arrNode[6][5].bObstacle = true;
	if (tile_alloc[iStartY][iStartX+1].m_alloc != T_ALLOC::TA_FREE) 	g_clients[npc_id]->m_arrNode[5][6].bObstacle = true;
		
	tile_alloc[iStartY][iStartX].m_alloc = T_ALLOC::TA_ALLOC;

	for (auto& n_vl : near_vl_nt)
	{
		if (n_vl == npc_id)
			continue;

		short iIdxY = -1;	short iIdxX = -1;

		if (CPacketHandler::GetInst()->is_tile(n_vl))
		{
			iIdxY = (short)(NPC_RADIUS * 0.5 - iStartY + g_tile[n_vl - (TILE_START_IDX)]->y);
			iIdxX = (short)(NPC_RADIUS * 0.5 - iStartX + g_tile[n_vl - (TILE_START_IDX)]->x);
		}
		else
		{
			int o_x = g_clients[n_vl]->x;
			int o_y = g_clients[n_vl]->y;

			if (iEndY != o_y || iEndX != o_x)
			{
				iIdxY = (short)(NPC_RADIUS * 0.5 - iStartY + o_y);
				iIdxX = (short)(NPC_RADIUS * 0.5 - iStartX + o_x);
			}
		}

		if (iIdxY < 0 || iIdxX < 0 || iIdxY > NPC_RADIUS || iIdxX > NPC_RADIUS)
		{
			continue;
		}
		g_clients[npc_id]->m_cl.lock();
		g_clients[npc_id]->m_arrNode[iIdxY][iIdxX].bObstacle = true;
		g_clients[npc_id]->m_cl.unlock();
	}
}

void CAIHandler::find_path(int npc_id, int target_id)
{
	size_t openlist_size = g_clients[npc_id]->m_openlist.size();

	int iStartX = g_clients[npc_id]->start_x;
	int iStartY = g_clients[npc_id]->start_y;
	int iEndX = g_clients[npc_id]->end_x;
	int iEndY = g_clients[npc_id]->end_y;

	g_clients[npc_id]->m_cl.lock();
	while (!g_clients[npc_id]->m_openlist.empty()) {
		g_clients[npc_id]->m_openlist.pop();
	}
	g_clients[npc_id]->m_cl.unlock();

	g_clients[npc_id]->m_cl.lock();
	g_clients[npc_id]->m_openlist.push(&g_clients[npc_id]->m_arrNode[NPC_RADIUS / 2][NPC_RADIUS / 2]); // 인덱스로 들어가는거 생각해봐야함.
	g_clients[npc_id]->m_arrNode[NPC_RADIUS / 2][NPC_RADIUS / 2].bOpened = false;
	g_clients[npc_id]->m_cl.unlock();

	tNode* node = {};
	int iY = -10; int iX = -10;
	while (1)
	{
		if (0 == g_clients[npc_id]->m_openlist.size())
			return;

		g_clients[npc_id]->m_cl.lock();
		node = g_clients[npc_id]->m_openlist.top();
		g_clients[npc_id]->m_cl.unlock();

		iY = (int)(NPC_RADIUS * 0.5 - iStartY + node->iCurIdxY);
		iX = (int)(NPC_RADIUS * 0.5 - iStartX + node->iCurIdxX);

		g_clients[npc_id]->m_cl.lock();
		g_clients[npc_id]->m_openlist.pop();
		g_clients[npc_id]->m_cl.unlock();

		if (iY < 0 || iX < 0 || iY > NPC_RADIUS || iX> NPC_RADIUS)
		{
			continue;
		}
		g_clients[npc_id]->m_arrNode[iY][iX].bClosed = true;

		g_clients[npc_id]->m_cl.lock();
		if (g_clients[npc_id]->m_arrNode[iY][iX].iCurIdxX == iEndX && g_clients[npc_id]->m_arrNode[iY][iX].iCurIdxY == iEndY)
		{
			g_clients[npc_id]->m_cl.unlock();
			break;
		}
		g_clients[npc_id]->m_cl.unlock();

		add_openlist(npc_id, iX, iY + 1, iX, iY);//add_openlist(npc_id, iX + 1,iY + 1, iX, iY);
		add_openlist(npc_id, iX + 1, iY, iX, iY);//add_openlist(npc_id, iX + 1,iY - 1, iX, iY);
		add_openlist(npc_id, iX, iY - 1, iX, iY);//add_openlist(npc_id, iX - 1,iY - 1, iX, iY);
		add_openlist(npc_id, iX - 1, iY, iX, iY);//add_openlist(npc_id, iX - 1,iY + 1, iX, iY);
	}

	g_clients[npc_id]->m_cl.lock();
	g_clients[npc_id]->m_listPath.push_front(&g_clients[npc_id]->m_arrNode[iY][iX]);
	g_clients[npc_id]->reserved_x = -1;
	g_clients[npc_id]->reserved_y = -1;
	g_clients[npc_id]->m_cl.unlock();

	// 여기서 메모리 늘어남, 같은놈들이 들어있어서 그럴 수 있음

	tNode* pNode = nullptr;
	while (true)
	{
		if (g_clients[npc_id]->m_listPath.size() > 10 || g_clients[npc_id]->m_listPath.size() == 0)
			break;

		pNode = g_clients[npc_id]->m_listPath.front();
		if (pNode->iCurIdxX == iStartX && pNode->iCurIdxY == iStartY)
		{
			g_clients[npc_id]->m_listPath.pop_front();
			if (g_clients[npc_id]->m_listPath.size() > 0)
			{
				
				tile_alloc[g_clients[npc_id]->m_listPath.front()->iCurIdxY][g_clients[npc_id]->m_listPath.front()->iCurIdxX].m_alloc = T_ALLOC::TA_RESERVE; // 제자리에서 한칸
				g_clients[npc_id]->reserved_x = g_clients[npc_id]->m_listPath.front()->iCurIdxX;
				g_clients[npc_id]->reserved_y = g_clients[npc_id]->m_listPath.front()->iCurIdxY;
			}

			break;
		}

		g_clients[npc_id]->m_listPath.push_front(
			&g_clients[npc_id]->m_arrNode[g_clients[npc_id]->m_listPath.front()->iParentIdxY][g_clients[npc_id]->m_listPath.front()->iParentIdxX]);
	}

}

void CAIHandler::add_openlist(int npc_id, int _iX, int _iY, int _iOrgX, int _iOrgY)
{
	// 여기서의 조건이 필요함
	g_clients[npc_id]->m_cl.lock();
	if (_iX < 0 || NPC_RADIUS < _iX || _iY < 0 || NPC_RADIUS < _iY
		|| g_clients[npc_id]->m_arrNode[_iY][_iX].bClosed || g_clients[npc_id]->m_arrNode[_iY][_iX].bObstacle)
	{
		g_clients[npc_id]->m_cl.unlock();
		return;
	}
	g_clients[npc_id]->m_cl.unlock();

	cal_cost(npc_id, _iX, _iY, _iOrgX, _iOrgY);

	g_clients[npc_id]->m_cl.lock();
	if (false == g_clients[npc_id]->m_arrNode[_iY][_iX].bOpened)
	{
		g_clients[npc_id]->m_cl.unlock();
		g_clients[npc_id]->m_openlist.push(&g_clients[npc_id]->m_arrNode[_iY][_iX]);
		g_clients[npc_id]->m_arrNode[_iY][_iX].bOpened = true;
	}
	else
		g_clients[npc_id]->m_cl.unlock();
}

void CAIHandler::cal_cost(int npc_id, int _iX, int _iY, int _iOrgX, int _iOrgY)
{
	// 이전 노드까지의 비용, + 1칸 거리 == 현재 노드까지 오는 비용
	g_clients[npc_id]->m_cl.lock();
	if (g_clients[npc_id]->m_arrNode[_iY][_iX].bOpened)
	{
		int iFromDist = (int)(10 * (sqrt((_iOrgY - _iY) * (_iOrgY - _iY) + (_iOrgX - _iX) * (_iOrgX - _iX))));
		// 이번에 추가되는 루트가 더 좋은 루트였다.
		if (g_clients[npc_id]->m_arrNode[_iY][_iX].fFrom > g_clients[npc_id]->m_arrNode[_iOrgY][_iOrgX].fFrom + iFromDist)
		{
			g_clients[npc_id]->m_cl.unlock();
			g_clients[npc_id]->m_arrNode[_iY][_iX].fFrom = g_clients[npc_id]->m_arrNode[_iOrgY][_iOrgX].fFrom + iFromDist;
			g_clients[npc_id]->m_arrNode[_iY][_iX].iParentIdxX = _iOrgX;
			g_clients[npc_id]->m_arrNode[_iY][_iX].iParentIdxY = _iOrgY;
		}
		else
			g_clients[npc_id]->m_cl.unlock();
	}
	else // openlist 에 처음 추가되는 상황
	{
		g_clients[npc_id]->m_cl.unlock();
		int iFromDist = (int)(10 * (sqrt((_iOrgY - _iY) * (_iOrgY - _iY) + (_iOrgX - _iX) * (_iOrgX - _iX))));

		g_clients[npc_id]->m_arrNode[_iY][_iX].fFrom = g_clients[npc_id]->m_arrNode[_iOrgY][_iOrgX].fFrom + iFromDist;
		g_clients[npc_id]->m_arrNode[_iY][_iX].fDest = abs(g_clients[npc_id]->end_x - g_clients[npc_id]->m_arrNode[_iY][_iX].iCurIdxX)
			+ abs(g_clients[npc_id]->end_y - g_clients[npc_id]->m_arrNode[_iY][_iX].iCurIdxY);
		g_clients[npc_id]->m_arrNode[_iY][_iX].fFinal = g_clients[npc_id]->m_arrNode[_iY][_iX].fFrom + g_clients[npc_id]->m_arrNode[_iY][_iX].fDest;

		g_clients[npc_id]->m_arrNode[_iY][_iX].iParentIdxX = _iOrgX;
		g_clients[npc_id]->m_arrNode[_iY][_iX].iParentIdxY = _iOrgY;
	}
}

void CAIHandler::astar_move(int npc_id, int target_id) 
{
	if (g_clients[npc_id]->m_status != ST_ACTIVE)
		return;

	g_clients[npc_id]->m_cl.lock();
	if (g_clients[npc_id]->m_listPath.size() == 0)
	{
		addtimer(npc_id, target_id, OP_RANDOM_MOVE, 1000); // 추가
		return;
	}
	tNode* pnode = g_clients[npc_id]->m_listPath.front();
	g_clients[npc_id]->m_cl.unlock();

	g_clients[npc_id]->m_cl.lock();
	tile_alloc[pnode->iCurIdxY][pnode->iCurIdxX].m_alloc = TA_ALLOC;
	tile_alloc[g_clients[npc_id]->y][g_clients[npc_id]->x].m_alloc = TA_FREE;
	g_clients[npc_id]->m_cl.unlock();

	g_clients[npc_id]->x = pnode->iCurIdxX;
	g_clients[npc_id]->y = pnode->iCurIdxY;


	g_clients[npc_id]->m_listPath.pop_front();

	reset_astar(npc_id);

	if (g_clients[npc_id]->x / COL_GAP != g_clients[npc_id]->col || g_clients[npc_id]->y / ROW_GAP != g_clients[npc_id]->row)
	{
		int iPrevCol = g_clients[npc_id]->col;
		int iPrevRow = g_clients[npc_id]->row;

		int iCol = g_clients[npc_id]->x / COL_GAP;
		int iRow = g_clients[npc_id]->y / ROW_GAP;

		if (iCol > MAX_COL - 1)
			iCol = iPrevCol;
		if (iRow > MAX_ROW - 1)
			iRow = iPrevRow;

		g_sectors[iPrevRow][iPrevCol].sector_lock.lock();
		g_sectors[iPrevRow][iPrevCol].m_setPlayerList.erase(g_clients[npc_id]->m_id);
		g_sectors[iPrevRow][iPrevCol].sector_lock.unlock();

		g_sectors[iRow][iCol].sector_lock.lock();
		g_sectors[iRow][iCol].m_setPlayerList.emplace(g_clients[npc_id]->m_id);
		g_clients[npc_id]->row = iRow;
		g_clients[npc_id]->col = iCol;
		g_sectors[iRow][iCol].sector_lock.unlock();
	}

	CViewProcessing::GetInst()->create_nearlist_np(npc_id, (g_clients[npc_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
		(g_clients[npc_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[npc_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[npc_id]->x + VIEW_RADIUS / 2) / COL_GAP);

	unordered_set<unsigned int> near_vl = g_clients[npc_id]->m_nearlist.nearlist;
	unordered_set<unsigned int> view_vl = g_clients[npc_id]->m_viewlist.viewlist;

	// 시작하자마자 밖으로 나가면 viewlist nearlist 둘다 없어서 지우지를 못함.
	bool keep_alive = false;

	for (auto& n_p : near_vl)
	{
		if (ST_ACTIVE != g_clients[n_p]->m_status) continue;
		if (true == CViewProcessing::GetInst()->is_near_check_pp(n_p, npc_id))
		{
			keep_alive = true;
			g_clients[n_p]->m_cl.lock();
			if (0 != g_clients[n_p]->m_viewlist.viewlist.count(npc_id)) {
				g_clients[n_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_move_packet(n_p, npc_id);
			}
			else {
				g_clients[n_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_enter_packet(n_p, npc_id);
			}
		}
	}

	for (auto& v_p : view_vl)
	{
		if (true != CViewProcessing::GetInst()->is_near_check_pp(v_p, npc_id)) // 근처에 없다.
		{
			g_clients[v_p]->m_cl.lock();
			if (0 != g_clients[v_p]->m_viewlist.viewlist.count(npc_id)) { // 플레이어 viewlist에 있다
				g_clients[v_p]->m_cl.unlock();
				CPacketHandler::GetInst()->send_leave_packet(v_p, npc_id);
			}
			else {
				g_clients[v_p]->m_cl.unlock();
			}
		}
	}

	if (true == keep_alive)
	{
		if (true == g_clients[npc_id]->is_dead)
		{
			return;
		}

		if (abs(g_clients[npc_id]->x - g_clients[target_id]->x) + abs(g_clients[npc_id]->y - g_clients[target_id]->y) <= 1)
			addtimer(npc_id, target_id, OP_ENEMY_ATTACK, 1000);
		else if (abs(g_clients[npc_id]->x - g_clients[target_id]->x) <= NPC_RADIUS / 2 && abs(g_clients[npc_id]->y - g_clients[target_id]->y <= NPC_RADIUS / 2))
			addtimer(npc_id, g_clients[npc_id]->target_id, OP_ASTAR_MOVE, 1000);
		else
		{
			if (g_clients[npc_id]->m_otype == O_REDS)
				g_clients[npc_id]->is_move = false;
			else
				addtimer(npc_id, -1, OP_RANDOM_MOVE, 1000);

			tile_alloc[g_clients[npc_id]->y][g_clients[npc_id]->x].m_alloc = TA_FREE;
			if (g_clients[npc_id]->reserved_x != -1 && g_clients[npc_id]->reserved_y != -1)
			{
				tile_alloc[g_clients[npc_id]->reserved_y][g_clients[npc_id]->reserved_x].m_alloc = TA_FREE;
				g_clients[npc_id]->reserved_x = -1;
				g_clients[npc_id]->reserved_y = -1;
			}
		}
	}
	else g_clients[npc_id]->m_status = ST_SLEEP;

	//if (g_clients[npc_id]->m_listPath.empty())
	//{
	//	reset_astar(npc_id);

	//	if (true == g_clients[npc_id]->is_dead)
	//	{
	//		return;
	//	}

	//	if (abs(g_clients[npc_id]->x - g_clients[target_id]->x) + abs(g_clients[npc_id]->y - g_clients[target_id]->y) <= 1)
	//		addtimer(npc_id, target_id, OP_ENEMY_ATTACK, 1000);
	//	else if(abs(g_clients[npc_id]->x - g_clients[target_id]->x) <= NPC_RADIUS / 2 && abs(g_clients[npc_id]->y - g_clients[target_id]->y <= NPC_RADIUS / 2))
	//		addtimer(npc_id, g_clients[npc_id]->target_id, OP_ASTAR_MOVE, 1000);
	//	else 
	//		addtimer(npc_id, -1, OP_RANDOM_MOVE, 1000);
	//	//g_clients[npc_id]->target_id = -1;
	//}
	//else // 플레이어 좌표까지 오게 하려면 위에 하면 됨 아래 else문
	//{
	//	reset_astar(npc_id);

	//	if (g_clients[npc_id]->x / COL_GAP != g_clients[npc_id]->col || g_clients[npc_id]->y / ROW_GAP != g_clients[npc_id]->row)
	//	{
	//		int iPrevCol = g_clients[npc_id]->col;
	//		int iPrevRow = g_clients[npc_id]->row;

	//		int iCol = g_clients[npc_id]->x / COL_GAP;
	//		int iRow = g_clients[npc_id]->y / ROW_GAP;

	//		if (iCol > MAX_COL - 1)
	//			iCol = iPrevCol;
	//		if (iRow > MAX_ROW - 1)
	//			iRow = iPrevRow;

	//		g_sectors[iPrevRow][iPrevCol].sector_lock.lock();
	//		g_sectors[iPrevRow][iPrevCol].m_setPlayerList.erase(g_clients[npc_id]->m_id);
	//		g_sectors[iPrevRow][iPrevCol].sector_lock.unlock();

	//		g_sectors[iRow][iCol].sector_lock.lock();
	//		g_sectors[iRow][iCol].m_setPlayerList.emplace(g_clients[npc_id]->m_id);
	//		g_clients[npc_id]->row = iRow;
	//		g_clients[npc_id]->col = iCol;
	//		g_sectors[iRow][iCol].sector_lock.unlock();
	//	}

	//	CViewProcessing::GetInst()->create_nearlist_np(npc_id, (g_clients[npc_id]->y - VIEW_RADIUS / 2) / ROW_GAP,
	//		(g_clients[npc_id]->x - VIEW_RADIUS / 2) / COL_GAP, (g_clients[npc_id]->y + VIEW_RADIUS / 2) / ROW_GAP, (g_clients[npc_id]->x + VIEW_RADIUS / 2) / COL_GAP);

	//	unordered_set<unsigned int> near_vl = g_clients[npc_id]->m_nearlist.nearlist;
	//	unordered_set<unsigned int> view_vl = g_clients[npc_id]->m_viewlist.viewlist;

	//	// 시작하자마자 밖으로 나가면 viewlist nearlist 둘다 없어서 지우지를 못함.
	//	bool keep_alive = false;

	//	for (auto& n_p : near_vl)
	//	{
	//		if (ST_ACTIVE != g_clients[n_p]->m_status) continue;
	//		if (true == CViewProcessing::GetInst()->is_near_check_pp(n_p, npc_id))
	//		{
	//			keep_alive = true;
	//			g_clients[n_p]->m_cl.lock();
	//			if (0 != g_clients[n_p]->m_viewlist.viewlist.count(npc_id)) {
	//				g_clients[n_p]->m_cl.unlock();
	//				CPacketHandler::GetInst()->send_move_packet(n_p, npc_id);
	//			}
	//			else {
	//				g_clients[n_p]->m_cl.unlock();
	//				CPacketHandler::GetInst()->send_enter_packet(n_p, npc_id);
	//			}
	//		}
	//	}

	//	for (auto& v_p : view_vl)
	//	{
	//		if (true != CViewProcessing::GetInst()->is_near_check_pp(v_p, npc_id)) // 근처에 없다.
	//		{
	//			g_clients[v_p]->m_cl.lock();
	//			if (0 != g_clients[v_p]->m_viewlist.viewlist.count(npc_id)) { // 플레이어 viewlist에 있다
	//				g_clients[v_p]->m_cl.unlock();
	//				CPacketHandler::GetInst()->send_leave_packet(v_p, npc_id);
	//			}
	//			else {
	//				g_clients[v_p]->m_cl.unlock();
	//			}
	//		}
	//	}

	//	if (true == keep_alive)
	//	{
	//		if (true == g_clients[npc_id]->is_dead)
	//		{
	//			//reset_astar(npc_id); //  g_clients[npc_id]->m_listPath.clear();
	//			//addtimer(npc_id, g_clients[npc_id]->target_id, OP_RESURRECT, 10000);
	//			return;
	//		}

	//		if (abs(g_clients[npc_id]->x - g_clients[target_id]->x) + abs(g_clients[npc_id]->y - g_clients[target_id]->y) <= 1)
	//			addtimer(npc_id, target_id, OP_ENEMY_ATTACK, 1000);
	//		else if (abs(g_clients[npc_id]->x - g_clients[target_id]->x) <= NPC_RADIUS / 2 && abs(g_clients[npc_id]->y - g_clients[target_id]->y <= NPC_RADIUS / 2))
	//			addtimer(npc_id, g_clients[npc_id]->target_id, OP_ASTAR_MOVE, 1000);
	//		else
	//			addtimer(npc_id, -1, OP_RANDOM_MOVE, 1000);
	//	}
	//	else g_clients[npc_id]->m_status = ST_SLEEP;
	//}
}

bool CAIHandler::is_conflict(int id1, int id2)
{
	if (abs(g_clients[id1]->x - g_clients[id2]->x) + abs(g_clients[id1]->y - g_clients[id2]->y) != 0)
		return false;

	return true;
}

int CAIHandler::get_dist(int id1, int id2)
{
	return abs(g_clients[id1]->x - g_clients[id2]->x) + abs(g_clients[id1]->y - g_clients[id2]->y);
}

bool CAIHandler::is_range(int id1, int id2)
{
	if (abs(g_clients[id1]->x - g_clients[id2]->x) > 5)
		return false;
	if (abs(g_clients[id1]->y - g_clients[id2]->y) > 5)
		return false;
	return true;
}

void CAIHandler::reset_astar(int npc_id)
{
	// 초기화 진행
	while (1)
	{
		g_clients[npc_id]->m_cl.lock();
		if (g_clients[npc_id]->m_openlist.size() == 0) {
			g_clients[npc_id]->m_cl.unlock();
			break;
		}
		g_clients[npc_id]->m_openlist.pop();
		g_clients[npc_id]->m_cl.unlock();
	}

	while (1)
	{
		if (g_clients[npc_id]->m_listPath.size() == 0) { break; }
		g_clients[npc_id]->m_listPath.pop_back();
	}

	for (int i = 0; i <= 10; ++i)
	{
		for (int j = 0; j <= 10; ++j)
		{
			g_clients[npc_id]->m_arrNode[j][i].bClosed = false;
			g_clients[npc_id]->m_arrNode[j][i].bOpened = false;
			g_clients[npc_id]->m_arrNode[j][i].bObstacle = false;

			g_clients[npc_id]->m_arrNode[j][i].iCurIdxX = 0;
			g_clients[npc_id]->m_arrNode[j][i].iCurIdxY = 0;

			g_clients[npc_id]->m_arrNode[j][i].iParentIdxX = 0;
			g_clients[npc_id]->m_arrNode[j][i].iParentIdxY = 0;

			g_clients[npc_id]->m_arrNode[j][i].fFrom = 0;
			g_clients[npc_id]->m_arrNode[j][i].fDest = 0;
			g_clients[npc_id]->m_arrNode[j][i].fFinal = 0;
		}
	}

	g_clients[npc_id]->start_x = 0;
	g_clients[npc_id]->start_y = 0;

	g_clients[npc_id]->end_x = 0;
	g_clients[npc_id]->end_y = 0;
}

void CAIHandler::do_attack(int user_id) // 마지막으로 이동한 방향으로 일반 공격
{
	CLIENT& user = *g_clients[user_id];
	int x = user.x;	int y = user.y;

	CViewProcessing::GetInst()->create_nearlist_pn(user_id, user.row - 1, user.col - 1, user.row + 1, user.col + 1);
	// 현재 위치에서 상하좌우로

	unordered_set <unsigned int> near_vl_pn = user.m_nearlist.nearlist;

	for (auto& near_vl : near_vl_pn)
	{
		if (!g_clients[near_vl]->is_dead && g_clients[near_vl]->m_otype != O_NPC)
		{
			WCHAR str[MAX_STR_LEN]{ L"" };
			wcscat_s(str, L"User : ");	wcscat_s(str, g_clients[user_id]->name); wcscat_s(str, L" attacked ");	wcscat_s(str, g_clients[near_vl]->name); wcscat_s(str, L"! ");
			wcscat_s(str, L"dealing ");	WCHAR num[5]{ L"" }; _itow_s(user.level * 10, num, 10);	wcscat_s(str, num);	wcscat_s(str, L" damages !");
			// wcscat_s(npcname, g_clients[g_clients[user_id]->m_event.target_id]->name);

			if (!g_clients[near_vl]->is_move && g_clients[near_vl]->m_otype == O_REDS)
			{
				g_clients[near_vl]->is_move = true; // // 이런식으로 짜면 안됨 구조화좀 해야됨.
				g_clients[near_vl]->target_id = user_id;
				addtimer(near_vl, user_id, OP_ASTAR_MOVE, 500);
			}

			CPacketHandler::GetInst()->send_chat_packet(user_id, SERVER_CHATTER, lstrlen(str), str);

			g_clients[near_vl]->hp -= user.level * 10;
		}

		if (g_clients[near_vl]->hp <= 0 && !g_clients[near_vl]->is_dead)
		{
			tile_alloc[g_clients[near_vl]->y][g_clients[near_vl]->x].m_alloc = TA_FREE;
			if (g_clients[near_vl]->reserved_y != -1 && g_clients[near_vl]->reserved_x != -1)
			{
				tile_alloc[g_clients[near_vl]->reserved_y][g_clients[near_vl]->reserved_x].m_alloc = TA_FREE;
				g_clients[near_vl]->reserved_y = -1;
				g_clients[near_vl]->reserved_x = -1;
			}
			g_clients[near_vl]->is_dead = true;
			if (g_clients[near_vl]->m_otype == O_BLUES)
			{
				if (g_clients[user_id]->have_mon_quest && !g_clients[user_id]->clear_mon_quest)
					++g_clients[user_id]->killscore_blueslime;
				reset_astar(near_vl);
			}
			else if (g_clients[near_vl]->m_otype == O_BOSSS)
			{
				if (g_clients[user_id]->have_boss_quest && !g_clients[user_id]->clear_boss_quest)
					++g_clients[user_id]->killscore_boss;
			}
			else if (g_clients[near_vl]->m_otype == O_REDS)
			{
				reset_astar(near_vl);
			}
			manage_exp(user_id, g_clients[near_vl]->level * 15); //g_clients[user_id]->exp += 
			CPacketHandler::GetInst()->send_stat_change(user_id); // exp , level에서 stat change
			addtimer(near_vl, -1, OP_RESURRECT, 30000);
		}
	}
}

void CAIHandler::do_resurrect(int user_id)
{
}

void CAIHandler::manage_exp(int user_id, int amount)
{
	int iExpToLevelUp = (int)pow(2, g_clients[user_id]->level - 1) * LEVEL_UP_EXP;
	int iCurExp = g_clients[user_id]->exp;
	int iAfterExp = g_clients[user_id]->exp + amount;

	if (iAfterExp < iExpToLevelUp)
	{
		g_clients[user_id]->exp += amount;
	}
	else if (iAfterExp >= iExpToLevelUp)
	{
		int iExpGap = iAfterExp - iExpToLevelUp;

		g_clients[user_id]->level += 1;
		g_clients[user_id]->exp = iExpGap;
		g_clients[user_id]->maxexp = (int)pow(2, g_clients[user_id]->level - 1) * LEVEL_UP_EXP;
		g_clients[user_id]->maxhp += 50;
		g_clients[user_id]->hp = g_clients[user_id]->maxhp;
		g_clients[user_id]->damage += 10;
	}

	WCHAR str[MAX_STR_LEN]{ L"" };
	wcscat_s(str, L"Killed a "); wcscat_s(str, g_clients[user_id]->name); wcscat_s(str, L", and gained  ");
	WCHAR num[10]{ L"" }; _itow_s(amount, num, 10);	wcscat_s(str, num);	wcscat_s(str, L" exp !");
	CPacketHandler::GetInst()->send_chat_packet(user_id, SERVER_CHATTER, lstrlen(str), str);
}

void CAIHandler::manage_hp(int user_id, int damage)
{
	int iCurUserHp = g_clients[user_id]->hp;
	int iHpGap = iCurUserHp - damage;
	g_clients[user_id]->m_cl.lock();
	if (iHpGap > 0)
	{
		g_clients[user_id]->m_cl.unlock();
		g_clients[user_id]->hp -= damage;
	}
	else
	{
		g_clients[user_id]->m_cl.unlock();
		g_clients[user_id]->is_dead = true;
		//g_clients[user_id]->hp -= damage;
		WCHAR str[20] = { L" player die" };
		WCHAR num[25]{ L"" };
		_itow_s(user_id, num, 10);
		wcscat_s(num, str);

		CPacketHandler::GetInst()->send_chat_packet(user_id, SERVER_CHATTER, lstrlen(num), num);
		addtimer(user_id, -1, OP_RESURRECT, 2000);
	}

}

void CAIHandler::heal(int user_id)
{
	int iCurUserHp = g_clients[user_id]->hp;
	int iCurUserMaxHp = g_clients[user_id]->maxhp;

	if (iCurUserHp == iCurUserMaxHp)
		return;

	if (iCurUserHp < iCurUserMaxHp) {
		iCurUserHp += (int)(0.1f * iCurUserMaxHp);

		if (iCurUserHp > iCurUserMaxHp)
			iCurUserHp -= (iCurUserHp - iCurUserMaxHp);
	}
	g_clients[user_id]->hp = iCurUserHp;
	CPacketHandler::GetInst()->send_stat_change(user_id);
}



//int iFirstY = 0, iSecondY = 0, iFirstX = 0, iSecondX = 0;
//
//if (g_clients[npc_id]->start_y > g_clients[npc_id]->end_y) { iFirstY = g_clients[npc_id]->end_y;	iSecondY = g_clients[npc_id]->start_y; }
//else { iFirstY = g_clients[npc_id]->start_y; iSecondY = g_clients[npc_id]->end_y; }
//if (g_clients[npc_id]->start_x > g_clients[npc_id]->end_y) { iFirstX = g_clients[npc_id]->end_x;	iSecondX = g_clients[npc_id]->start_x; }
//else { iFirstX = g_clients[npc_id]->start_x; iSecondX = g_clients[npc_id]->end_x; }



			//reset_astar(npc_id); //  g_clients[npc_id]->m_listPath.clear();
			//addtimer(npc_id, target_id, OP_RESURRECT, 10000);


			//if (g_clients[npc_id]->target_id != -1)
			//	addtimer(npc_id, g_clients[npc_id]->target_id, OP_ASTAR_MOVE, 1000);
			//else
			//	addtimer(npc_id, g_clients[npc_id]->target_id, OP_RANDOM_MOVE, 1000);