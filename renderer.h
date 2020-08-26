#pragma once

void draw_number_hex(uint_fast8_t pos_x, uint_fast8_t pos_y, uint_fast16_t number, int length)
{
	for (int i = 0; i < length; i++)
	{
		VRAM[0xB800 + (-i * 2) + (pos_x * 2) + pos_y * 64] = (number >> (i << 2)) & 0xF;
		VRAM[0xB801 + (-i * 2) + (pos_x * 2) + pos_y * 64] = 6;
	}

}

void draw_number_dec(uint_fast8_t pos_x, uint_fast8_t pos_y, int number, uint_fast8_t off = 0, uint_fast8_t props = 6)
{
	int length = int(to_string(number).length());
	for (int i = 0; i < length; i++)
	{
		VRAM[0xB800 + (-i * 2) + (pos_x * 2) + pos_y * 64] = off + (int(number / pow(10, i)) % 10);
		VRAM[0xB801 + (-i * 2) + (pos_x * 2) + pos_y * 64] = props;
	}

}

void render_oam(uint_fast16_t offset_o = 0, bool pr_only = false, int_fast16_t res_x = 256, int_fast16_t res_y = 224)
{
	for (uint_fast16_t i = 0; i < 0x400; i += 8) //Tile, Size, XY (4 bytes), PAL, ANG, in total 8 bytes per entry. 0 to 7.
	{
		uint_fast8_t flags = RAM[offset_o + i + 6] >> 4;

		if (bool(flags >> 3) == pr_only)
		{
			uint_fast8_t size = RAM[offset_o + i + 1];
			int_fast16_t size_x = (1 + (size & 0xF)) << 3;
			int_fast16_t size_y = (1 + ((size >> 4) & 0xF)) << 3;
			int x_position = RAM[offset_o + i + 2] + int_fast8_t(RAM[offset_o + i + 3]) * 256;
			int y_position = RAM[offset_o + i + 4] + int_fast8_t(RAM[offset_o + i + 5]) * 256;
			uint_fast16_t tile = RAM[offset_o + i] + ((flags & 1) << 8);

			if (tile != 0x0 &&
				(x_position - CameraX) > (-size_x) && (x_position - CameraX) < (res_x + size_x) &&
				(y_position - CameraY) > (-16 - size_y) && (y_position - CameraY) < (res_y + size_y)
				)
			{
				if (drawDiag)
				{
					blocks_on_screen += ((size_x >> 4) * (size_y >> 4)) * 4;
				}
				uint_fast8_t pal = RAM[offset_o + i + 6] & 0xF;
				double angle = (double(RAM[offset_o + i + 7]) / 256.0) * 360.0;
				draw_tile_custom(x_position - CameraX, res_y - 32 - y_position + CameraY, size, angle, tile, pal,
					SDL_RendererFlip(
						((flags >> 1) & 1) +
						(((flags >> 2) & 1) << 1)
					)
				);
			}
		}
	}
}

