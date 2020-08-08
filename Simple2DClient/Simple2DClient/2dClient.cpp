#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <queue>
#include <vector>
#include <list>

#include <locale>
#include <codecvt>
#include <string>

using namespace std;
using namespace chrono;

#include "protocol.h"

sf::TcpSocket g_socket;
  
constexpr auto SCREEN_WIDTH = 20;
constexpr auto SCREEN_HEIGHT = 20;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH /2 + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH /2 + 10;
constexpr auto BUF_SIZE = 200;

int iCurChatNum = 0;

void send_packet(void* packet);

int g_left_x;
int g_top_y;
int g_myid;

sf::RenderWindow* g_window;
sf::Font g_font;

bool message = false;
bool capslock = false;
wchar_t szmessage[MAX_STR_LEN];

constexpr unsigned char O_SKILL_1 = 10;

bool b_chat = false;

class OBJECT {
public:
	high_resolution_clock::time_point m_skill_time_out;
private:
	sf::Sprite m_sprite;
	bool m_showing;

	char m_mess[MAX_STR_LEN];
	high_resolution_clock::time_point m_time_out;
	sf::Text m_text;
	list<sf::Text> text_list;
	sf::Text m_name;

	vector<bool> quest_vec;

	sf::Text m_text_pp;
	high_resolution_clock::time_point m_time_out_pp;

public:
	char m_otype;
	int m_x, m_y;
	int m_hp, m_exp, m_level;
	int m_pattern;
	WCHAR name[MAX_ID_LEN];
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		m_time_out = high_resolution_clock::now();
	}
	OBJECT() {
		m_showing = false;
		m_time_out = high_resolution_clock::now();
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;

		if (m_otype == O_SKILL_1)
		{
			if (high_resolution_clock::now() < m_skill_time_out) // 시간 지나면 --skillimage_num 해줘야 하는데
			{
				m_sprite.setPosition(rx, ry);
				g_window->draw(m_sprite);
				m_name.setPosition(rx - 10, ry - 10);
				g_window->draw(m_name);
			}
		}
		else
		{
			//m_name.getString().toWideString();
			m_sprite.setPosition(rx, ry);
			g_window->draw(m_sprite);
			//sf::String str = L"한";
			m_name.setPosition(rx - 10, ry - 10);
			//m_name.getString().toWideString();
			g_window->draw(m_name);
		}

		if (high_resolution_clock::now() < m_time_out)
		{
			int idx = 0;
			for (auto& text : text_list)
			{
				text.setFillColor(sf::Color(255, 255, 0));
				text.setPosition(100, 900 + 30 * idx);
				++idx;
			}

			const sf::Texture& texture = g_font.getTexture(10);
			sf::Glyph glyph = g_font.getGlyph(10, 10, sf::Text::Bold);

			for (auto& text : text_list)
			{
				g_window->draw(text);
			}
		}

		if (high_resolution_clock::now() < m_time_out_pp)
		{
			m_text_pp.setFillColor(sf::Color(0, 120, 255));
			m_text_pp.setPosition(rx - 20, ry - 50);
			
			const sf::Texture& texture = g_font.getTexture(12);
			sf::Glyph glyph = g_font.getGlyph(12, 12, sf::Text::Bold);

			g_window->draw(m_text_pp);
		}
	}
	void set_name(WCHAR str[]) {
		m_name.setFont(g_font);
		
		m_name.setString(wstring(str));
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}
	void add_chat(wchar_t chat[]) {
		if (text_list.size() >= 10)
		{
			text_list.pop_back();
			--iCurChatNum;
		}
		m_text.setFont(g_font);
		m_text.setString(chat);
		text_list.emplace_front(m_text);
		m_time_out = high_resolution_clock::now() + 1s;
	}
	void add_chat_pp(wchar_t chat[]) {
		m_text_pp.setFont(g_font);
		m_text_pp.setString(chat);
		m_time_out_pp = high_resolution_clock::now() + 3s;
	}
};

OBJECT* avatar;
unordered_map <int, OBJECT> npcs;
unordered_map <int, OBJECT> skills;
int skillimage_num = 0;


OBJECT tile;

OBJECT white_tile;
OBJECT black_tile;

sf::Texture* board;
sf::Texture* pieces;
sf::Texture* bricks;
sf::Texture* blueslime;
sf::Texture* redslime;
sf::Texture* bossslime;
sf::Texture* stone;
sf::Texture* boss_waterfall;

