#pragma once
#include <iostream>


#include <unordered_set>
#include <mutex>
#include <map>
#include <set>
#include <vector>

using namespace std;

constexpr int MAX_ID_LEN = 50;
constexpr int MAX_NAME_LEN = 50;
constexpr int MAX_STR_LEN = 80;

constexpr auto MAX_USER = 20000;
constexpr int MAX_NPC = 2000;
constexpr int MAX_QUEST_NPC = 2;
constexpr int MAX_BOSS = 2;
constexpr int MAX_TILE = 40000;
constexpr int NPC_START_IDX = MAX_USER;
constexpr int TILE_START_IDX = MAX_USER + MAX_NPC;

constexpr int START_POINT_X = 400;
constexpr int START_POINT_Y = 400;

#define WORLD_WIDTH		800
#define WORLD_HEIGHT	800

#define SERVER_PORT		9000

#define C2S_LOGIN		1
#define C2S_MOVE		2
#define C2S_ATTACK		3 
#define C2S_CHAT		4
#define C2S_LOGOUT		5
#define C2S_TELEPORT	6

#define S2C_LOGIN_OK		1
#define S2C_LOGIN_FAIL		2 
#define S2C_MOVE			3
#define S2C_ENTER			4
#define S2C_LEAVE			5
#define S2C_CHAT			6
#define S2C_STAT_CHANGE		7 
#define S2C_MONSTER_ATTACK	8
#define S2C_PLAYER_DEAD		9

#pragma pack(push ,1)

struct sc_packet_login_ok { // 16
	char size;
	char type;
	int id;
	short x, y;
	short hp;
	short level;
	int	exp;
};

struct sc_packet_login_fail
{
	char size;
	char type;
};

struct sc_packet_move {  // 8
	char size;
	char type;
	int id;
	short x, y;
	unsigned move_time;
};

constexpr unsigned char O_PLAYER = 0;
constexpr unsigned char O_NPC = 1;
constexpr unsigned char O_BLUES = 2; 
constexpr unsigned char O_REDS = 3; 
constexpr unsigned char O_BOSSS = 4;
constexpr unsigned char O_TILE = 5;

struct sc_packet_enter { //
	char size;
	char type;
	int id;
	wchar_t name[MAX_ID_LEN]; // 이름 char로 되있음 제시된 protocol에는
	char o_type;
	short x, y;
};

struct sc_packet_leave {
	char size;
	char type;
	int id;
};

struct sc_packet_chat {
	char size;
	char type;
	int	 id;
	wchar_t chat[MAX_STR_LEN];
};

struct sc_packet_stat_change
{
	char size;
	char type;
	short hp, level;
	int exp;
	int id; // 추가함
	short x, y;
};

struct sc_packet_monster_attack
{
	char size;
	char type;
	int  boss_id;
	short x, y;
	char pattern;
};

constexpr unsigned char D_UP = 0;
constexpr unsigned char D_DOWN = 1;
constexpr unsigned char D_LEFT = 2;
constexpr unsigned char D_RIGHT = 3;

struct cs_packet_login {
	char	size;
	char	type;
	wchar_t	name[MAX_ID_LEN]; // 역시 char로
};

struct cs_packet_move {
	char	size;
	char	type;
	char	direction;
	unsigned move_time;
};

struct cs_packet_attack
{
	char size;
	char type;
};

struct cs_packet_chat {
	char size;
	char type;
	wchar_t message[MAX_STR_LEN];
};

struct cs_packet_logout {
	char size;
	char type;
};

struct cs_packet_teleport{
	char size;
	char type;
};

#pragma pack (pop)