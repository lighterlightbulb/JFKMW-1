#pragma once

/*
	the zsnes like UI for jfk mario world
*/

#define snow_size 100 //Amount of snow particles.
void draw_pixel_to_surface(uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, SDL_Surface* screen_s)
{
	if (x1 > -1 && x1 < screen_s->w && y1 > -1 && y1 < screen_s->h)
	{
		Uint32* p_screen = (Uint32*)screen_s->pixels;
		p_screen += y1 * screen_s->w + x1;
		*p_screen = SDL_MapRGBA(screen_s->format, r, g, b, 255);
	}
}

Uint8 zsnes_font[0x3C0];

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
				zsnes_font[(current_letter * 5) + current_offset ] ^= (uint_fast8_t(line.at(i) == '1') << i);
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
void draw_string(bool dark, string str, int_fast16_t x, int_fast16_t y, SDL_Surface* surface)
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
						if ((x + x_l) < surface->w && (y + y_l) < surface->h && (x + x_l) >= 0 && (y + y_l) >= 0)
						{
							uint_fast8_t formula = dark ? (16 + y_l * 16) : (255 - y_l * 16);
							draw_pixel_to_surface(x + x_l, y + y_l, formula, formula, formula, surface);
						}
					}
				}
			}
			x += 6;
		}
	}
}

class ZSNES_button
{
public:
	string name;
	int_fast16_t x_s;
	int_fast16_t y_s;
	int_fast16_t x_e;
	int_fast16_t y_e;
	ZSNES_button(string n_n, int_fast16_t n_x_s, int_fast16_t n_y_s, int_fast16_t n_x_e, int_fast16_t n_y_e)
	{
		name = n_n;
		x_s = n_x_s;
		y_s = n_y_s;
		x_e = n_x_e;
		y_e = n_y_e;
	}
};

class ZSNES_ui
{
	SDL_Surface* surface; //for drawing
public:
	SDL_Texture* texture; //final texture for output
	vector<ZSNES_button> button;
	string button_pressed;
	string hint;
	string message;

	//variables
	float snow_x[snow_size];
	float snow_y[snow_size];
	float snow_s;
	float snow_x_s[snow_size];
	double changelog_x = 256;
	double level_x = -256;
	vector<string> levels_found;

	//initializer. done always
	ZSNES_ui()
	{
		button_pressed = "none";
		clean_surface();
		for (int i = 0; i < snow_size; i++)
		{
			snow_x[i] = float(rand() % 256);
			snow_y[i] = float(rand() % 256);
			snow_x_s[i] = float(1 + (rand() % 3)) / 6.f;
		}
		snow_s = 0.f;
#if defined(USE_FILESYSTEM)

		string patht = path + "Levels";
		for (const auto& entry : fs::directory_iterator(patht))
		{
			string st = entry.path().string();
			st = st.substr(st.length() - 2, st.length());
			if (st.substr(0, 1) == "\\")
			{
				st = "0" + st.substr(1, 2);
			}
			levels_found.push_back(st);
		}
#else
		levels_found.push_back("search not supported");
#endif
	}

	//add button
	void add_button(string name, int_fast16_t x_s, int_fast16_t y_s, int_fast16_t x_e, int_fast16_t y_e)
	{
		button.push_back(ZSNES_button(name, x_s, y_s, x_e, y_e));
	}

	//initializes the ZSNES surface
	void clean_surface()
	{
		Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00; amask = 0x000000ff;
#else
		rmask = 0x000000ff; gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;
#endif

		if (surface) {
			SDL_FreeSurface(surface);
		}
		surface = SDL_CreateRGBSurface(0, 256, 224, 32,
			rmask, gmask, bmask, amask);

	}

	//draws rectangle on the surface
	void draw_rectangle(int_fast16_t x_s, int_fast16_t x_e, int_fast16_t y_s, int_fast16_t y_e, uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint_fast8_t a = 255)
	{
		SDL_Rect rect;
		rect.x = x_s; rect.w = x_e - x_s;
		rect.y = y_s; rect.h = y_e - y_s;
		SDL_FillRect(surface, &rect, 0x00000000 + r + (g << 8) + (b << 16) + (a << 24));
	}

	//draws a button
	void draw_button(bool pressed, int_fast16_t x_s, int_fast16_t y_s, int_fast16_t x_e, int_fast16_t y_e)
	{
		draw_rectangle(x_s, x_e, y_s, y_e, 88, 84, 80); //bg.

		if (pressed)
		{
			draw_rectangle(x_s, x_s + 1, y_s, y_e - 1, 72, 68, 64); //left side 104, 100, 96);
			draw_rectangle(x_s + 1, x_e, y_s, y_s + 1, 56, 52, 48); //top side 120, 116, 112);
			draw_rectangle(x_e - 1, x_e, y_s + 1, y_e, 104, 100, 96); //right side 72, 68, 64);
			draw_rectangle(x_s, x_e - 1, y_e - 1, y_e, 120, 116, 112); //bottom side 56, 52, 48);
		}
		else
		{
			draw_rectangle(x_s, x_s + 1, y_s, y_e - 1, 104, 100, 96); //left side
			draw_rectangle(x_s + 1, x_e, y_s, y_s + 1, 120, 116, 112); //top side
			draw_rectangle(x_e - 1, x_e, y_s + 1, y_e, 72, 68, 64); //right side
			draw_rectangle(x_s, x_e - 1, y_e - 1, y_e, 56, 52, 48); //bottom side
		}
	}

