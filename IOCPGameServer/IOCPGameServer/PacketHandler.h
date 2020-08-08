#pragma once
#include "define.h"
#include "extern.h"

class CPacketHandler
{
public:
	void recv_packet_construct(int user_id, int io_byte);
	void process_packet(int user_id, char buf[]);
	void send_packet(int user_id, void* packet);

	void enter_game(int user_id, WCHAR name[]);
	void send_login_ok_packet(int user_id);
	void send_enter_packet(int user_id, int o_id);
	void send_chat_packet(int user_id, int chatter, char len, WCHAR mess[]);
	void send_leave_packet(int user_id, int o_id);
	void send_login_fail_packet(int user_id);
	void send_stat_change(int user_id);
	void send_boss_attack(int user_id, int boss_id, char pattern);
	void disconnect(int user_id);

	void send_move_packet(int user_id, int mover);

	void do_move(int user_id, int direction);
	void npc_move(int id, ENUMOP op);

	bool is_player(int id) { return id < NPC_START_IDX; }
	bool is_npc(int id){ if (id >= NPC_START_IDX && id < NPC_START_IDX + MAX_QUEST_NPC)	return	true;	else return false; }
	bool is_monster(int id) { if (id >= NPC_START_IDX + MAX_QUEST_NPC && id < NPC_START_IDX + MAX_NPC) return true; else return false; }
	//bool is_tile(int id) {	if (id >= TILE_START_IDX && id < TILE_START_IDX + MAX_TILE)	return	true;	else return false;}
	bool is_tile(int id) { if (id >= TILE_START_IDX && id < TILE_START_IDX + MAX_TILE)		return	true;	else return false; }
	void activate_npc(int id);
	void activate_tile(int id);

	//
	//find_path();
	bool col_check_ct(int user_x,int user_y, int tile_id);

public:
	SINGLE(CPacketHandler);
};

