
#include <Windows.h>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
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
#include <conio.h>
#include <locale>
#include <codecvt>

using namespace std;

double window_scale_x = 1;
double window_scale_y = 1;

bool mouse_w_up = false;
bool mouse_w_down = false;

#include <SDL.h>
#include <SDL_image.h>
#undef main

#ifdef NDEBUG
string path = "";
#else
string path = "E:/JFKMarioWorld/Debug/";
#endif

SDL_GameController* gGameController;
int controller = 0;
bool keyboard_or_controller = false;
double fps = 0;
int frm_counter = 0;
string current_file = "";
string jfkmw_executable = "";
string clevel = "";

#include "ConsoleStuff.h"
#include "SDL_General.h"
#include "ImageLoading.h"
#include "LevelData.h"
#include "LevelManager.h"
#include "Rendering.h"
#include "Controls.h"


int main()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << red << "Error initializing SDL!" << white << endl;
		return 1;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	cout << cyan << "[SDL] SDL Initialized." << white << endl;
	screen(768, 544, "Solar Energy V1.3.0");

	load_zsnes_font();
	InitializeMap16();
	InitializeConsole();
	InitializeLevel();
	init_controls();
	InitInstalledSprites();

	//After a level was loaded
	while (running())
	{
		SDL_GetWindowSize(win, &p_w, &p_h);
		if (p_w != old_p_w || p_h != old_p_h)
		{
			old_p_w = p_w;
			old_p_h = p_h;
		}

		window_scale_x = double(double(p_w) / 768.0);
		window_scale_y = double(double(p_h) / 544.0);



		if (SDL_GetWindowFlags(win) & SDL_WINDOW_INPUT_FOCUS)
		{
			SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
			SDL_RenderClear(ren);
			
			frm_counter++;
			chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
			handle_controls();
			draw_to_screen();

			chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
			chrono::duration<double> total_time_ticks = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
			fps = double(1.0 / (total_time_ticks.count() / 1.0));

			SDL_RenderPresent(ren);
		}
	}
	return 1;
}