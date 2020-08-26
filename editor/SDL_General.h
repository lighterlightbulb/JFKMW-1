#pragma once
/*
	sdl_general.h

	has functions that are helpful.
*/

Uint8 zsnes_font[0x3C0];

SDL_Window* win; //The window
SDL_Renderer* ren; //The renderer
int p_w = 0;
int p_h = 0;

int old_p_w = 0;
int old_p_h = 0;

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void load_zsnes_font()
{
	cout << cyan << "[ZUI] Loading zfont.txt" << endl;
	uint_fast16_t current_letter = 0;
	uint_fast8_t current_offset = 0;
	ifstream cFile(path + "zfont.txt");
	if (cFile.is_open())
	{
		string line;
		while (getline(cFile, line)) {
			if (line[0] == ';') { current_letter += 1; current_offset = 0; continue; }
			if (line.empty()) { continue; }


			for (uint_fast8_t i = 0; i < 8; i++)
			{
				zsnes_font[(current_letter * 5) + current_offset] ^= (uint_fast8_t(line.at(i) == '1') << i);
			}

			current_offset += 1;
		}

		cout << cyan << "[ZUI] Loaded 0x" << hex << int(current_letter) << dec << " letters." << endl;
	}
	cFile.close();
}
uint_fast8_t char_to_zsnes_font_letter(char l) //use to convert strings
{
	uint_fast8_t new_l = uint_fast8_t(l);
	if (new_l == 0x2E) { return 0x27; }
	if (new_l == 0x3A) { return 0x2D; }
	if (new_l == 0x5F) { return 0x25; }
	if (new_l == 0x2D) { return 0x24; }
	if (new_l > 0x60) { return new_l - 0x57; }
	if (new_l > 0x40) { return new_l - 0x37; }
	if (new_l >= 0x30) { return new_l - 0x30; }
	return 0x8C;
}

//draws a string
void draw_string(string str, int_fast16_t x, int_fast16_t y)
{
	int_fast16_t orig_x = x;
	for (int i = 0; i < str.size(); i++) {
		uint_fast16_t arr_l = char_to_zsnes_font_letter(str.at(i));
		if (str.at(i) == '\n')
		{
			y += 6;
			x = orig_x;
		}
		else
		{
			for (uint_fast8_t x_l = 0; x_l < 8; x_l++) {
				for (uint_fast8_t y_l = 0; y_l < 5; y_l++) {
					if ((zsnes_font[(arr_l * 5) + y_l] >> x_l) & 1)
					{
						uint_fast8_t formula = 255 - y_l * 16;

						SDL_SetRenderDrawColor(ren, formula, formula, formula, 255);
						SDL_RenderDrawPoint(ren, x + x_l, y + y_l);
					}
				}
			}
			x += 6;
		}
	}
}

void screen(int width, int height, const std::string& text = "test")
{
	if (win) { SDL_DestroyWindow(win); }

	int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	win = SDL_CreateWindow(text.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	p_w = width;
	p_h = height;

	if (win == NULL) { std::cout << red << "[SDL] window error : " << SDL_GetError() << std::endl; SDL_Quit(); std::exit(1); }

	flags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;

	ren = SDL_CreateRenderer(win, -1, flags);
	if (ren == NULL) { std::cout << red << "[SDL] renderer error : " << SDL_GetError() << std::endl; SDL_Quit(); std::exit(1); }


	std::cout << blue << "[SDL] screen resolution : " << width << "x" << height << white << std::endl;
	std::cout << blue << "[SDL] current video driver : " << SDL_GetCurrentVideoDriver() << white << std::endl;
}


/*

	check if the game is still running.

*/
SDL_Event event = { 0 };
bool running()
{
	mouse_w_up = false; mouse_w_down = false;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) return false;
		if (event.type == SDL_MOUSEWHEEL)
		{
			if (event.wheel.y > 0) // scroll up
			{
				// Put code for handling "scroll up" here!
				mouse_w_up = true;
			}
			else if (event.wheel.y < 0) // scroll down
			{
				// Put code for handling "scroll down" here!
				mouse_w_down = true;
			}
		}
	}

	return true;
}