void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	bricks = new sf::Texture;
	blueslime = new sf::Texture;
	redslime = new sf::Texture;
	bossslime = new sf::Texture;
	stone = new sf::Texture;
	boss_waterfall = new sf::Texture;
	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		while (true);
	}
	else {
		cout << "Font Loading Success!\n";
	}
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("chess2.png");
	bricks->loadFromFile("brick.png");
	blueslime->loadFromFile("blueslime.png");
	redslime->loadFromFile("redslime.png");
	bossslime->loadFromFile("bossslime.png");
	stone->loadFromFile("stone.png");
	boss_waterfall->loadFromFile("waterfall.png");
	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = new OBJECT{ *pieces, 128, 0, 64, 64 };
	avatar->move(4, 4);
	avatar->m_otype = O_PLAYER;
}

void client_finish()
{
	delete board;
	delete pieces;
	delete bricks;
	delete blueslime;
	delete redslime;
	delete bossslime;
	delete stone;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case S2C_LOGIN_OK:
	{
		sc_packet_login_ok* my_packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
		g_myid = my_packet->id;
		avatar->move(my_packet->x, my_packet->y);
		avatar->m_hp = my_packet->hp;
		avatar->m_exp = my_packet->exp;
		avatar->m_level = my_packet->level;
		g_left_x = my_packet->x - (SCREEN_WIDTH / 2);
		g_top_y = my_packet->y - (SCREEN_HEIGHT / 2);
		avatar->show();
	}
	break;
	case S2C_LOGIN_FAIL:
	{
		sc_packet_login_fail* packet = reinterpret_cast<sc_packet_login_fail*>(ptr);
		client_finish();
	}
	case S2C_ENTER:
	{
		sc_packet_enter* my_packet = reinterpret_cast<sc_packet_enter*>(ptr);
		int id = my_packet->id;

		char type = my_packet->o_type;

		if (id == g_myid) {
			avatar->move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - (SCREEN_WIDTH / 2);
			g_top_y = my_packet->y - (SCREEN_HEIGHT / 2);
			avatar->show();
		}
		else {
			if (id < NPC_START_IDX)
			{
				npcs[id] = OBJECT{ *pieces, 64, 0, 64, 64 };
			}
			else if (id >= NPC_START_IDX && id < TILE_START_IDX)
			{
				if (type == O_BLUES)
				{
					npcs[id] = OBJECT{ *blueslime, 0, 0, 64, 64 };
					npcs[id].m_otype = O_BLUES;
				}
				else if (type == O_REDS)
				{
					npcs[id] = OBJECT{ *redslime, 0, 0, 64, 64 };
					npcs[id].m_otype = O_REDS;
				}
				else if (type == O_BOSSS)
				{
					npcs[id] = OBJECT{ *bossslime, 0, 0, 64, 64 };
					npcs[id].m_otype = O_BOSSS;
				}
				else if (type == O_NPC)
				{
					npcs[id] = OBJECT{ *pieces, 0, 0, 64, 64 };
					npcs[id].m_otype = O_NPC;
				}

			}
			else
			{
				npcs[id] = OBJECT{ *stone, 0, 0, 64, 64 };
			}
			wcscpy_s(npcs[id].name, my_packet->name);
			npcs[id].set_name(my_packet->name);
			npcs[id].move(my_packet->x, my_packet->y);
			npcs[id].show();
		}
	}
	break;
	case S2C_MOVE:
	{
		sc_packet_move* my_packet = reinterpret_cast<sc_packet_move*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar->move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - (SCREEN_WIDTH / 2);
			g_top_y = my_packet->y - (SCREEN_HEIGHT / 2);
		}
		else {
			if (0 != npcs.count(other_id))
			{
				npcs[other_id].move(my_packet->x, my_packet->y);
				//npcs[other_id].show();
			}
		}
	}
	break;

	case S2C_LEAVE:
	{
		sc_packet_leave* my_packet = reinterpret_cast<sc_packet_leave*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar->hide();
		}
		else {
			if (0 != npcs.count(other_id))
				npcs[other_id].hide();
		}
	}
	break;
	case S2C_STAT_CHANGE:
	{
		sc_packet_stat_change* my_packet = reinterpret_cast<sc_packet_stat_change*>(ptr);

		// 다른 애들의 체력까지 알아야 할 수도 있음 이건 고려as
		avatar->m_hp = my_packet->hp;
		avatar->m_exp = my_packet->exp;
		avatar->m_level = my_packet->level;
		avatar->m_x = my_packet->x;
		avatar->m_y = my_packet->y;
	}
	break;
	case S2C_CHAT:
	{
		sc_packet_chat* my_packet = reinterpret_cast<sc_packet_chat*>(ptr);
		int o_id = my_packet->id; // 이 아이디는 전쳇일때 사용될거임 npcs 에는 다른 클라 id가 있으므로
		if (o_id < MAX_USER)
		{
			if(o_id == g_myid)
				avatar->add_chat_pp(my_packet->chat);
			else
				npcs[o_id].add_chat_pp(my_packet->chat);
		}
		else if (o_id >= MAX_USER && o_id < MAX_USER + MAX_QUEST_NPC)
		{
			npcs[o_id].add_chat_pp(my_packet->chat);
		}
		else
		{
			avatar->add_chat(my_packet->chat);
		}


	}
	break;
	case S2C_MONSTER_ATTACK:
	{
		sc_packet_monster_attack* packet = reinterpret_cast<sc_packet_monster_attack*>(ptr);
		int boss_id = packet->boss_id;

		npcs[boss_id].m_x = packet->x;
		npcs[boss_id].m_y = packet->y;
		npcs[boss_id].m_pattern = packet->pattern;
		
		
		if (packet->pattern == 0) // 시간 끝나면 없애야함
		{
			for (int i = 0; i <= 10; ++i)
			{
				if (i == 5)
					continue;
				skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
				skills[skillimage_num].move(packet->x - i + 5, packet->y);
				skills[skillimage_num].show();
				skills[skillimage_num].m_otype = O_SKILL_1;
				skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
				++skillimage_num;
			}
			for (int i = 0; i <= 10; ++i)
			{
				if (i == 5)
					continue;
				skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
				skills[skillimage_num].move(packet->x, packet->y - i + 5);
				skills[skillimage_num].show();
				skills[skillimage_num].m_otype = O_SKILL_1;
				skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
				++skillimage_num;
			}
		}
		else if (packet->pattern == 1)
		{
			for (int i = 0; i < 5; ++i)
			{
				skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
				skills[skillimage_num].move(packet->x - i, packet->y - i);
				skills[skillimage_num].show();
				skills[skillimage_num].m_otype = O_SKILL_1;
				skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
				++skillimage_num;
			}
			for (int i = 0; i < 5; ++i)
			{
				skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
				skills[skillimage_num].move(packet->x - i, packet->y + i);
				skills[skillimage_num].show();
				skills[skillimage_num].m_otype = O_SKILL_1;
				skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
				++skillimage_num;
			}
			for (int i = 0; i < 5; ++i)
			{
				skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
				skills[skillimage_num].move(packet->x + i, packet->y + i);
				skills[skillimage_num].show();
				skills[skillimage_num].m_otype = O_SKILL_1;
				skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
				++skillimage_num;
			}
			for (int i = 0; i < 5; ++i)
			{
				skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
				skills[skillimage_num].move(packet->x + i, packet->y - i);
				skills[skillimage_num].show();
				skills[skillimage_num].m_otype = O_SKILL_1;
				skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
				++skillimage_num;
			}
		}
		else if (packet->pattern == 2)
		{
			for (int i = 1; i <= 11; ++i)
			{
				for(int j = 1;j<5;++j)
				{
					skills[skillimage_num] = OBJECT{ *boss_waterfall, 0, 0, 64, 64 };
					skills[skillimage_num].move(packet->x - i+6, packet->y - 5 + 2*j);
					skills[skillimage_num].show();
					skills[skillimage_num].m_otype = O_SKILL_1;
					skills[skillimage_num].m_skill_time_out = high_resolution_clock::now() + 1s;
					++skillimage_num;
				}
			}
		}
	}
	break;
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);

	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = g_socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}

	if (recv_result == sf::Socket::Disconnected)
	{
		wcout << L"서버 접속 종료.";
		g_window->close();
	}

	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			//if (((tile_x + tile_y) % 2) == 0) {
			if (((tile_x / 3 + tile_y / 3) % 2) == 0) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}
	avatar->draw();
	//	for (auto &pl : players) pl.draw();
	for (auto& npc : npcs) npc.second.draw();
	for (auto& skillimages : skills) skillimages.second.draw();
	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar->m_x, avatar->m_y);
	
	text.setString(buf);
	g_window->draw(text);

	sf::Text levtext;
	levtext.setFont(g_font);
	char levbuf[20] {};
	char level[10]{};
	strcat_s(levbuf, "LV:");
	_itoa_s(avatar->m_level, level, 10);
	strcat_s(levbuf, level);
	levtext.setString(levbuf);
	levtext.setPosition(480, 0);
	g_window->draw(levtext);
	
	sf::Text hptext;
	hptext.setFont(g_font);
	char hpbuf[20]{};
	char hp[10]{};
	strcat_s(hpbuf, "HP:");
	_itoa_s(avatar->m_hp, hp, 10);
	strcat_s(hpbuf, hp);
	hptext.setString(hpbuf);
	hptext.setPosition(630, 0);
	g_window->draw(hptext);


	sf::Text exptext;
	exptext.setFont(g_font);
	char expbuf[20]{}; 
	char exp[10]{};
	strcat_s(expbuf, "EXP:");
	_itoa_s(avatar->m_exp, exp, 10);
	strcat_s(expbuf, exp);
	exptext.setString(expbuf);
	exptext.setPosition(800, 0);
	g_window->draw(exptext);

	if (b_chat)
	{
		sf::Text nowchatting;
		nowchatting.setFont(g_font);
		char nowchat[20] = "now chatting..";
		nowchatting.setString(nowchat);
		nowchatting.setPosition(avatar->m_x + 200, avatar->m_y + 370);
		g_window->draw(nowchatting);
	}
}

