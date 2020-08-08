#pragma once
#include "define.h"
#include "protocol.h"

class CViewProcessing
{
public:
	void	create_nearlist(int user_id, int row_start, int col_start, int row_end, int col_end);
	void	create_nearlist_p(int user_id, int row_start, int col_start, int row_end, int col_end);
	void	create_nearlist_pp(int user_id, int row_start, int col_start, int row_end, int col_end);
	void	create_nearlist_np(int user_id, int row_start, int col_start, int row_end, int col_end);
	void	create_nearlist_pn(int user_id, int row_start, int col_start, int row_end, int col_end);

	void	create_viewlist_pn(int npc_id, int row_start, int col_start, int row_end, int col_end);

	void	create_tilelist_nt(int user_id, int row_start, int col_start, int row_end, int col_end);
	void	create_tilelist_pt(int user_id, int row_start, int col_start, int row_end, int col_end);

	void	create_obstacle_nt(int user_id, int row_start, int col_start, int row_end, int col_end);

	void	check_near_view(int user_id);
	void	check_near_view_pn(int npc_id);

	void	change_sector(int user_id, int x, int y, int col, int row);

	bool	is_obstacle_check(int id1, int id2);
	bool	is_near_check_pp(int id1, int id2);
	bool	is_neartile_check_nt(int id1, int id2);
	bool	is_neartile_check_pt(int id1, int id2);
	//bool	is_near_check_pn(int id1, int id2);
	bool	is_under_ten(int id1, int id2);

	bool	is_attack_range_pn(int user_id, int mon_id);

public:
	SINGLE(CViewProcessing);
};
