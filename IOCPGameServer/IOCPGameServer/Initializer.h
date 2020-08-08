#pragma once

#include "define.h"
#include "extern.h"


class CInitializer
{
public:
	void initialize_clients();
	void initialize_sectors();
	void initialize_npcs();
	void initialize_tiles();
	void initialize_db();

public:
	SINGLE(CInitializer);
};

