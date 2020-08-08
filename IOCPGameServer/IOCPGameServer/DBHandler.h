#pragma once
#include "define.h"
#include "extern.h"

class CDBHandler
{
public:
	void init();
	void DB_login(int user_id, WCHAR name[]);
	void DB_logout(int user_id);
	void DB_save_userdata(int user_id);
	void DB_insert_user(int user_id, WCHAR name[]);
	void DB_finish();

public:
	SINGLE(CDBHandler);
};