void drawBackground(uint_fast16_t res_x = 256, uint_fast16_t res_y = 224)
{
	if (RAM[0x3F05] != curr_bg)
	{
		curr_bg = RAM[0x3F05];
		bg_texture = TexManager.loadTexture(path + "Sprites/backgrounds/Background" + to_string(int(curr_bg)) + ".png");
	}

	int formula_x = (-int(double(CameraX) * (double(RAM[0x3F06]) / 16.0) + ASM.Get_Ram(0x1466, 2))) % 512;
	int formula_y = (int(double(CameraY) * (double(RAM[0x3F07]) / 16.0) + ASM.Get_Ram(0x1468, 2))) % 512;

	SDL_Rect DestR;
	SDL_Rect SrcR;
	uint_fast8_t mosaic_val = RAM[0x3F10] >> 4;
	if (mosaic_val > 0)
	{
		uint_fast8_t m = min(16, (3 + mosaic_val));
		SrcR.w = 1;
		SrcR.h = 1;

		DestR.w = 1 * m;
		DestR.h = 1 * m;

		uint_fast16_t draw_x = res_x / m;
		uint_fast16_t draw_y = res_y / m;
		draw_x++;
		draw_y++;

		for (uint_fast16_t x = 0; x < draw_x; x++)
		{
			for (uint_fast16_t y = 0; y < draw_y; y++)
			{
				SrcR.x = (-formula_x + (x * m)) & 0x1FF;
				SrcR.y = (-formula_y + (272 + (224-res_y)) + (y * m)) % 512;

				DestR.x = x * m;
				DestR.y = y * m;

				SDL_RenderCopy(ren, bg_texture, &SrcR, &DestR);
			}
		}
	}
	else
	{
		if (layer2mode_x || layer2mode_y)
		{

			SrcR.x = 0;
			SrcR.w = 512;
			SrcR.h = 1;
			DestR.w = 512;
			DestR.h = 1;

			uint_fast8_t l_s = min(224, res_y);
			for (int x = -1; x < 3; x++)
			{
				for (uint_fast8_t i = 0; i < l_s; i++)
				{
					SrcR.y = (-formula_y + 256 + int(i) + int(layer2_shiftY[i] & 0x1FF)) & 0x1FF;

					DestR.x = ((layer2_shiftX[i] & 0x1FF) + formula_x + (x * 512));
					DestR.y = i;

					SDL_RenderCopy(ren, bg_texture, &SrcR, &DestR);
				}
			}
		}
		else
		{

			double bg_scale_x = 32.0 / double(RAM[0x38]);
			double bg_scale_y = 32.0 / double(RAM[0x39]);
			int off_x = int(512.0 * bg_scale_x);
			int off_y = int(512.0 * bg_scale_y);

			int am_x = max(1, int(2.0 / bg_scale_x));
			int am_y = max(1, int(2.0 / bg_scale_y));


			for (int x = 0; x < am_x; x++) {
				for (int y = 0; y < am_y; y++) {
					RenderBackground(
						formula_x + x * off_x,
						-272 + (res_y - 224) + formula_y + (y * -off_y) + (512 - off_y)
					);
				}
			}
		}
	}
}

