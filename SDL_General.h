#pragma once

//ColorRGB

////////////////////////////////////////////////////////////////////////////////
//BASIC SCREEN FUNCTIONS////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
HWND sdl_window;

//Namespace variables/Defines
static HMENU hFile;
static HMENU hInput;
static HMENU hSound;
static HMENU hVideo;
static HMENU hHelp;
static HMENU hMenuBar;


//Function which retrieves the address/Handle of an SDL window
//Also retrieves the specific subsystem used by SDL to create that window which is platform specific (Windows, MAC OS x, IOS, etc...)
void getSDLWinHandle()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(win, &wmInfo);

	sdl_window = wmInfo.info.win.window;
}

void ActivateMenu()
{
	hMenuBar = CreateMenu();
	hFile = CreateMenu();
	hInput = CreateMenu();
	hSound = CreateMenu();
	hVideo = CreateMenu();
	hHelp = CreateMenu();


	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"File");
	AppendMenu(hFile, MF_STRING, 1, L"Reload Configuration");
	AppendMenu(hFile, MF_STRING, 2, L"Exit");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hInput, L"Input");
	AppendMenu(hInput, MF_STRING, 3, L"Use config for now LOL");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSound, L"Sound");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hVideo, L"Video");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"Help");
	AppendMenu(hHelp, MF_STRING, 4, L"About");

	SetMenu(sdl_window, hMenuBar);
}
#endif

//The screen function: sets up the window for 32-bit color graphics.
//Creates a graphical screen of width*height pixels in 32-bit color.
//Set fullscreen to false for a window, or to true for fullscreen output
//text is the caption or title of the window
//also inits SDL therefore you MUST call screen before any other InstantCG or SDL functions
void screen(int width, int height)
{
	cout << cyan << "[SDL] Creating window" << endl;
	if (win) { SDL_DestroyWindow(win); }

	w = width;

#if defined(_WIN32)

	h = height + (fullscreen ? 0 : 20);
#else
	h = height;
#endif

	int flags = SDL_WINDOW_SHOWN;
	if (fullscreen)
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	else
	{
		flags |= SDL_WINDOW_RESIZABLE;
	}
	if (opengl)
	{
		flags |= SDL_WINDOW_OPENGL;
	}

	win = SDL_CreateWindow("JFK Mario World Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);


	if (win == NULL) { cout << cyan << "[SDL] Window error: " << SDL_GetError() << white << endl; SDL_Quit(); exit(1); }
	cout << cyan << "[SDL] Window created, initializing renderer..." << endl;

	flags = 0;
	if (v_sync)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
	}
	if (renderer_accelerated)
	{
		flags |= SDL_RENDERER_ACCELERATED;
	}
	else
	{
		flags |= SDL_RENDERER_SOFTWARE;
	}
	
	ren = SDL_CreateRenderer(win, rendering_device, flags);

	global_texture_s = SDL_CreateTexture(ren, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_TARGET, int_res_x, int_res_y);
	if (splitscreen)
	{
		target_texture_p1 = SDL_CreateTexture(ren, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_TARGET, int_res_x, int_res_y/2);
		target_texture_p2 = SDL_CreateTexture(ren, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_TARGET, int_res_x, int_res_y/2);
	}


	if (ren == NULL) { cout << cyan << "[SDL] Renderer error: " << SDL_GetError() << white << endl; SDL_Quit(); exit(1); }


	cout << cyan << "[SDL] Initialized window of " << dec << width << "x" << height << " resolution" << white << endl;
	cout << cyan << "[SDL] Current video card : " << SDL_GetCurrentVideoDriver() << white << endl;

	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	screen_s_l1 = *SDL_CreateRGBSurface(0, int_res_x + 16, int_res_y + 16, 32,
		rmask, gmask, bmask, amask);
#if defined(_WIN32)
	//Enable WinAPI Events Processing
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	if (!fullscreen)
	{
		getSDLWinHandle();
		ActivateMenu();
	}
#endif
}

