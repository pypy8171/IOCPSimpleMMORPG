#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>

#include "protocol.h"

#include <sqlext.h>  

#include <atomic>
#include <queue>
#include <unordered_map>
#include <chrono>

#pragma comment (lib, "WS2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "lua53.lib")

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

using namespace chrono;

constexpr auto MAX_PACKET_SIZE = 255;
constexpr auto MAX_BUF_SIZE = 255;


constexpr auto MAX_ROW = 40;
constexpr auto MAX_COL = 40;

constexpr auto ROW_GAP = 20;
constexpr auto COL_GAP = 20;

constexpr auto VIEW_RADIUS = 17;

constexpr int MAX_TILE_ROW = 200;
constexpr int MAX_TILE_COL = 200;

constexpr int NPC_RADIUS = 10;

constexpr int LEVEL_UP_EXP = 100; // pow(2,level-1) * LEVEL_UP_EXP // 2의 0승 x LEVEL_UP_EXP 
constexpr int SERVER_CHATTER = 90000;

constexpr int VILLAGE_LEFTX = 394;
constexpr int VILLAGE_LEFTY = 394;
constexpr int VILLAGE_RIGHTX = 406;
constexpr int VILLAGE_RIGHTY = 406;

extern SQLHENV henv;
extern SQLHDBC hdbc;
extern SQLHSTMT hstmt;
extern SQLRETURN retcode;
extern SQLWCHAR dUser_name[MAX_NAME_LEN];
extern SQLINTEGER dUser_id, dUser_Level, dUser_posx, dUser_posy, dUser_hp, dUser_maxhp, dUser_exp, dUser_maxexp, dUser_monquest, dUser_monquestprocess, dUser_bossquest, dUser_bossquestprocess, dUser_normaldamage;
extern SQLLEN cbName, cbID, cbLevel, cbPosx, cbPosy, cbHp, cbMaxHp, cbExp, cbMaxExp, cbMonQuest,cbMonQuestProcess, cbBossQuest, cbBossQuestProcess, cbUserNormalDamage;

enum ENUMOP {
	OP_RECV = 0, OP_SEND, OP_ACCEPT, OP_RANDOM_MOVE, OP_RANDOM_MOVE_FINISH, OP_ASTAR_MOVE, OP_ENEMY_ATTACK, OP_RESURRECT, OP_PLAYER_MOVE,
	OP_HEAL, OP_PLAYER_ATTACK, OP_BOSS_ATTACK, OP_DATA_SAVE
};

enum C_STATUS { ST_FREE, ST_ALLOC, ST_ACTIVE, ST_SLEEP };
enum NPC_STATUS { S_NONACTIVE, S_ACTIVE };
enum BOSS_PATTERN { CROSS, X, WHOLE, RHOMBUS };

struct Event_Type {																	// 이벤트 큐 // 누가 언제 무엇을
	int obj_id;
	ENUMOP event_id;																// 랜덤이동이다 뭐다 이런것
	high_resolution_clock::time_point wakeup_time;
	int target_id;																	// 누구한테 어떤 공격을 하는가 이런것. // union 사용

	constexpr bool operator < (const Event_Type& _Left)const{
		return (wakeup_time > _Left.wakeup_time);
	}
	constexpr bool operator < (const Event_Type* _Left)const{
		return (wakeup_time > _Left->wakeup_time);
	}
};

class Compare {
public:
	bool operator() (const Event_Type* lhs, const Event_Type* rhs) const{
		return (lhs->wakeup_time > rhs->wakeup_time);
	}
};

extern priority_queue < Event_Type*, vector<Event_Type*>, Compare > timer_queue;


enum T_ALLOC { TA_FREE, TA_RESERVE, TA_ALLOC };

struct TILE_ALLOC
{
	T_ALLOC m_alloc;
};

extern TILE_ALLOC tile_alloc [WORLD_HEIGHT][WORLD_WIDTH];

struct tNode // 17
{
	short iCurIdxX = 0;	short iCurIdxY = 0;
	short iParentIdxX = 0;	short iParentIdxY = 0;
	short fFrom = 0;	short fDest = 0;	short fFinal = 0;
	char bClosed = false;	char bOpened = false;	char bObstacle = false;
};


class NodeCompare {
public:
	bool operator()(tNode* lhs, tNode* rhs)
	{
		return lhs->fFinal > rhs->fFinal;
	}
};

struct ViewList
{
	//SRWLOCK						rwlock;
	unordered_set<unsigned int>			viewlist; // 13만이 넘는 경우 때문
};

struct NearList
{
	unordered_set<unsigned int>			nearlist;
};

struct RemoveList
{
	unordered_set<unsigned int>			removelist;
};

