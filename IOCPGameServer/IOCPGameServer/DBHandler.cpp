#include "DBHandler.h"
#include "PacketHandler.h"
#include "ViewProcessing.h"

CDBHandler::CDBHandler()
{
}

CDBHandler::~CDBHandler()
{
}

void CDBHandler::init()
{
	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"game_db_odbc", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					std::cout << "db access ok" << std::endl;
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				}
				else HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
			}
		}
	}
}

void CDBHandler::DB_login(int user_id, WCHAR name[])
{
	WCHAR querystr[100] = L"EXEC select_player ";
	wcscat_s(querystr, name);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)querystr, SQL_NTS); // by ~ 소팅 기준

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{

		std::cout << "Select OK" << std::endl;

		retcode = SQLBindCol(hstmt, 1, SQL_C_LONG,		&dUser_id,	100, &cbID); // c스타일로 할때는 _c해줘야함
		retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR,		dUser_name,	MAX_NAME_LEN, &cbName);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG,		&dUser_Level, 100, &cbLevel);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG,		&dUser_posx, 100, &cbPosx);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG,		&dUser_posy, 100, &cbPosy);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG,		&dUser_hp, 100, &cbHp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG,		&dUser_maxhp, 100, &cbMaxHp);
		retcode = SQLBindCol(hstmt, 8, SQL_C_LONG,		&dUser_exp, 100, &cbExp);
		retcode = SQLBindCol(hstmt, 9, SQL_C_LONG,		&dUser_maxexp, 100, &cbMaxExp);
		retcode = SQLBindCol(hstmt, 10, SQL_C_BINARY,	&dUser_monquest, 100, &cbMonQuest);
		retcode = SQLBindCol(hstmt, 11, SQL_C_LONG,		&dUser_monquestprocess, 100, &cbMonQuestProcess);
		retcode = SQLBindCol(hstmt, 12, SQL_C_BINARY,	&dUser_bossquest, 100, &cbBossQuest);
		retcode = SQLBindCol(hstmt, 13, SQL_C_LONG,		&dUser_bossquestprocess, 100, &cbBossQuestProcess);
		retcode = SQLBindCol(hstmt, 14, SQL_C_LONG,		&dUser_normaldamage, 100, &cbUserNormalDamage);


		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
		{
			show_error();
			HandleDiagnosticRecord(hstmt, SQL_HANDLE_DBC, retcode);
			CPacketHandler::GetInst()->disconnect(g_clients[user_id]->m_id);

			if (retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLCancel(hstmt);
			}
			return;
		}
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			// 만약 active이면 여기서 break 걸고 못들어오게 한다. 끊어버림.
			bool alive = true;

			for (int i = 0; i < g_curUser; ++i)
			{
				if (g_clients[i]->m_status != ST_ACTIVE)
					continue;

				wstring name = (WCHAR*)dUser_name;
				name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

				if (0 == (name.compare(g_clients[i]->name)))
				{
					wcout << "user : " << name;
					wprintf(L" %d, %d, %d, %d\n", dUser_posx, dUser_posy, dUser_Level, dUser_hp);
					HandleDiagnosticRecord(hstmt, SQL_HANDLE_DBC, retcode);
					CPacketHandler::GetInst()->disconnect(user_id);
					alive = false;
					cout << "로그인 실패 ! 이미 해당 아이디로 로그인하였습니다." << endl;
					break;
				}
			}

			if (alive)
			{
				//g_clients[user_id]->m_id = dUser_id;
				wcscpy_s(g_clients[user_id]->name, reinterpret_cast<WCHAR*>(dUser_name));
				g_clients[user_id]->level = (short)dUser_Level;
				g_clients[user_id]->x = (short)dUser_posx;
				g_clients[user_id]->y = (short)dUser_posy;
				g_clients[user_id]->hp = (short)dUser_hp;
				g_clients[user_id]->maxhp = (short)dUser_maxhp;
				g_clients[user_id]->exp = (int)dUser_exp;
				g_clients[user_id]->maxexp = (int)dUser_maxexp;
				g_clients[user_id]->clear_mon_quest = (bool)dUser_monquest;
				g_clients[user_id]->killscore_blueslime = (int)dUser_monquestprocess;
				g_clients[user_id]->clear_boss_quest = (bool)dUser_bossquest;
				g_clients[user_id]->killscore_boss = (int)dUser_bossquestprocess;
				g_clients[user_id]->damage = (int)dUser_normaldamage;

				if (g_clients[user_id]->killscore_blueslime > 0)
					g_clients[user_id]->have_mon_quest = true;
				if (g_clients[user_id]->killscore_boss > 0)
					g_clients[user_id]->have_boss_quest = true;
		

				CViewProcessing::GetInst()->change_sector(user_id, g_clients[user_id]->x, g_clients[user_id]->y, g_clients[user_id]->col, g_clients[user_id]->row);

				// sector에 집어 넣어준다.
				short col = g_clients[user_id]->x / COL_GAP;
				short row = g_clients[user_id]->y / ROW_GAP;
				g_sectors[row][col].m_setPlayerList.emplace(user_id);
				g_clients[user_id]->row = row;
				g_clients[user_id]->col = col;

				CPacketHandler::GetInst()->enter_game(user_id, name);

				wcout << "user : " << g_clients[user_id]->name << " login" << endl;
			}
		}
		else
		{
			// 여기서 새로 집어넣어준다. db에도 
			SQLCancel(hstmt);
			cout << "DB에 존재하지 않는 아이디 입니다. DB에 추가합니다." << endl;
			DB_insert_user(user_id, name);

		}
	}
	else
	{
		WCHAR id[10];
		_itow_s(user_id, id, 10);
		CPacketHandler::GetInst()->enter_game(user_id, id);
		//HandleDiagnosticRecord(hstmt, SQL_HANDLE_DBC, retcode);
		//CPacketHandler::GetInst()->disconnect(g_clients[user_id]->m_id);
	}

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLCancel(hstmt);
	}
}