void send_packet(void* packet)
{
	char* p = reinterpret_cast<char*>(packet);
	size_t sent;
	g_socket.send(p, p[0], sent);
}

void send_move_packet(unsigned char dir)
{
	cs_packet_move m_packet;
	m_packet.type = C2S_MOVE;
	m_packet.size = sizeof(m_packet);
	m_packet.direction = dir;
	send_packet(&m_packet);
}

void send_attack_packet()
{
	cs_packet_attack packet;
	packet.type = C2S_ATTACK;
	packet.size = sizeof(cs_packet_attack);
	send_packet(&packet);
}

void send_chat_packet(wchar_t chat[])
{
	cs_packet_chat packet;
	packet.type = C2S_CHAT;
	packet.size = sizeof(cs_packet_chat)*2;
	wcscpy_s(packet.message, chat);
	send_packet(&packet);
}



int main()
{
	wcout.imbue(locale("korean"));
	setlocale(LC_ALL, "Korean");


	sf::Socket::Status status = g_socket.connect("127.0.0.1", SERVER_PORT);

	g_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}
	else
	{

	}

	client_initialize();


	WCHAR	cl_name[MAX_ID_LEN]{};

	std::cout << "input player name : ";
	std::wcin >> cl_name;

	cs_packet_login l_packet;
	ZeroMemory(l_packet.name, MAX_ID_LEN);
	l_packet.size = sizeof(l_packet);
	l_packet.type = C2S_LOGIN;
	int t_id = GetCurrentProcessId();
	//cout << l_packet.name << endl;
	wprintf_s(l_packet.name, "%s");
	wcscpy_s(l_packet.name, cl_name);
	wcscpy_s(avatar->name, l_packet.name);
	//strcpy_s(avatar.name, l_packet.name);
	avatar->set_name(l_packet.name);
	send_packet(&l_packet);

	//cs_packet_login l_packet;
	//l_packet.size = sizeof(l_packet);
	//l_packet.type = C2S_LOGIN;
	//int t_id = GetCurrentProcessId();
	//sprintf_s(l_packet.name, "P%03d", t_id % 1000);
	//strcpy_s(avatar.name, l_packet.name);
	//avatar.set_name(l_packet.name);
	//send_packet(&l_packet);

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH , WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	sf::View view = g_window->getView();
	view.zoom(2.f);
	view.move(SCREEN_WIDTH * TILE_WIDTH / 4, SCREEN_HEIGHT* TILE_WIDTH / 4);
	g_window->setView(view);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)	
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int p_type = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					send_move_packet(D_LEFT);
					break;
				case sf::Keyboard::Right:
					send_move_packet(D_RIGHT);
					break;
				case sf::Keyboard::Up:
					send_move_packet(D_UP);
					break;
				case sf::Keyboard::Down:
					send_move_packet(D_DOWN);
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::Enter:
					b_chat = !b_chat;
					static bool beforemessage;
					beforemessage = message;
					message =!message;
					if (!message && beforemessage)
					{
						send_chat_packet(szmessage);
						memset(szmessage, 0, 80);
					}

				break;
				//스페이스 넣어야 함
				case sf::Keyboard::LShift:{ capslock =! capslock;}break;
				case sf::Keyboard::Space: {	wcscat_s(szmessage, L" "); }break;
				case sf::Keyboard::A: {
					if (message)
					{
						if (!capslock)
							wcscat_s(szmessage, L"A");
						else if (capslock)
							wcscat_s(szmessage, L"a");
					}
					else
						send_attack_packet();
				}
									  break;
				case sf::Keyboard::B: {if (!capslock)	wcscat_s(szmessage, L"B"); else if (capslock) wcscat_s(szmessage, L"b"); }break;
				case sf::Keyboard::C: {if (!capslock)	wcscat_s(szmessage, L"C"); else if (capslock) wcscat_s(szmessage, L"c"); }break;
				case sf::Keyboard::D: {if (!capslock)	wcscat_s(szmessage, L"D"); else if (capslock) wcscat_s(szmessage, L"d"); }break;
				case sf::Keyboard::E: {if (!capslock)	wcscat_s(szmessage, L"E"); else if (capslock) wcscat_s(szmessage, L"e"); }break;
				case sf::Keyboard::F: {if (!capslock)	wcscat_s(szmessage, L"F"); else if (capslock) wcscat_s(szmessage, L"f"); }break;
				case sf::Keyboard::G: {if (!capslock)	wcscat_s(szmessage, L"G"); else if (capslock) wcscat_s(szmessage, L"g"); }break;
				case sf::Keyboard::H: {if (!capslock)	wcscat_s(szmessage, L"H"); else if (capslock) wcscat_s(szmessage, L"h"); }break;
				case sf::Keyboard::I: {if (!capslock)	wcscat_s(szmessage, L"I"); else if (capslock) wcscat_s(szmessage, L"i"); }break;
				case sf::Keyboard::J: {if (!capslock)	wcscat_s(szmessage, L"J"); else if (capslock) wcscat_s(szmessage, L"j"); }break;
				case sf::Keyboard::K: {if (!capslock)	wcscat_s(szmessage, L"K"); else if (capslock) wcscat_s(szmessage, L"k"); }break;
				case sf::Keyboard::L: {if (!capslock)	wcscat_s(szmessage, L"L"); else if (capslock) wcscat_s(szmessage, L"l"); }break;
				case sf::Keyboard::M: {if (!capslock)	wcscat_s(szmessage, L"M"); else if (capslock) wcscat_s(szmessage, L"m"); }break;
				case sf::Keyboard::N: {if (!capslock)	wcscat_s(szmessage, L"N"); else if (capslock) wcscat_s(szmessage, L"n"); }break;
				case sf::Keyboard::O: {if (!capslock)	wcscat_s(szmessage, L"O"); else if (capslock) wcscat_s(szmessage, L"o"); }break;
				case sf::Keyboard::P: {if (!capslock)	wcscat_s(szmessage, L"P"); else if (capslock) wcscat_s(szmessage, L"p"); }break;
				case sf::Keyboard::Q: {if (!capslock)	wcscat_s(szmessage, L"Q"); else if (capslock) wcscat_s(szmessage, L"q"); }break;
				case sf::Keyboard::R: {if (!capslock)	wcscat_s(szmessage, L"R"); else if (capslock) wcscat_s(szmessage, L"r"); }break;
				case sf::Keyboard::S: {if (!capslock)	wcscat_s(szmessage, L"S"); else if (capslock) wcscat_s(szmessage, L"s"); }break;
				case sf::Keyboard::T: {if (!capslock)	wcscat_s(szmessage, L"T"); else if (capslock) wcscat_s(szmessage, L"t"); }break;
				case sf::Keyboard::U: {if (!capslock)	wcscat_s(szmessage, L"U"); else if (capslock) wcscat_s(szmessage, L"u"); }break;
				case sf::Keyboard::V: {if (!capslock)	wcscat_s(szmessage, L"V"); else if (capslock) wcscat_s(szmessage, L"v"); }break;
				case sf::Keyboard::W: {if (!capslock)	wcscat_s(szmessage, L"W"); else if (capslock) wcscat_s(szmessage, L"w"); }break;
				case sf::Keyboard::X: {if (!capslock)	wcscat_s(szmessage, L"X"); else if (capslock) wcscat_s(szmessage, L"x"); }break;
				case sf::Keyboard::Y: {if (!capslock)	wcscat_s(szmessage, L"Y"); else if (capslock) wcscat_s(szmessage, L"y"); }break;
				case sf::Keyboard::Z: {if (!capslock)	wcscat_s(szmessage, L"Z"); else if (capslock) wcscat_s(szmessage, L"z"); }break;
				}
			}
		}

		window.clear();
		client_main();
		window.display();
	}

	cs_packet_logout packet;
	packet.size = sizeof(cs_packet_logout);
	packet.type = C2S_LOGOUT;
	send_packet(&packet);

	client_finish();

	return 0;
}