void handleRenderingForPlayer(int player, uint_fast16_t res_x = 256, uint_fast16_t res_y = 224)
{
	blocks_on_screen = 0;
	CheckForPlayers();
	if (Mario.size() < player)
	{
		return;
	}

	//Initialize destR variable were gonna use later for rendering
	SDL_Rect DestR;
	SDL_Rect SrcR;

	/*
		Sorry for shitcode
	*/
	uint_fast8_t transition_type = RAM[0x1493] > 0 ? 3 : RAM[0x3F11];
	uint_fast8_t mosaic_val = RAM[0x3F10] >> 4;
	uint_fast8_t bright_val = RAM[0x3F10] & 0xF;

	MPlayer& LocalPlayer = get_mario(player);
	LocalPlayer.ProcessCamera();
	CameraX = int_fast16_t(LocalPlayer.CAMERA_X - ((res_x / 2) - 8));
	CameraY = int_fast16_t(max(0.0, LocalPlayer.CAMERA_Y - (res_y / 2)));
	if (RAM[0x1887] > 0)
	{
		CameraY += (global_frame_counter % 3);
	}

	if (CameraX < 0) { CameraX = 0; }
	if (CameraY < 0) { CameraY = 0; }
	if (CameraX > (-int_fast16_t(res_x) + int_fast16_t(mapWidth) * 16))
	{
		CameraX = (-int_fast16_t(res_x) + int_fast16_t(mapWidth) * 16);
	}
	if (CameraY > (-int_fast16_t(res_y) + int_fast16_t(mapHeight) * 16))
	{
		CameraY = (-int_fast16_t(res_y) + int_fast16_t(mapHeight) * 16);
	}

	int_fast16_t offsetX = int_fast16_t(CameraX >> 4);
	uint_fast8_t offsetXPixel = uint_fast8_t(CameraX) & 0xF;
	int_fast16_t offsetY = int_fast16_t(CameraY >> 4);
	uint_fast8_t offsetYPixel = uint_fast8_t(CameraY) & 0xF;

	//Draw BG
	if (drawBg) {
		drawBackground(res_x, res_y);
	}

	if (drawL1)
	{
		// Start rendering, by locking surface.
		SDL_LockSurface(&screen_s_l1);
		SDL_Surface* screen_plane = &screen_s_l1;
		SDL_memset(screen_plane->pixels, 0, screen_plane->h * screen_plane->pitch);


		//Draw scenery
		uint_fast8_t int_b_x = uint_fast8_t(res_x / 16) + 1;
		uint_fast8_t int_b_y = uint_fast8_t(res_y / 16) + 1;


		for (uint_fast8_t x = 0; x < int_b_x; x++)
		{
			for (uint_fast8_t y = 0; y < int_b_y; y++)
			{
				uint_fast16_t tile = map16_handler.get_tile(x + offsetX, y + offsetY) & 0x1FF;

				if (tile != 0x25)
				{
					uint_fast16_t entry = tile << 4;
					uint_fast8_t block_palette;

					//Block 8x8 tiles
					for (uint_fast8_t i = 0; i < 4; i++)
					{
						uint_fast16_t block_index = map16_entries[entry + 1 + (i << 1)] + (map16_entries[entry + (i << 1)] << 8);
						uint_fast8_t index = map16_entries[entry + tile_palette_2 - (i <= 1)];

						if (tile >= 0x133 && tile <= 0x136)
						{
							block_palette = pipe_colors[((x + offsetX) >> 4) & 3];
						}
						else
						{
							block_palette = index >> ((i & 1) << 2) & 0xF; // (i & 1) ? (index & 0xF) : (index >> 4);
						}
						if (block_index != 0xF8)
						{
							if (drawDiag)
							{
								blocks_on_screen++;
							}
							draw8x8_tile(
								((i << 3) & 0xF) + (x << 4),
								(res_y) - (i > 1 ? -8 : 0) - (y << 4),
								block_index, block_palette
							);
						}
					}
				}
			}
		}


		//End rendering
		SDL_UnlockSurface(&screen_s_l1);
		//We can now draw the screen finished product.

		SDL_DestroyTexture(screen_t_l1);
		screen_t_l1 = SDL_CreateTextureFromSurface(ren, &screen_s_l1);

		if (RAM[0x40] != 0)
		{
			SDL_SetTextureBlendMode(screen_t_l1, SDL_BlendMode(RAM[0x40]));
		}

		if (mosaic_val > 0)
		{
			uint_fast8_t m = min(16, (2 + mosaic_val));
			SrcR.w = 1;
			SrcR.h = 1;

			DestR.w = 1 * m;
			DestR.h = 1 * m;

			uint_fast16_t draw_x = res_x / m;
			uint_fast16_t draw_y = res_y / m;
			draw_x++;
			draw_y++;

			for (uint_fast16_t x = 0; x < draw_x; x++)
			{
				for (uint_fast16_t y = 0; y < draw_y; y++)
				{
					SrcR.x = (x * m);
					SrcR.y = 16 + (y * m);

					DestR.x = x * m;
					DestR.y = y * m;

					SDL_RenderCopy(ren, screen_t_l1, &SrcR, &DestR);
				}
			}
		}
		else
		{
			if (!layer1mode_y && !layer1mode_x)
			{
				SrcR.x = 0;
				SrcR.y = 0;
				SrcR.w = res_x + 16;
				SrcR.h = res_y + 16;

				DestR.x = -offsetXPixel;
				DestR.y = -16 + offsetYPixel;
				DestR.w = res_x + 16;
				DestR.h = res_y + 16;
				SDL_RenderCopy(ren, screen_t_l1, &SrcR, &DestR);
			}
			else
			{
				SrcR.x = 0;
				SrcR.w = res_x + 16;
				SrcR.h = 1;
				DestR.w = res_x + 16;
				DestR.h = 1;

				
				for (uint_fast16_t i = 0; i < 256; i++)
				{
					SrcR.y = (i + (layer1_shiftY[(-offsetYPixel + i) % 224])) % 256;

					DestR.y = ((i + -16 + offsetYPixel));
					DestR.x = (-offsetXPixel + layer1_shiftX[(offsetYPixel + (i - 16)) % 224]);

					SDL_RenderCopy(ren, screen_t_l1, &SrcR, &DestR);
				}
			}
		}

		
	}

	//Draw screen darkening (Level Clear)
	if (transition_type == 3)
	{
		SDL_SetRenderDrawColor(ren, 0, 0, 0, bright_val == 0xF ? 255 : (bright_val << 4));
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(ren, nullptr);
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	}


	//Draw OAM (under)
	if (drawSprites)
	{
		render_oam(0x200, false, res_x, res_y);
	}

	//Draw Mario
	for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
	{


		MPlayer& CurrentMario = *item;

		if (CurrentMario.x > (CameraX - camBoundX) && CurrentMario.y > (CameraY - camBoundY) && CurrentMario.x < (CameraX + res_x + camBoundX) && CurrentMario.y < (CameraY + res_y + camBoundY))
		{
			float is_skidding = 1.f - (float(abs(CurrentMario.SKIDDING)) * 2.f);

			int offs = 0;

			if (CurrentMario.GRABBED_SPRITE != 0xFF && !CurrentMario.in_pipe)
			{
				uint_fast8_t tile = uint_fast8_t(ASM.Get_Ram(0x2F00 + CurrentMario.GRABBED_SPRITE, 1));
				if (tile != 0)
				{
					uint_fast8_t size = uint_fast8_t(((ASM.Get_Ram(0x2E80 + CurrentMario.GRABBED_SPRITE, 1) & 0x7F) >> 4) + (((ASM.Get_Ram(0x2E80 + CurrentMario.GRABBED_SPRITE, 1) & 0x7F) >> 4) << 4));

					int_fast16_t x_position = int_fast16_t(double(CurrentMario.x + CurrentMario.to_scale * -12.0));
					int_fast16_t y_position = int_fast16_t(double(CurrentMario.y - (CurrentMario.STATE > 0 ? 13.0 : 16.0) - (CurrentMario.CROUCH ? 2 : 0) * CurrentMario.STATE));

					uint_fast8_t pal = uint_fast8_t(ASM.Get_Ram(0x2E80 + CurrentMario.GRABBED_SPRITE, 1) & 0xF);

					double angle = 0.0;
					draw_tile_custom(x_position - CameraX, res_y - 32 - y_position + CameraY, size, angle, tile, pal, SDL_FLIP_NONE);
				}
			}

			if (!CurrentMario.invisible)
			{
				CreateSprite(path + "Sprites/mario/" + to_string(CurrentMario.skin) + "/" + CurrentMario.sprite + ".png",
					-8 + int(CurrentMario.x) - int(CameraX), 
					res_y - 32 - int(CurrentMario.y) + int(CameraY), 
					int(CurrentMario.to_scale * is_skidding) * 32, 
					32);
			}
		}
	}

	//Draw OAM (priority)
	if (drawSprites)
	{
		render_oam(0x200, true, res_x, res_y);
	}

	/*
		Normal hud
	*/
	if (hudMode == 0)
	{
		//Status bar code here
		for (int i = 0; i < 5; i++)
		{
			VRAM[0xB804 + (i * 2) + 128] = 0x40 + i + (my_skin % 3) * 5;
			VRAM[0xB805 + (i * 2) + 128] = 3;
		}

		//WO's
		VRAM[0xB806 + 192] = 0x26;
		VRAM[0xB807 + 192] = 0x6;
		draw_number_dec(5, 3, 99 - LocalPlayer.WO_counter);

		//Dragon coins
		for (uint_fast8_t d_c = 0; d_c < RAM[0x1420]; d_c++)
		{
			VRAM[0xB800 + ((d_c + 8) * 2) + 128] = 0x2F;
			VRAM[0xB801 + ((d_c + 8) * 2) + 128] = 0x7;
		}
		VRAM[0xB800 + (9 * 2) + 192] = 0x6A;
		VRAM[0xB801 + (9 * 2) + 192] = 0x3;
		VRAM[0xB800 + (10 * 2) + 192] = 0x26;
		VRAM[0xB801 + (10 * 2) + 192] = 0x6;

		//Bonus stars
		draw_number_dec(13, 2, 0, 0x6B);
		draw_number_dec(13, 3, 0, 0x75);

		//Coins
		draw_number_dec(29, 2, RAM[0x0DBF] % 100);

		VRAM[0xB800 + (25 * 2) + 128] = 0x2F;
		VRAM[0xB801 + (25 * 2) + 128] = 0x7;
		VRAM[0xB800 + (26 * 2) + 128] = 0x26;
		VRAM[0xB801 + (26 * 2) + 128] = 0x6;

		//Score
		draw_number_dec(29, 3, int(LocalPlayer.KO_counter) * 200);


		//Reserve

		/* Top */
		VRAM[0xB800 + (14 * 2) + 64] = 0x30;
		VRAM[0xB801 + (14 * 2) + 64] = 0x3;
		VRAM[0xB800 + (15 * 2) + 64] = 0x31;
		VRAM[0xB801 + (15 * 2) + 64] = 0x3;
		VRAM[0xB800 + (16 * 2) + 64] = 0x31;
		VRAM[0xB801 + (16 * 2) + 64] = 0x3;
		VRAM[0xB800 + (17 * 2) + 64] = 0x32;
		VRAM[0xB801 + (17 * 2) + 64] = 0x3;

		/* Middle */
		VRAM[0xB800 + (14 * 2) + 128] = 0x33;
		VRAM[0xB801 + (14 * 2) + 128] = 0x3;
		VRAM[0xB800 + (17 * 2) + 128] = 0x34;
		VRAM[0xB801 + (17 * 2) + 128] = 0x3;
		VRAM[0xB800 + (14 * 2) + 192] = 0x33;
		VRAM[0xB801 + (14 * 2) + 192] = 0x3;
		VRAM[0xB800 + (17 * 2) + 192] = 0x34;
		VRAM[0xB801 + (17 * 2) + 192] = 0x3;

		/* Bottom */
		VRAM[0xB800 + (14 * 2) + 256] = 0x35;
		VRAM[0xB801 + (14 * 2) + 256] = 0x3;
		VRAM[0xB800 + (15 * 2) + 256] = 0x36;
		VRAM[0xB801 + (15 * 2) + 256] = 0x3;
		VRAM[0xB800 + (16 * 2) + 256] = 0x36;
		VRAM[0xB801 + (16 * 2) + 256] = 0x3;
		VRAM[0xB800 + (17 * 2) + 256] = 0x37;
		VRAM[0xB801 + (17 * 2) + 256] = 0x3;

		//Time
		VRAM[0xB800 + (19 * 2) + 128] = 0x2E;
		VRAM[0xB801 + (19 * 2) + 128] = 0x7;
		VRAM[0xB800 + (20 * 2) + 128] = 0x3F;
		VRAM[0xB801 + (20 * 2) + 128] = 0x7;
		VRAM[0xB800 + (21 * 2) + 128] = 0x4F;
		VRAM[0xB801 + (21 * 2) + 128] = 0x7;

		//Change later
		draw_number_dec(21, 3, 999 - ingame_frame_counter / 60, 0, 7);

		//KB & Ping
		if (networking)
		{
			VRAM[0xB800 + (12 * 2) + 192] = 0x14;	VRAM[0xB801 + (12 * 2) + 192] = 6;
			VRAM[0xB800 + (13 * 2) + 192] = 0x0B;	VRAM[0xB801 + (13 * 2) + 192] = 6;
			draw_number_dec(11, 3, data_size_now / 1024);

			VRAM[0xB800 + (12 * 2) + 128] = 0x16;	VRAM[0xB801 + (12 * 2) + 128] = 6;
			VRAM[0xB800 + (13 * 2) + 128] = 0x1C;	VRAM[0xB801 + (13 * 2) + 128] = 6;
			draw_number_dec(11, 2, ((abs(latest_server_response) % 3600) % 1000) / 3);
		}
	}
	/*
		Debug hud
	*/
	if (hudMode == 1)
	{
		//Status bar code here
		for (int i = 0; i < 5; i++)
		{
			VRAM[0xB804 + (i * 2) + 128] = 0x40 + i + (my_skin % 3) * 5;
			VRAM[0xB805 + (i * 2) + 128] = 3;
		}

		//WO's
		VRAM[0xB806 + 192] = 0x26;
		VRAM[0xB807 + 192] = 0x6;
		draw_number_dec(5, 3, LocalPlayer.WO_counter);

		//Player X/Y
		draw_number_hex(21, 2, int(LocalPlayer.x), 4);
		draw_number_hex(26, 2, int(LocalPlayer.y), 4);

		//Player Speed X/Y
		draw_number_hex(16, 2, uint_fast16_t(LocalPlayer.Y_SPEED * 256.0), 4);
		draw_number_hex(11, 2, uint_fast16_t(LocalPlayer.X_SPEED * 256.0), 4);

		//Networking symbols
		VRAM[0xB800 + 20 + 192] = networking ? 0x17 : 0x15; VRAM[0xB801 + 20 + 192] = 6;
		VRAM[0xB800 + 18 + 192] = 0x3A; VRAM[0xB801 + 18 + 192] = 6;
		VRAM[0xB800 + 16 + 192] = isClient ? 0xC : 0x1C; VRAM[0xB801 + 16 + 192] = 6;

		//FCounter
		draw_number_hex(29, 2, RAM[0x14], 2);

		//Ping
		VRAM[0xB800 + 56 + 192] = 0x16;	VRAM[0xB801 + 56 + 192] = 6;
		VRAM[0xB800 + 58 + 192] = 0x1C;	VRAM[0xB801 + 58 + 192] = 6;
		draw_number_dec(27, 3, ((abs(latest_server_response) % 3600) % 1000) / 3);

		//FPS
		VRAM[0xB800 + 44 + 192] = 0xF;	VRAM[0xB801 + 44 + 192] = 6;
		VRAM[0xB800 + 46 + 192] = 0x19;	VRAM[0xB801 + 46 + 192] = 6;
		VRAM[0xB800 + 48 + 192] = 0x1C;	VRAM[0xB801 + 48 + 192] = 6;
		draw_number_dec(21, 3, int(1.0 / (total_time_ticks.count() / 1.0)));

		//KB
		//VRAM[0xB800 + 26 + 192] = 0x24;	VRAM[0xB801 + 26 + 192] = 6;
		VRAM[0xB800 + 30 + 192] = 0x14;	VRAM[0xB801 + 30 + 192] = 6;
		VRAM[0xB800 + 32 + 192] = 0x0B;	VRAM[0xB801 + 32 + 192] = 6;
		draw_number_dec(14, 3, data_size_now / 1024);
	}
	//End hud

	Render_Chat();

	//Player list logic (shouldn't be here, but oh well)
	bool stat = (state[input_settings[8]]) || BUTTONS_GAMEPAD[9];
	if (stat != pressed_select)
	{
		pressed_select = stat;
		if (stat)
		{
			RAM[0x1DFC] = 0x15;
			showing_player_list = !showing_player_list;
		}
	}

	//Player list
	if (showing_player_list)
	{
		memset(&VRAM[0xBC00], 0xFF, 896); //Clear bottom


		int y = 26;
		int plr_numb = 1;
		for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
		{
			MPlayer& CurrentMario = *item;
			//Draw the PlayerNumber) icon
			VRAM[0xB802 + (y << 6)] = plr_numb;
			VRAM[0xB803 + (y << 6)] = 6;
			VRAM[0xB804 + (y << 6)] = 0x3C;
			VRAM[0xB805 + (y << 6)] = 6;
			for (int i = 0; i < player_name_size; i++)
			{
				VRAM[0xB808 + (i * 2) + (y << 6)] = char_to_smw(CurrentMario.player_name_cut[i]);
				VRAM[0xB809 + (i * 2) + (y << 6)] = 6;
			}

			//KO's
			draw_number_dec(17, y, CurrentMario.KO_counter);
			VRAM[0xB826 + (y << 6)] = 0x14; VRAM[0xB827 + (y << 6)] = 6;
			VRAM[0xB828 + (y << 6)] = 0x18; VRAM[0xB829 + (y << 6)] = 6;
			VRAM[0xB82A + (y << 6)] = 0x39; VRAM[0xB82B + (y << 6)] = 6;
			VRAM[0xB82C + (y << 6)] = 0x1C; VRAM[0xB82D + (y << 6)] = 6;

			//WO's
			draw_number_dec(25, y, CurrentMario.WO_counter);
			VRAM[0xB836 + (y << 6)] = 0x20; VRAM[0xB837 + (y << 6)] = 6;
			VRAM[0xB838 + (y << 6)] = 0x18; VRAM[0xB839 + (y << 6)] = 6;
			VRAM[0xB83A + (y << 6)] = 0x39; VRAM[0xB83B + (y << 6)] = 6;
			VRAM[0xB83C + (y << 6)] = 0x1C; VRAM[0xB83D + (y << 6)] = 6;




			y -= 1;
			plr_numb += 1;
		}
	}

	//Draw L3 player names
	for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
	{
		MPlayer& CurrentMario = *item;

		int s_off_x = (res_x - 256) / 2;
		int s_off_y = (res_y - 224) / 2;
		if (!CurrentMario.PlayerControlled && CurrentMario.x > (CameraX - camBoundX) && CurrentMario.y > (CameraY - camBoundY) && CurrentMario.x < (CameraX + res_x + camBoundX) && CurrentMario.y < (CameraY + res_y + camBoundY))
		{
			for (int i = 0; i < 5; i++)
			{
				uint_fast8_t new_l = char_to_smw(CurrentMario.player_name_cut[i]);
				draw8x8_tile_2bpp(-s_off_x + -12 + int(CurrentMario.x) - int(CameraX) + i * 8, s_off_y + 224 - int(CurrentMario.y + (CurrentMario.STATE ? 40 : 32)) + int(CameraY), new_l, 6);
			}
		}
	}

	//Draw windows (Messagebox, retry, etc)
	if (RAM[0x1B89] > 0)
	{

		double Window_Size_X = (RAM[0x1B88] >> 1) ? 146.0 : 160.0;
		double Window_Size_Y = (RAM[0x1B88] >> 1) ? 56.0 : 80.0;
		int DestinationX = 128;
		int DestinationY = (RAM[0x1B88] >> 1) ? 122 : 80;

		Window_Size_X = Window_Size_X * ((1.0 + double(RAM[0x1B89])) / 256.0);
		Window_Size_Y = Window_Size_Y * ((1.0 + double(RAM[0x1B89])) / 256.0);

		DestR.x = ((res_x - 256) / 2) + DestinationX - int(Window_Size_X / 2.0);
		DestR.y = DestinationY - int(Window_Size_Y / 2.0);
		DestR.w = int(Window_Size_X);
		DestR.h = int(Window_Size_Y);
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderFillRect(ren, &DestR);
	}

	//Draw L3
	uint_fast16_t rows = splitscreen ? 5 : 28;
	for (uint_fast16_t t3_x = 0; t3_x < 32; t3_x++) {
		for (uint_fast16_t t3_y = 0; t3_y < rows; t3_y++) {
			if (VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)] < MAX_L3_TILES) {
				draw8x8_tile_2bpp(t3_x << 3, (t3_y >= 17 ? int_res_y - 224 : (splitscreen * -8)) + (t3_y << 3), VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)], VRAM[0xB801 + (t3_x << 1) + (t3_y << 6)]);
			}
		}
	}


	if (drawDiag && !splitscreen)
	{
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_Rect rect;
		rect.x = 0; rect.w = 256;
		rect.y = 224 - 128; rect.h = 128;
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 0x3F);
		SDL_RenderFillRect(ren, &rect);

		int ping_c = ((abs(latest_server_response) % 3600) % 1000) / 3;
		int fps = int(1.0 / (total_time_ticks.count() / 1.0));
		fps_diag[127] = fps;
		ping_diag[111] = ping_c;
		block_diag[111] = blocks_on_screen;
		kbs_diag[111] = data_size_now / 512;