void PrepareRendering()
{

	if (show_full_screen || !splitscreen)
	{
		uint_fast16_t c = RAM[0x3D00] + (RAM[0x3E00] << 8);
		SDL_SetRenderDrawColor(ren, (c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, (c >> 10) << 3, 255);
		SDL_SetRenderTarget(ren, global_texture_s);
		SDL_RenderClear(ren);
	}
	if (!fullscreen)
	{
		SDL_GetWindowSize(win, &resolution_x, &resolution_y);
		w = resolution_x;
		h = resolution_y;
	}

	if (!forced_scale)
	{
		if (integer_scaling)
		{
			scale = int(max(1, resolution_y / int_res_y));
		}
		else
		{
			scale = max(1.0, double(resolution_y) / double(int_res_y));
		}
	}
	sp_offset_x = (w / 2) - int(double(int_res_x / 2) * scale);
	sp_offset_y = (h / 2) - int(double(int_res_y / 2) * scale);

}

void end_game()
{
	SDL_DestroyRenderer(ren);
}



void drawRect(int_fast16_t x1, int_fast16_t y1, uint_fast8_t color, SDL_Surface * screen_s)
{
	if (x1 > -1 && x1 < screen_s->w && y1 > -1 && y1 < screen_s->h)
	{
		Uint32* p_screen = (Uint32*)screen_s->pixels;
		p_screen += y1 * screen_s->w + x1;
		*p_screen = palette_array[color]; //SDL_MapRGBA(screen_s->format, r, g, b, 255);
	}
}

void cls()
{
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);

	PrepareRendering();
}

void DrawMouse()
{
	if (gGameController)
	{
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 127);
		SDL_Rect rect = { sp_offset_x + int(double(mouse_x) * scale), sp_offset_y + int(double(mouse_y) * scale), int(scale), int(scale) }; SDL_RenderFillRect(ren, &rect);
	}
}

void redraw()
{
	SDL_SetRenderTarget(ren, NULL);

	SDL_Rect rect = { sp_offset_x, sp_offset_y, int(int_res_x * scale), int(int_res_y * scale) };
	if (show_full_screen || !splitscreen)
	{
		SDL_RenderCopy(ren, global_texture_s, NULL, &rect);
	}
	else
	{
		rect.h = int((int_res_y / 2) * scale);
		SDL_RenderCopy(ren, target_texture_p1, NULL, &rect);
		rect.y += int((int_res_y / 2) * scale);
		SDL_RenderCopy(ren, target_texture_p2, NULL, &rect);
	}
	DrawMouse();
	SDL_RenderPresent(ren);
}

bool done()
{
	if (!v_sync) {
		SDL_Delay(16);
	}

	if (!networking && !gGameController)
	{
		mouse_w_up = false;
		mouse_w_down = false;
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) return true;
		if (event.type == SDL_MOUSEWHEEL)
		{
			mouse_w_up = event.wheel.y > 0;
			mouse_w_down = event.wheel.y < 0;
		}
#if defined (_WIN32)
		if (event.type == SDL_SYSWMEVENT)
		{
			if (event.syswm.msg->msg.win.msg == WM_COMMAND)
			{
				switch (LOWORD(event.syswm.msg->msg.win.wParam))
				{
				case 1:
					load_configuration();
					break;
				case 2:
					return true;
					break;
				case 4:

					string str = "JFKMW has been a ongoing 3 year effort to create a fun Super Mario World experience for everyone, with lots of modability and options.\n\nThanks to all contributors, programmers, and level designers.\n\n\nJFK Mario World, Made by the JFKMW Team, originally programmed by john fortnite kennedy.\n\nVersion " + GAME_VERSION;
					std::wstring stemp = std::wstring(str.begin(), str.end());
					LPCWSTR sw = stemp.c_str();

					MessageBox(NULL, sw, L"About", MB_OK);
					break;
				}
			}
		}
#endif
	}
	return quit;
}

