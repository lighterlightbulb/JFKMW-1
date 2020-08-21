#pragma once
/*
	JFK Mario World Overworld Implementation

	Should improve later but for now it's fine.
*/

class overworldSystem
{
public:
	//Some variables
	double PositionX = 192.0;
	double PositionY = 176.0;
	double CamX, CamY;
	double SpeedX = 0.0;
	double SpeedY = 0.0;
	double CamSX, CamSY;
	bool inWater = false;
	bool FreeCam = false;
	bool ResetCam = false;
	bool climbing = false;
	

	uint_fast8_t old_level = 0;

	string level_strings[0x100];



	uint_fast16_t ind_offset[4] = {
			 0,  0x400,
		0x0800, 0x0C00
	};


	//Check if we are standing on a tile
	bool StandingOnTile()
	{
		int TilePosX = int(PositionX) >> 4;
		int TilePosY = int(PositionY + 32) >> 4;

		uint_fast8_t tile = Get_Tile(TilePosX, TilePosY);

		return (tile >= 0x66 && tile <= 0x6D) && !(int(PositionX) & 0xF) && !(int(PositionY) & 0xF);
	}

	//Get the tile at x,y
	uint_fast8_t Get_Tile(uint_fast8_t x, uint_fast8_t y)
	{
		return RAM[0x3000 + x + (y << 5)];
	}

	//Get the level number at x,y
	uint_fast8_t Get_Level(uint_fast8_t x, uint_fast8_t y)
	{
		return RAM[0x3400 + x + (y << 5)];
	}

	//Initialize overworld
	void Initialize()
	{
		//Reset camera
		FreeCam = false;
		ResetCam = false;

		//Free up memory that was probably used in the level
		ClearSpriteCache();
		LevelSprites.clear();
		particles.clear();

		//Set to overworld
		in_Overworld = true;

		//Load exanimation file, and clear out hud memory.
		decode_graphics_file("Graphics/GFX00.bin", 8);
		decode_graphics_file("Graphics/hud.bin", 11);
		memset(&RAM[0x1B800], 0xFF, 0x800);

		//Clear overworld L1 and L2
		memset(&RAM[0x3000], 0x00, 0x400);
		memset(&RAM[0x6000], 0x00, 0x2000);


		//Reset counters
		global_frame_counter = 0;
		ingame_frame_counter = 0;

		//Clear OAM
		memset(&RAM[0x200], 0, 0x400);

		//Load OW Data (With swag)
		ifstream infile(path + "Overworld/MainOW_L1.bin", ios::binary);
		infile.read((char*)&RAM[0x3000], 0x400);
		infile.close();

		ifstream infile2(path + "Overworld/MainOW_L2.bin", ios::binary);
		infile2.read((char*)&RAM[0x6000], 0x2000);
		infile2.close();

		ifstream infile3(path + "Overworld/MainOW_Levels.bin", ios::binary);
		infile3.read((char*)&RAM[0x3400], 0x0400);
		infile3.close();

		//Initialize level names
		ifstream LevelNames(path + "Overworld/LevelNames.txt");
		if (LevelNames.is_open())
		{
			string line;
			int i = 0;
			while (getline(LevelNames, line)) {
				
				auto delimiterPos = line.find("=");
				auto name = line.substr(0, delimiterPos);
				auto value = line.substr(delimiterPos + 1);
				level_strings[stoi(name, nullptr, 16)] = value;
			}
		}
		LevelNames.close();

		//Load OW Palette
		read_from_palette(path + "Overworld/MainOW.mw3");

		decode_graphics_file("Overworld/OW_GFX00.bin", 0);
		decode_graphics_file("Overworld/OW_GFX01.bin", 1);
		decode_graphics_file("Overworld/OW_GFX02.bin", 2);
		decode_graphics_file("Overworld/OW_GFX03.bin", 3);

		//Loaded it
		cout << yellow << "[JFKMW] Loaded overworld" << white << endl;
	}