	//process ZSNES ui
	void process()
	{
		SDL_UnlockSurface(surface);
		SDL_Rect rect;
		rect.x = 0; rect.w = 256;
		rect.y = 0; rect.h = 224;

		draw_rectangle(0, 256, 0, 224, 64, 44, 128, 128);
		draw_rectangle(5, 235, 6, 20, 48, 32, 96, 128);

		for (int i = 0; i < snow_size; i++)
		{
			
			snow_x[i] += ((snow_s/2) + snow_x_s[i] + (float(rand() % 10) - 5.f) / 10.f) * float(60.0 / monitor_hz);
			snow_y[i] += ((snow_s) + (float(rand() % 2)/2.f)) * float(60.0 / monitor_hz);

			draw_pixel_to_surface(uint_fast8_t(snow_x[i]), uint_fast8_t(snow_y[i]), 176, 180, 200, surface);
		}

		snow_s = min(1.f, snow_s + 0.001f);
		draw_rectangle(0, 230, 0, 15, 0, 44, 136);
		draw_rectangle(0, 230, 1, 14, 0, 48, 152);
		draw_rectangle(0, 230, 2, 13, 0, 52, 168);
		draw_rectangle(0, 230, 3, 12, 0, 56, 184);

		draw_rectangle(0, 230, 4, 11, 0, 60, 200);
		draw_rectangle(0, 230, 5, 10, 0, 64, 210);
		draw_rectangle(0, 230, 6, 9, 0, 68, 232);
		draw_rectangle(0, 230, 7, 8, 0, 72, 248);

		button_pressed = "none";

		for (int i = 0; i < button.size(); i++) {
			ZSNES_button& b = button[i];
			bool check = mouse_down && (mouse_x > b.x_s && mouse_x < b.x_e && mouse_y > b.y_s && mouse_y < b.y_e);
			if (check == true) {
				button_pressed = b.name;
			}
			draw_button(check, b.x_s, b.y_s, b.x_e, b.y_e);

			draw_string(true, b.name, b.x_s + 4, b.y_s + 4, surface);
			draw_string(false, b.name, b.x_s + 3, b.y_s + 3, surface);
		}

		changelog_x = changelog_x + (0 - changelog_x) / 10.0;
		level_x = level_x + (5 - level_x) / 10.0;



		//Numbers (HOw to otpimize please help)
		if (getKey(0x30)) { hint = hint + "0"; }
		if (getKey(0x31)) { hint = hint + "1"; }
		if (getKey(0x32)) { hint = hint + "2"; }
		if (getKey(0x33)) { hint = hint + "3"; }
		if (getKey(0x34)) { hint = hint + "4"; }
		if (getKey(0x35)) { hint = hint + "5"; }
		if (getKey(0x36)) { hint = hint + "6"; }
		if (getKey(0x37)) { hint = hint + "7"; }
		if (getKey(0x38)) { hint = hint + "8"; }
		if (getKey(0x39)) { hint = hint + "9"; }
		//Letters
		if (getKey(0x41)) { hint = hint + "A"; }
		if (getKey(0x42)) { hint = hint + "B"; }
		if (getKey(0x43)) { hint = hint + "C"; }
		if (getKey(0x44)) { hint = hint + "D"; }
		if (getKey(0x45)) { hint = hint + "E"; }
		if (getKey(0x46)) { hint = hint + "F"; }
		//Period
		if (getKey(0xBE)) { hint = hint + "."; }
		//Delete
		if (getKey(0x08) && hint.size() > 0) { hint.pop_back(); }

		hint = hint.substr(0, 15);

		draw_string(false, "JFKMW - " + message, 5, 224 - 16, surface);
		draw_string(false, "Option: " + hint + ((global_frame_counter % 64) > 32 ? "_" : ""), 5, 224 - 22, surface);
		draw_string(false, "Type a option then press a button.", 5, 224 - 10, surface);

		string t = "Changelog V" + GAME_VERSION + ": \n\n" + CHANGELOG;
		t += "\n\n\n--------------------\n\nPlease remember to\nread the documentation!";
		if (dirExists((path + "Overworld").c_str()))
		{
			t += "\n\n--------------------\n\nOverworld detected,\nPress O to go to it.";
		}
		draw_string(false, t, int_fast16_t(changelog_x) + 96, 20, surface);


		draw_string(false, "Levels Found:", int_fast16_t(level_x), 20, surface);
		for (int i = 0; i < levels_found.size(); i++)
		{
			draw_string(false, levels_found[i], int_fast16_t(level_x) + (i / 27) * 15, 30 + (i % 27) * 6, surface);
		}
	}

	//finish processing, copy to texture.
	void finish_processing(SDL_Renderer* r)
	{
		SDL_LockSurface(surface);
		if (texture)
		{
			SDL_DestroyTexture(texture);
		}
		texture = SDL_CreateTextureFromSurface(r, surface);
	}
};

ZSNES_ui zsnes_ui;
