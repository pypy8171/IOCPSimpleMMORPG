#pragma once
#include "define.h"
#include "extern.h"

class CAIHandler
{
public:
	void astar_finding(int npc_id, int target_id, ENUMOP event_type);
	void init_node(int npc_id, int target_id);
	void find_path(int npc_id, int target_id);
	void add_openlist(int npc_id, int _iX, int _iY, int _iOrgX, int _iOrgY);
	void cal_cost(int npc_id, int _iX, int _iY, int _iOrgX, int _iOrgY);

	void astar_move(int id, int target_id);

	bool is_conflict(int id1, int id2);
	int	 get_dist(int id1, int id2);
	bool is_range(int id1, int id2);

	void reset_astar(int npc_id);

	void do_attack(int user_id);
	void do_resurrect(int user_id);

	void manage_exp(int user_id, int amount);
	void manage_hp(int user_id, int damage);
	void heal(int user_id);

public:
	SINGLE(CAIHandler);
};