	//Draw ow tile (Shitty)
	void drawL1Tile(uint_fast8_t tile, int x, int y)
	{
		if (tile >= 0x66 && tile <= 0x6D)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int e = 0; e < 2; e++)
				{
					draw8x8_tile_f(
						x + i * 8,
						y + e * 8,
						0x186, ((tile >> 1) & 1) ? 6 : 7,
						i == 1,
						e == 0
					);
				}
			}
		}
	}

	//Render override
	void Render()
	{
		//Player pos
		int TilePosX = int(PositionX) >> 4;
		int TilePosY = int(PositionY + 32) >> 4;
		uint_fast8_t curr_tile = Get_Tile(TilePosX, TilePosY);

		inWater = (curr_tile >= 0x28 && curr_tile <= 0x3E) || (curr_tile >= 0x6A && curr_tile <= 0x6D);
		climbing = curr_tile == 0x3F;
			 
		//Initialize some variables
		SDL_Rect DestR = { 0, 0, 256, 224 };
		SDL_Rect SrcR = { 0, 0, 256, 224 };
		uint_fast8_t mosaic_val = RAM[0x3F10] >> 4;
		uint_fast8_t bright_val = RAM[0x3F10] & 0xF;

		//Convert 16-bit palette to 32-bit palette
		ConvertPalette();

		//Lock surface for drawing
		SDL_LockSurface(&screen_s_l1);
		SDL_Surface* screen_plane = &screen_s_l1;
		SDL_memset(screen_plane->pixels, 0, screen_plane->h * screen_plane->pitch);

		//Layer 2
		if (drawBg)
		{
			uint_fast16_t Cam_PX_Off = ((int(CamX) >> 4) << 1);
			uint_fast16_t Cam_PY_Off = ((int(CamY) >> 4) << 1);
			//Draw scenery
			for (uint_fast16_t x = 0; x < 30; x++)
			{
				for (uint_fast16_t y = 0; y < 23; y++)
				{
					uint_fast16_t px = x + Cam_PX_Off;
					uint_fast16_t py = y + Cam_PY_Off;
					uint_fast16_t ind = (px & 0x1F) + ((py & 0x1F) << 5) + ind_offset[((px >> 5) + ((py >> 5) << 1)) & 3];
					uint_fast16_t ent = ind << 1;
					uint_fast8_t tile = RAM[0x6000 + ent];
					uint_fast8_t props = RAM[0x6001 + ent];
					draw8x8_tile_f(
						16 + (x << 3),
						40 + (y << 3),
						tile, (props >> 2) & 7,
						props & 0x40, props & 0x80
					);
				}
			}
		}

		if (drawL1)
		{
			//Draw Layer 1
			for (uint_fast16_t x = 0; x < 15; x++)
			{
				for (uint_fast16_t y = 0; y < 12; y++)
				{
					uint_fast16_t px = x + (int(CamX) >> 4);
					uint_fast16_t py = 2 + y + (int(CamY) >> 4);
					uint_fast16_t ind = px + (py << 5);

					uint_fast8_t tile = RAM[0x3000 + ind];
					if (tile != 0)
					{
						drawL1Tile(tile, 16 + (x << 4), 40 + (y << 4));
					}
				}
			}
		}

		//Draw screen s l1
		SDL_UnlockSurface(&screen_s_l1);
		//We can now draw the screen finished product.

		SDL_DestroyTexture(screen_t_l1);
		screen_t_l1 = SDL_CreateTextureFromSurface(ren, &screen_s_l1);
		if (mosaic_val > 0)
		{
			uint_fast8_t m = min(16, (2 + mosaic_val));
			SrcR.w = 1;
			SrcR.h = 1;

			DestR.w = 1 * m;
			DestR.h = 1 * m;

			uint_fast16_t draw_x = 256 / m;
			uint_fast16_t draw_y = 224 / m;
			draw_x++;
			draw_y++;

			for (uint_fast16_t x = 0; x < draw_x; x++)
			{
				for (uint_fast16_t y = 0; y < draw_y; y++)
				{
					SrcR.x = (x * m);
					SrcR.y = (y * m);

					DestR.x = ((int_res_x - 256) / 2) -(int(CamX) & 0xF) + x * m;
					DestR.y = ((int_res_y - 224) / 2) -(int(CamY) & 0xF) + y * m;

					SDL_RenderCopy(ren, screen_t_l1, &SrcR, &DestR);
				}
			}
		}
		else
		{
			DestR.x = ((int_res_x - 256) / 2) - (int(CamX) & 0xF);
			DestR.y = ((int_res_y - 224) / 2) - (int(CamY) & 0xF);
			DestR.w = int_res_x + 16;
			DestR.h = int_res_y + 16;
			SDL_RenderCopy(ren, screen_t_l1, nullptr, &DestR);
		}


		//Might have to be redone later, but for now it works.
		DestR.x = ((int_res_x - 256) / 2) + 16 + int(PositionX - CamX);
		DestR.y = ((int_res_y - 224) / 2) + (inWater ? 33 : 28) + int(PositionY - CamY);
		DestR.w = 16;
		DestR.h = inWater ? 15 : 24;

		SrcR.x = 0;
		SrcR.y = 0;
		SrcR.w = 16;
		SrcR.h = inWater ? 15 : 24;

		//Mario sprite
		string SPR = "OW0";
		if (SpeedX != 0.0) {
			SPR = "OW1";
		}
		if (SpeedY < 0) {
			SPR = "OW2";
		}
		if (RAM[0x3F11] == 4) {
			SPR = "OW3";
		}
		if (climbing) {
			SPR = "OW4";
		}

		string spr_p = path + "Sprites/mario/" + to_string(my_skin) + "/OW/" + SPR + "_" + to_string((ingame_frame_counter >> 3) & 3) + ".png";

		SDL_RendererFlip flip = SpeedX < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
		SDL_RenderCopyEx(ren, TexManager.loadTexture(spr_p), &SrcR, &DestR, 0, NULL, flip);

		//Water thing
		if (inWater)
		{
			CreateSprite(path + "Sprites/OW/OW_Water_" + to_string((ingame_frame_counter >> 3) & 1) + ".png", DestR.x, DestR.y + 15, 16, 4);
		}

		//Draw border and HUD
		if (hudMode < 2)
		{
			//The border
			DestR.x = ((int_res_x - 256) / 2);
			DestR.y = ((int_res_y - 224) / 2);
			DestR.w = 256;
			DestR.h = 224;
			SDL_RenderCopy(ren, TexManager.loadTexture(path + "Sprites/OW/OWBorder.png"), NULL, &DestR);

			if (FreeCam && (ingame_frame_counter & 0x1F) < 0x18 && !ResetCam)
			{
				SDL_RenderCopy(ren, TexManager.loadTexture(path + "Sprites/OW/OWBorderArrows.png"), NULL, &DestR);
			}
			//Walking mario on the OW border
			DestR.x = ((int_res_x - 256) / 2) + 24;
			DestR.y = ((int_res_y - 224) / 2) + 6;
			DestR.w = 32;
			DestR.h = 32;
			SDL_RenderCopy(ren, TexManager.loadTexture(path + "Sprites/mario/" + to_string(my_skin) + "/WALK" + to_string((ingame_frame_counter >> 3) % 3) + "_1.png"), NULL, &DestR);

			//Text
			if (StandingOnTile())
			{
				if (Get_Level(TilePosX, TilePosY) != old_level)
				{
					old_level = Get_Level(TilePosX, TilePosY);
					RAM[0x1DFC] = 0x23;
				}
				string text = level_strings[old_level];
				int y = 31 - (int(text.length())/19) * 8;
				for (int i = 0; i < text.length(); i++)
				{
					uint_fast8_t t = char_to_smw(text.at(i));

					draw8x8_tile_2bpp((11 + (i % 19)) << 3, y + (i / 19) * 8, t, 7);
				}
			}

			//Lives
			draw8x8_tile_2bpp(64, 30, 0, 6);
		}

		//Draw diagnostics
		if (hudMode == 1)
		{
			int fps = int(1.0 / (total_time_ticks.count() / 1.0));
			string t1 = "Version " + GAME_VERSION;
			string t2 = to_string(fps) + " fps";
			for (int i = 0; i < t1.length(); i++)
			{
				uint_fast8_t t = char_to_smw(t1.at(i));
				draw8x8_tile_2bpp(1 + ((2 + i) << 3), 224 - 34, t, 6);
			}
			for (int i = 0; i < t2.length(); i++)
			{
				uint_fast8_t t = char_to_smw(t2.at(i));
				draw8x8_tile_2bpp(1 + ((2 + i) << 3), 224 - 26, t, 6);
			}
		}


		//Chat
		Render_Chat();

		//Draw L3
		for (uint_fast16_t t3_x = 0; t3_x < 32; t3_x++)
		{
			for (uint_fast16_t t3_y = 0; t3_y < 28; t3_y++)
			{
				if (VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)] < MAX_L3_TILES)
				{
					draw8x8_tile_2bpp(t3_x << 3, t3_y << 3, VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)], VRAM[0xB801 + (t3_x << 1) + (t3_y << 6)]);
				}
			}
		}

		//Darken the screen globally
		SDL_SetRenderDrawColor(ren, 0, 0, 0, bright_val == 0xF ? 255 : (bright_val << 4));
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(ren, nullptr);
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	}
	
	//Process OW
	void Process()
	{
		RAM[0x1493] = 0;
		RAM[0x1DFB] = 0x83;

		/*
			Animate water (To-do dedicate own exanimation)
		*/

		memcpy(&RAM[VRAM_Location + (0x75 * 32)], &RAM[VRAM_Location + 0x2000 + (((ingame_frame_counter >> 3) & 7) << 5)], 32);
		memcpy(&RAM[VRAM_Location + (0x7D * 32)], &RAM[VRAM_Location + 0x8000 + (0x68 * 32) + (((ingame_frame_counter >> 3) & 7) << 5)], 32);
		memcpy(&RAM[VRAM_Location + (0x76 * 32)], &RAM[VRAM_Location + 0x2000 + (0x08 * 32) + (((ingame_frame_counter >> 3) & 7) << 5)], 32);

		/*
			Fade in
		*/
		if (global_frame_counter == 2)
		{
			RAM[0x3F10] = 0x0F;
			RAM[0x3F11] = 2;
		}

		/*
			Cam toggle
		*/
		if (pad_p[button_start])
		{
			if (FreeCam)
			{
				if (!ResetCam)
				{
					ResetCam = true;
					double gotoX = PositionX - (120.0 - 16.0);
					double gotoY = PositionY - (104.0 - 24.0);
					gotoX = min(287.0, max(0.0, gotoX));
					gotoY = min(343.0, max(0.0, gotoY));

					double ang = atan2(gotoY - CamY, gotoX - CamX);
					CamSX = cos(ang) * 5.0;
					CamSY = sin(ang) * 5.0;
				}
			}
			else
			{
				FreeCam = true;
				ResetCam = false;
			}
		}

		/*
			Modes
		*/
		if (!FreeCam)
		{
			/*
				Movement
			*/

			int TilePosX = int(PositionX) >> 4;
			int TilePosY = int(PositionY + 32) >> 4;

			if (StandingOnTile())
			{
				SpeedX = pad_p[button_right] - pad_p[button_left];
				SpeedY = pad_p[button_down] - pad_p[button_up];

				if (SpeedX != 0 && Get_Tile(uint_fast8_t(TilePosX + SpeedX), TilePosY) == 0x0) {
					SpeedX = 0;
				}
				if (SpeedY != 0 && Get_Tile(uint_fast8_t(TilePosX), uint_fast8_t(TilePosY + SpeedY)) == 0x0) {
					SpeedY = 0;
				}
			}

			PositionX += SpeedX;
			PositionY += SpeedY / (1 + climbing);

			/*
				Enter level
			*/
			if (((pad_p[button_b] || pad_p[button_a]) || pad_p[button_y]) && StandingOnTile() && RAM[0x3F11] != 4)
			{
				int TilePosX = int(PositionX) >> 4;
				int TilePosY = int(PositionY + 32) >> 4;

				ASM.Write_To_Ram(0x3F08, Get_Level(TilePosX, TilePosY), 2);
				RAM[0x3F11] = 4;
				RAM[0x3F10] = 0x00;
			}


			/*
				Camera IG
			*/
			CamX = PositionX - (120.0 - 16.0);
			CamY = PositionY - (104.0 - 24.0);
		}
		else
		{
			/*
				FreeCam mode
			*/
			if (!ResetCam) {
				CamX += (s_pad[button_right] - s_pad[button_left]) * 2;
				CamY += (s_pad[button_down] - s_pad[button_up]) * 2;
			}
			else {
				CamX += CamSX;
				CamY += CamSY;
				double gotoX = PositionX - (120.0 - 16.0);
				double gotoY = PositionY - (104.0 - 24.0);
				gotoX = min(287.0, max(0.0, gotoX));
				gotoY = min(343.0, max(0.0, gotoY));

				double dist = sqrt(pow(gotoX - CamX, 2) + pow(gotoY - CamY, 2));
				if (dist < 8)
				{
					ResetCam = false;
					FreeCam = false;
					CamX = gotoX;
					CamY = gotoY;
				}
			}
		}
		CamX = min(287.0, max(0.0, CamX));
		CamY = min(343.0, max(0.0, CamY));

		if (!RAM[0x3F11])
		{
			ingame_frame_counter += 1;
		}
	}
};

overworldSystem overworld;