#if defined(_WIN32)
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)& pmc, sizeof(pmc));
		SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

		int ram_u = (int(virtualMemUsedByMe) / 1024) / 1024;
		ram_diag[15] = ram_u;
#endif

		if (!(global_frame_counter % 16))
		{
			memcpy(ram_diag, &ram_diag[1], 15 * sizeof(int));
		}
		memcpy(fps_diag, &fps_diag[1], 127 * sizeof(int));
		memcpy(kbs_diag, &kbs_diag[1], 111 * sizeof(int));
		memcpy(ping_diag, &ping_diag[1], 111 * sizeof(int));
		memcpy(block_diag, &block_diag[1], 111 * sizeof(int));


		for (uint_fast8_t l = 0; l < 128; l++)
		{
			int curr_t = fps_diag[l] / 10;

			rect.x = l; rect.w = 1;
			rect.y = 224 - curr_t; rect.h = curr_t;

			int g = min(255, curr_t * 4);
			int r = 255 - g;

			SDL_Rect rect;
			rect.x = l; rect.w = 1;
			rect.y = 224 - curr_t; rect.h = curr_t;
			SDL_SetRenderDrawColor(ren, r, g, 0, 0xBF);
			SDL_RenderFillRect(ren, &rect);
		}

		for (uint_fast8_t l = 0; l < 16; l++)
		{
			int curr_t = ram_diag[l];
			rect.x = 128 + l; rect.w = 1;
			rect.y = 224 - curr_t; rect.h = curr_t;

			SDL_SetRenderDrawColor(ren, 0xFF, 0x7F, 0x7F, 0xBF);
			SDL_RenderFillRect(ren, &rect);
		}

		for (uint_fast8_t l = 0; l < 112; l++)
		{
			int curr_t = block_diag[l] / 8;
			rect.x = 144 + l; rect.w = 1;
			rect.y = 224 - curr_t; rect.h = curr_t;
			SDL_SetRenderDrawColor(ren, 0x7F, 0x7F, 0xFF, 0xBF);
			SDL_RenderFillRect(ren, &rect);
		}
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	}


	//Draw screen darkening (Fades)
	if (transition_type != 3)
	{
		SDL_SetRenderDrawColor(ren, 0, 0, 0, bright_val == 0xF ? 255 : (bright_val << 4));
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(ren, nullptr);
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	}
}


void render()
{
	PrepareRendering();

	/*
		Convert 16bit palette to 32bit palette (for speed)
	*/
	ConvertPalette();
	memcpy(VRAM, &RAM[VRAM_Location], VRAM_Size * sizeof(uint_fast8_t));

	/*
		Overworld rendering
	*/
	if (in_Overworld)
	{
		overworld.Render();
		return;
	}

	if (splitscreen)
	{
		uint_fast16_t c = RAM[0x3D00] + (RAM[0x3E00] << 8);

		SDL_SetRenderTarget(ren, target_texture_p1);
		SDL_SetRenderDrawColor(ren, (c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, (c >> 10) << 3, 255);
		SDL_RenderClear(ren);
		handleRenderingForPlayer(1, int_res_x, int_res_y / 2);

		SDL_SetRenderTarget(ren, target_texture_p2);
		SDL_SetRenderDrawColor(ren, (c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, (c >> 10) << 3, 255);
		SDL_RenderClear(ren);
		handleRenderingForPlayer(2, int_res_x, int_res_y / 2);
	}
	else
	{
		handleRenderingForPlayer(SelfPlayerNumber, int_res_x, int_res_y);
	}
}