void CDBHandler::DB_logout(int user_id) // 처음에 거부 당할때는 login fail로 가야됨
{
	//나갈때 뷰리스트에 있던 npc들 nonactive로 해야할 듯 // leave에서 해줌 -> npc움직일때 모든 플레이어에게서 벗어나면 nonactive로 만듬
	WCHAR level[3], x[5], y[5], hp[10], maxhp[10], exp[10], maxexp[10], monquest[2], monquestprocess[5], bossquest[2], bossquestprocess[5], normaldamage[5];;
	_itow_s(g_clients[user_id]->level, level, 10); // 마지막 인자는 진수
	_itow_s(g_clients[user_id]->x, x, 10);
	_itow_s(g_clients[user_id]->y, y, 10);
	_itow_s(g_clients[user_id]->hp, hp, 10);
	_itow_s(g_clients[user_id]->maxhp, maxhp, 10);
	_itow_s(g_clients[user_id]->exp, exp, 10);
	_itow_s(g_clients[user_id]->maxexp, maxexp, 10);
	_itow_s(g_clients[user_id]->clear_mon_quest, monquest, 10);
	_itow_s(g_clients[user_id]->killscore_blueslime, monquestprocess, 10);
	_itow_s(g_clients[user_id]->clear_boss_quest, bossquest, 10);
	_itow_s(g_clients[user_id]->killscore_boss, bossquestprocess, 10);
	_itow_s(g_clients[user_id]->damage, normaldamage, 10);

	WCHAR token[2] = L",";

	WCHAR querystr[100] = L"EXEC select_logout_player ";
	wcscat_s(querystr, g_clients[user_id]->name); wcscat_s(querystr, token);
	wcscat_s(querystr, level);	wcscat_s(querystr, token);
	wcscat_s(querystr, x);	wcscat_s(querystr, token);
	wcscat_s(querystr, y);	wcscat_s(querystr, token);
	wcscat_s(querystr, hp); wcscat_s(querystr, token);
	wcscat_s(querystr, maxhp); wcscat_s(querystr, token);
	wcscat_s(querystr, exp); wcscat_s(querystr, token);
	wcscat_s(querystr, maxexp); wcscat_s(querystr, token);
	wcscat_s(querystr, monquest); wcscat_s(querystr, token);
	wcscat_s(querystr, monquestprocess); wcscat_s(querystr, token);
	wcscat_s(querystr, bossquest); wcscat_s(querystr, token);
	wcscat_s(querystr, bossquestprocess); wcscat_s(querystr, token);
	wcscat_s(querystr, normaldamage);

	//retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)querystr, SQL_NTS); // by ~ 소팅 기준

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		//cout << "Select OK" << endl;
		//wcout << "user : " << g_clients[user_id]->name << " logout" << endl;
		// sector에서 뺀다.
		short col = g_clients[user_id]->col;
		short row = g_clients[user_id]->row;
		g_sectors[row][col].m_setPlayerList.erase(g_clients[user_id]->m_id);
	}
	else
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_DBC, retcode);
	}

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) // stress 테스트할때 여기서 에러날때 있음 disconnect할때
	{
		SQLCancel(hstmt);
	}

	ZeroMemory(&g_clients[user_id]->name, sizeof(g_clients[user_id]->name));
}