struct EXOVER
{
	WSAOVERLAPPED	over;
	ENUMOP			op;
	char			io_buf[MAX_BUF_SIZE];
	union {
		WSABUF			wsabuf;
		SOCKET			c_socket;
		int				player_id;
	};
	int				target_id;
};

struct CLIENT
{
	
	mutex		m_cl;
	SOCKET		m_socket;
	int			m_id;
	EXOVER		m_recv_over;
	int			m_prev_size;
	char		m_packet_buf[MAX_PACKET_SIZE];
	atomic <C_STATUS>	m_status;
	short		x, y, row, col;
	char		m_otype;
	
	short		level, hp, maxhp, damage;
	WCHAR		name[MAX_ID_LEN + 1]; // 꽉차서 올수 있으므로 +1
	int			exp, maxexp;
	int			target_id;
	char		m_movenum = -1;
	unsigned	m_move_time;

	bool		is_dead = false;
	bool		have_mon_quest = false;
	bool		clear_mon_quest = false;
	short		killscore_blueslime = 0;
	bool		have_boss_quest = false;
	bool		clear_boss_quest = false;
	short		killscore_boss = 0;

	high_resolution_clock::time_point move_delay = high_resolution_clock::now();
	high_resolution_clock::time_point attack_delay = high_resolution_clock::now();
	high_resolution_clock::time_point data_save = high_resolution_clock::now();

	bool		is_move = false; 

	// 시야처리
	ViewList	m_viewlist;
	NearList	m_nearlist;
	//RemoveList	m_removelist;

	Event_Type  m_event;

	// sciprt에서 사용
	lua_State*	L;
	mutex		lua_l;


	// 길찾기 용도
	priority_queue<tNode*, vector<tNode*>, NodeCompare>	m_openlist;
	list<tNode*>										m_listPath;
	tNode												m_arrNode[NPC_RADIUS + 1][NPC_RADIUS + 1];
	short												start_x, start_y, end_x, end_y, reserved_x = -1, reserved_y = -1;
};

struct SECTOR
{
	mutex						sector_lock;
	unordered_set<unsigned int>	m_setPlayerList;
	short						m_StartX, m_StartY, m_EndX, m_EndY;
};

struct TILE
{
	atomic <C_STATUS>	m_status;
	int					m_id;
	short				x, y, row, col;
	char				m_otype;
	WCHAR				name[MAX_ID_LEN + 1];
};

extern SECTOR g_sectors[MAX_ROW][MAX_COL];
extern TILE* g_tile[MAX_TILE];
extern CLIENT* g_clients[MAX_USER + MAX_NPC]; // 순수 클라이언트는 *로 안하는게 좋을

extern mutex timer_lock;

extern HANDLE g_iocp;
extern int g_curUser;

extern void addtimer(int id, int target, ENUMOP op, unsigned int timer);

extern int API_send_message(lua_State* L);
extern int API_get_x(lua_State* L);
extern int API_get_y(lua_State* L);
extern int API_get_move_num(lua_State* L);
extern int API_set_move_num(lua_State* L);
extern int API_random_move(lua_State* L);
extern int API_set_monster_stat(lua_State* L);
extern int API_boss_ai(lua_State* L);
extern int API_boss_attack(lua_State* L);

void show_error();
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

// 지형 정보도 db에 있어야 하나?, 그냥 처음 서버 킬때 랜덤으로 뿌려주나, 

extern int a;


//struct USER
//{
//	bool		have_mon_quest = false;
//	bool		clear_mon_quest = false;
//	short		killscore_blueslime = 0;
//	bool		have_boss_quest = false;
//	bool		clear_boss_quest = false;
//
//	high_resolution_clock::time_point move_delay = high_resolution_clock::now();
//	high_resolution_clock::time_point attack_delay = high_resolution_clock::now();
//	high_resolution_clock::time_point data_save = high_resolution_clock::now();
//
//	// 시야처리
//	ViewList	m_viewlist;
//	NearList	m_nearlist;
//};
//
//struct MONSTER
//{
//	ViewList	m_viewlist;
//	NearList	m_nearlist;
//
//	// 길찾기 용도
//	priority_queue<tNode*, vector<tNode*>, NodeCompare>	m_openlist;
//	list<tNode*>								m_listPath;
//	tNode										m_arrNode[NPC_RADIUS + 1][NPC_RADIUS + 1];
//	short start_x, start_y, end_x, end_y;
//};
//
//struct BOSS
//{
//
//};
//
//struct NPC
//{
//
//};
//
//struct 
// 아래에 + socket 정보들
//{
//	short		x, y, row, col;
//
//};
//
//extern USER* g_users[MAX_USER];
//extern MONSTER* g_monsters[MAX_NPC - MAX_QUEST_NPC];
//extern NPC* g_npcs[MAX_QUEST_NPC];
//extern BOSS* g_boss[MAX_BOSS];