void read_from_palette(string file)
{
	//cout << red << "[PAL] Loading File " << file << white << endl;

	ifstream input(file, ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
	uint_fast8_t curr = 0;
	uint_fast8_t curr_p = 0;
	uint_fast8_t current_buffer[2];
	for (auto &v : buffer)
	{
		current_buffer[curr] = uint8_t(v);

		if (curr)
		{
			RAM[0x3D00 + curr_p] = current_buffer[0];
			RAM[0x3E00 + curr_p] = current_buffer[1];

			curr_p++;
		}

		curr = (curr + 1) & 1;	
	}
	input.close();

}


void decode_graphics_file(string file, int offset = 0)
{
	offset *= 4096;
	if (offset >= 0xC000)
	{
		ClearSpriteCache();
	}

	file = path + file;

	ifstream input(file, ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
	int current_byte = 0;
	for (auto& v : buffer)
	{
		RAM[VRAM_Location + (offset)+current_byte] = uint_fast8_t(v);
		current_byte++;
	}
	input.close();
	if (offset == 0xB000)
	{
		PreloadL3();
	}

	//cout << yellow << "[GFX] Loading File " << file << " at 0x" << int_to_hex(0x10000 + offset) << " " << current_byte << " bytes loaded" << white << endl;
}


/*
-------------------------------- Colour possibilities 4BPP -------------------------------- 
0000 = Colour 0 
0001 = Colour 1 
0010 = Colour 2 
0011 = Colour 3 
0100 = Colour 4 
0101 = Colour 5 
0110 = Colour 6 
0111 = Colour 7 
1000 = Colour 8 
1001 = Colour 9 
1010 = Colour A 
1011 = Colour B 
1100 = Colour C 
1101 = Colour D 
1110 = Colour E 
1111 = Colour F

The format is again very similar:

12345678 12345678 12345678 12345678

The one without bold, cursive and underlined is for the lowest bit for each 1x8 row of pixels, the one in bold for the second lowest bit, 
the one in cursive for the second highest bit and the underlined one for the highest bit. Except now you might think, oh. 
So this takes 4 bytes per 1x8 line. Well, that is correct. But do you also think, x0-x3 cover the first 1x8 line? 
If so, then you're slightly off. x0 and x1 are used for the lowest and second-lowest bits, indeed, but the second-highest 
and highest bits of the 8x8 tile are not at x2-x3 - instead, they are at x10-x11. For some reason, the lower bits of the entire 
8x8 tile are handled before the higher bits. Either way, how does this work? Well, assuming everything has colour 0 again, change x0 to $80, 
and the top-leftmost pixel will use colour 1. To $40, it will be the second pixel, etc. etc. We already covered that. If x0 and x1 both are $80, 
it will use colour 3. But here's the trick that makes 4bpp different from 2bpp - it has high bits. It means that if x0, x1 and x10 are $80, the
topleft-most pixel will use colour 7. Just look at the colour possibilities table for 4bpp, and it should make sense - x10 controls the 
second-highest bit, x1 the second-lowest and x0 the lowest. That together forms colour 7. If x0, x1, x10 and x11 are all $80, the top-leftmost 
pixel will use colour F. As you would expect, each 8x8 tile requires 32 bytes.
*/

void draw8x8_tile(int_fast16_t x, int_fast16_t y, uint_fast16_t tile, uint_fast8_t palette)
{
	palette = palette << 4;	tile = tile << 5;
	uint_fast8_t color1 = 0;
	uint_fast8_t i = 0;
	uint_fast8_t index = 0;

	uint_fast8_t graphics_array[32];
	memcpy(graphics_array, &VRAM[tile], 32 * sizeof(uint_fast8_t));

	for (index = 0; index < 8; index++)
	{
		uint_fast8_t ind = index << 1;
		for (i = 0; i < 8; i++)
		{
			uint_fast16_t y_p = y + index;
			uint_fast16_t x_p = 7 - i + x;
			color1 =
				((graphics_array[0 + ind] >> i) & 1) + 
				(((graphics_array[1 + ind] >> i) & 1) << 1) + 
				(((graphics_array[16 + ind] >> i) & 1) << 2) + 
				(((graphics_array[17 + ind] >> i) & 1) << 3)
				;

			if (color1 != 0)
			{
				Uint32* p_screen = (Uint32*)(&screen_s_l1)->pixels + ((y_p * (int_res_x + 16)) + x_p);
				*p_screen = palette_array[color1 + palette];
			}
		}
	}
}

void draw8x8_tile_f(int_fast16_t x, int_fast16_t y, uint_fast16_t tile, uint_fast8_t palette, bool flipx, bool flipy)
{
	palette = palette << 4;	tile = tile << 5;
	uint_fast8_t color1 = 0;
	uint_fast8_t i = 0;
	uint_fast8_t index = 0;

	uint_fast8_t graphics_array[32];
	memcpy(graphics_array, &VRAM[tile], 32 * sizeof(uint_fast8_t));

	for (index = 0; index < 8; index++)
	{
		uint_fast8_t ind = index << 1;
		for (i = 0; i < 8; i++)
		{
			uint_fast16_t y_p = y + (flipy ? (7 - index) : index);
			uint_fast16_t x_p = x + (flipx ? (i) : (7 - i));
			color1 =
				((graphics_array[0 + ind] >> i) & 1) +
				(((graphics_array[1 + ind] >> i) & 1) << 1) +
				(((graphics_array[16 + ind] >> i) & 1) << 2) +
				(((graphics_array[17 + ind] >> i) & 1) << 3)
				;

			if (color1 != 0)
			{
				Uint32* p_screen = (Uint32*)(&screen_s_l1)->pixels + ((y_p * (int_res_x + 16)) + x_p);
				*p_screen = palette_array[color1 + palette];
			}
		}
	}
}



void draw8x8_tile_2bpp(int_fast16_t x, int_fast16_t y, uint_fast16_t tile, uint_fast16_t palette_offs)
{
	SDL_Rect DestR;
	SDL_Rect SrcR;
	DestR.x = x + (int_res_x - 256) / 2;
	DestR.y = y + (show_full_screen ? (int_res_y - 224) / 2 : 0);

	DestR.w = 8;
	DestR.h = 8;

	SrcR.x = (tile & 0xF) << 3;
	SrcR.y = ((tile >> 4) << 3);
	SrcR.w = 8; SrcR.h = 8;
	SDL_RenderCopy(ren, cached_l3_tiles[palette_offs], &SrcR, &DestR);
}



//drawtilecustom, if anyone knew how to fucking optimize this, i'd fucking love you for life.

void draw_tile_custom(int_fast16_t x, int_fast16_t y, uint_fast8_t size, double angle, uint_fast16_t tile, uint_fast8_t palette, SDL_RendererFlip flip)
{
	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
	//SDL_Texture* drawnTex;
	uint_fast32_t flags = tile + (size << 16) + (palette << 24);
	uint_fast8_t size_x = (size % 16) + 1;
	uint_fast8_t size_y = (size / 16) + 1;
	SDL_Texture* drawnTex = loadSprTexture(flags);

	if (drawnTex == NULL)
	{
		Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00; amask = 0x000000ff;
#else
		rmask = 0x000000ff; gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;
#endif
		SDL_Surface* ti = SDL_CreateRGBSurface(0, size_x * 8, size_y * 8, 32,
			rmask, gmask, bmask, amask);

		//set up
		palette = palette << 4;
		SDL_LockSurface(ti);

		uint_fast8_t color1 = 0;
		for (uint_fast8_t x_b = 0; x_b < size_x; x_b++) //Shitty loop thing sorry
		{
			for (uint_fast8_t y_b = 0; y_b < size_y; y_b++)
			{
				uint_fast8_t x_off_t = x_b << 3;
				uint_fast8_t y_off_t = y_b << 3;

				//cout << dec << int(x_b) << ", " << int(y_b) << endl;
				uint_fast16_t curr_tile = ((tile + x_b + (y_b << 4)) << 5) + 0xC000;
				if (curr_tile > 0xFFE0) //Fix a crash
				{
					curr_tile = 0xFFE0;
				}

				uint_fast8_t graphics_array[32];
				memcpy(graphics_array, &VRAM[curr_tile], 32 * sizeof(uint_fast8_t));

				//drawing a block now.
				for (uint_fast8_t index = 0; index < 8; index++)
				{
					//stupid method here, can be optimized as fuck.
					uint_fast8_t ind = index << 1;

					for (uint_fast8_t i = 0; i < 8; i++)
					{
						color1 =
							((graphics_array[0 + ind] >> i) & 1) +
							(((graphics_array[1 + ind] >> i) & 1) << 1) +
							(((graphics_array[16 + ind] >> i) & 1) << 2) +
							(((graphics_array[17 + ind] >> i) & 1) << 3)
							;

						if (color1 != 0)
						{
							drawRect(7 - i + x_off_t, index + y_off_t, color1 + palette, ti);
						}
					}
				}
			}
		}


		SDL_Texture* NewTex = SDL_CreateTextureFromSurface(ren, ti);
		drawnTex = NewTex;
		addSprTexture(flags, NewTex);


		SDL_FreeSurface(ti);
	}

	SDL_Rect SourceR;
	SourceR.x = x;
	SourceR.y = y;
	SourceR.w = size_x << 3; //Same bs as above.
	SourceR.h = size_y << 3;
	SDL_RenderCopyEx(ren, drawnTex, NULL, &SourceR, angle, NULL, flip);
}