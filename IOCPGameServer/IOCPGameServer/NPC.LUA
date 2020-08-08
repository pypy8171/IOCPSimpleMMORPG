myid = 0
posx = 0
posy = 0
hp = 0
type = 0
level = 0
boss_changeposX = 0
boss_changeposY = 0

function set_uid(x)
myid = x
end

function event_player_move(p_id)
	if  math.abs( API_get_x(p_id) - API_get_x(myid)) + math.abs(API_get_y(p_id) - API_get_y(myid)) <= 1 then
			API_send_message(myid, p_id, "~~~hello~~~");
			API_set_move_num(myid,3);
	end
end

function event_ai_finish(myid)
	if API_get_move_num(myid) == 0 then
		API_send_message(myid, p_id, "~~~bye~~~");
	end
end

function  event_ai_move(myid, eventtype)
		API_random_move(myid, eventtype);
end


function save_monster_stat(id, lv, x, y, _hp, _type)
	myid = id;
	level = lv;
	posx = x;
	posy =y; 
	hp = _hp; 
	type = _type;
	boss_changeposX = x;
	boss_changeposY = y;
end

function set_monster_stat()
	API_set_monster_stat(myid,level,posx,posy,hp,type);
end

function boss_ai(p_id)
	p_posx = API_get_x(p_id);
	p_posy = API_get_y(p_id);
	boss_changeposX = API_get_x(myid)
	boss_changeposY = API_get_y(myid);

	local pattern = math.random(0,2);
	local attackP = {};

	if(pattern == 0) then
		for i = 1,5 do
			table.insert(attackP,{x = boss_changeposX - i, y = boss_changeposY});
			table.insert(attackP,{x = boss_changeposX + i, y = boss_changeposY});
			table.insert(attackP,{x = boss_changeposX, y = boss_changeposY - i});
			table.insert(attackP,{x = boss_changeposX, y = boss_changeposY + i});
		end
		API_boss_ai(pattern, myid)
		for i = 1,20 do
			API_boss_attack(myid, p_id, attackP[i].x,attackP[i].y)
		end
	end
	if(pattern == 1) then
		for i = 1,5 do
			table.insert(attackP,{x = boss_changeposX - i, y =boss_changeposY - i});
			table.insert(attackP,{x = boss_changeposX - i, y =boss_changeposY + i});
			table.insert(attackP,{x = boss_changeposX + i, y =boss_changeposY + i});
			table.insert(attackP,{x = boss_changeposX + i, y =boss_changeposY - i});
		end
		API_boss_ai(pattern, myid)
		for i = 1,20 do
			API_boss_attack(myid, p_id, attackP[i].x,attackP[i].y)
		end
	end
	if(pattern == 2) then
		for i = 1, 11 do
			table.insert(attackP,{x = boss_changeposX - i + 6, y = boss_changeposY - 3});
			table.insert(attackP,{x = boss_changeposX - i + 6, y = boss_changeposY - 1});
			table.insert(attackP,{x = boss_changeposX - i + 6, y = boss_changeposY + 1});
			table.insert(attackP,{x = boss_changeposX - i + 6, y = boss_changeposY + 3});
		end
		API_boss_ai(pattern, myid)
		for i = 1, 44 do
			API_boss_attack(myid,p_id, attackP[i].x, attackP[i].y);
		end
	end
end