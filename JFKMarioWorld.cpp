/*

	JFK Mario World cpp file. This is the main file of jfk mario world and the start of the game.

	Linux users do not support some features.

	动态网自由门 天安門 天安门 法輪功 李洪志 Free Tibet 六四天安門事件 The Tiananmen Square protests of 1989 天安門大屠殺 The Tiananmen Square Massacre 反右派鬥爭 The Anti-Rightist Struggle 
	大躍進政策 The Great Leap Forward 文化大革命 The Great Proletarian Cultural Revolution 人權 Human Rights 民運 Democratization 自由 Freedom 獨立 Independence 多黨制 Multi-party system 
	台灣 臺灣 Taiwan Formosa 中華民國 Republic of China 西藏 土伯特 唐古特 Tibet 達賴喇嘛 Dalai Lama 法輪功 Falun Dafa 新疆維吾爾自治區 The Xinjiang Uyghur Autonomous Region 諾貝爾和平獎 
	Nobel Peace Prize 劉暁波 Liu Xiaobo 民主 言論 思想 反共 反革命 抗議 運動 騷亂 暴亂 騷擾 擾亂 抗暴 平反 維權 示威游行 李洪志 法輪大法 大法弟子 強制斷種 強制堕胎 民族淨化 人體實驗 肅清 胡耀邦 
	趙紫陽 魏京生 王丹 還政於民 和平演變 激流中國 北京之春 大紀元時報 九評論共産黨 獨裁 專制 壓制 統一 監視 鎮壓 迫害 侵略 掠奪 破壞 拷問 屠殺 活摘器官 誘拐 買賣人口 遊進 走私 毒品 賣淫 春畫 賭博 
	六合彩 天安門 天安门 法輪功 李洪志 Winnie the Pooh 劉曉波动态网自由门

*/

#if defined(_WIN32)
#include <Windows.h>
#include "psapi.h"

#define USE_FILESYSTEM

#elif defined(__linux__)

#define NDEBUG

#include <unistd.h>
#include <list>
void Sleep(int time) {
	usleep(time * 1000);
}
#endif

#if defined(DARWIN)

#define DISABLE_NETWORK

#endif

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <ratio>
#include <chrono>
#include <ctype.h>
#include <cstring>
#include <cassert>
#if defined(USE_FILESYSTEM)
#include <filesystem>
#endif
#include "snes_spc/spc.h"

using namespace std;
#if defined(USE_FILESYSTEM)
namespace fs = std::filesystem;
#endif

#include <SDL.h>
#if defined(_WIN32)
#include <SDL_syswm.h>
#endif
#include <SDL_image.h>
#include <SDL_mixer.h>

#if not defined(DISABLE_NETWORK)
#include <SFML/Network.hpp>
#endif



#if defined(__linux__)
#if not defined(DISABLE_NETWORK)
#define uint_fast8_t sf::Uint8
#define uint_fast16_t sf::Uint32
#define uint_fast32_t sf::Uint32
#define uint_fast64_t sf::Uint64
#define int_fast8_t sf::Int8
#define int_fast16_t sf::Int32
#define int_fast32_t sf::Int32
#define int_fast64_t sf::Int64
#endif

istream& getline(istream& stream, string& str)
{
	char ch;
	str.clear();
	while (stream.get(ch)) {
		if (ch == '\r') {
			stream.get(ch);
			if (ch == '\n') {
				break;
			}
		}
		str.push_back(ch);
	}
	return stream;
}
#endif


#include "cout.h"

#include "Global.h"
#include "ASM.h"
#include "HDMA.h"
#include "config.h"

#include "Sprite_shared.h"

#include "Particle_System.h"

#include "LuaImplementation.h"

#include "Sound.h"


#include "Global.h"
#include "SDL_General.h"
#include "sprite.h"

#include "map16block.h"
#include "exanimation.h"
#include "level_and_load.h"

#include "input.h"

#include "Sprite_system.h"
#include "mario.h"

#include "messageboxes.h"

#include "chat.h"

#include "debugging.h"

#include "overworld.h"
#include "transitions.h"

#include "game.h"


#if not defined(DISABLE_NETWORK)
#include "Netplay.h"
#endif

#include "zsnes_ui.h"
#include "renderer.h"


#if not defined(DISABLE_NETWORK)
#include "server.h"
#endif
#include "main.h"

int main(int argc, char* argv[])
{
	load_configuration();

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scale_quality.c_str());

#if not defined(DISABLE_NETWORK)
	bool hosting = false;
	if (argc > 1)
	{
		if (strcmp(argv[1], "-h") == 0) {
			hosting = true;
		}
		if (strcmp(argv[1], "-c") == 0) {
			hosting = false;
		}
		if (strcmp(argv[1], "-l") == 0) {
			testing_level = argv[2];
		}
	}

#if defined(_WIN32)
	string t = "JFKMW Console - " + string(hosting ? "Server" : "Client") + " " + GAME_VERSION;
	wstring stemp = wstring(t.begin(), t.end());
	LPCWSTR sw = stemp.c_str();
	SetConsoleTitle(sw);
#endif

	if (hosting) {
		cout << yellow << "[JFKMW] Welcome to JFK mario world " + GAME_VERSION + "! Hosting a server." << endl;
		if (argc > 2) {
			server_code(string(argv[2]));
		}
		else {
			server_code();
		}
	}
	else {
		cout << yellow << "[JFKMW] Welcome to JFK mario world " + GAME_VERSION + "!, Press R to reload the last level you played, Press Q if you want to play singleplayer, or W to connect to a server. Make sure you have typed in a option first in the UI!" << white << endl;
		player_code();
	}
#else
	player_code();
#endif
	if (testing_level == "")
	{
		cout << yellow << "[JFKMW] Quitting JFK mario world. Thanks for testing!" << white << endl;
		Sleep(1000);
	}
	end_game();

	return 0;
}