void CDBHandler::DB_save_userdata(int user_id)
{
	//나갈때 뷰리스트에 있던 npc들 nonactive로 해야할 듯 // leave에서 해줌 -> npc움직일때 모든 플레이어에게서 벗어나면 nonactive로 만듬
	WCHAR level[3], x[5], y[5], hp[10], maxhp[10], exp[10], maxexp[10], monquest[2], monquestprocess[5], bossquest[2], bossquestprocess[5], normaldamage[5];
	_itow_s(g_clients[user_id]->level, level, 10); // 마지막 인자는 진수
	_itow_s(g_clients[user_id]->x, x, 10);
	_itow_s(g_clients[user_id]->y, y, 10);
	_itow_s(g_clients[user_id]->hp, hp, 10);
	_itow_s(g_clients[user_id]->maxhp, maxhp, 10);
	_itow_s(g_clients[user_id]->exp, exp, 10);
	_itow_s(g_clients[user_id]->maxexp, maxexp, 10);
	_itow_s(g_clients[user_id]->clear_mon_quest, monquest, 10);
	_itow_s(g_clients[user_id]->killscore_blueslime, monquestprocess, 10);
	_itow_s(g_clients[user_id]->clear_boss_quest, bossquest, 10);
	_itow_s(g_clients[user_id]->killscore_boss, bossquestprocess, 10);
	_itow_s(g_clients[user_id]->damage, normaldamage, 10);

	WCHAR token[2] = L",";

	WCHAR querystr[100] = L"EXEC save_player_data ";
	wcscat_s(querystr, g_clients[user_id]->name); wcscat_s(querystr, token);
	wcscat_s(querystr, level);	wcscat_s(querystr, token);
	wcscat_s(querystr, x);	wcscat_s(querystr, token);
	wcscat_s(querystr, y);	wcscat_s(querystr, token);
	wcscat_s(querystr, hp); wcscat_s(querystr, token);
	wcscat_s(querystr, maxhp); wcscat_s(querystr, token);
	wcscat_s(querystr, exp); wcscat_s(querystr, token);
	wcscat_s(querystr, maxexp); wcscat_s(querystr, token);
	wcscat_s(querystr, monquest); wcscat_s(querystr, token);
	wcscat_s(querystr, monquestprocess); wcscat_s(querystr, token);
	wcscat_s(querystr, bossquest); wcscat_s(querystr, token);
	wcscat_s(querystr, bossquestprocess); wcscat_s(querystr, token);
	wcscat_s(querystr, normaldamage);

	//retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)querystr, SQL_NTS); // by ~ 소팅 기준

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
	//	wcout << "save user : " << g_clients[user_id]->name << endl;
	}
	else
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_DBC, retcode);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLCancel(hstmt);
	}
}

void CDBHandler::DB_insert_user(int user_id, WCHAR name[])
{
	WCHAR querystr[80] = L"EXEC insert_player_data ";
	wcscat_s(querystr, name);

	//retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)querystr, SQL_NTS); // by ~ 소팅 기준

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		cout << "Insert Success" << endl;
		SQLCancel(hstmt);
		DB_login(user_id, name);
		return;
	}
	else
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_DBC, retcode);
	}


	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLCancel(hstmt);
	}
}

void CDBHandler::DB_finish()
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	SQLDisconnect(hdbc);
}
