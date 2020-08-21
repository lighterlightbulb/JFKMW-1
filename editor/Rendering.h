#pragma once

int DRAW_SIZE_X = 48;
int DRAW_SIZE_Y = 32;

int camera_x = 1;
int camera_y = 1;
int picked_tile = 0x25;
int picked_sprite = -1;
int mouse_x, mouse_y;
int block_size = 0; //Placement size
int updates = 0;
bool snap = false;
double stick_x = 0.0;
double stick_y = 0.0;
bool mouse_l = false;
bool mouse_r = false;
bool mouse_p = false;
bool mouse_l_old = false;

bool input_down = false;
bool input_up = false;
bool input_left = false;
bool input_right = false;
bool input_del = false;

int just_dropped = 0;


class button
{
public:
	int x = 0;
	int b = 0;
	bool pressed = false;
	bool pressed_old = false;
	bool pressed_look = false;

	void* onButtonDown;
	void* onButtonUp;
};

button buttons[9];

#define color_FF 0xFF403030*2
#define color_F0 0xFF2B1A1A*2
#define color_E3 0xFF281717*2
#define color_A0 0xFF241414*2
#define color_69 0xFF000000
#define color_BG 0xFF3D2525

void FillRect(SDL_Rect rect, uint_fast32_t c)
{
	SDL_SetRenderDrawColor(ren, c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF);
	SDL_RenderFillRect(ren, &rect);
}

void draw_button(int x, int b, bool pressed)
{
	SDL_Rect rect;
	rect.x = x; rect.w = 23;
	rect.y = 6; rect.h = 22;
	FillRect(rect, pressed ? color_FF : color_69);

	rect.x = x; rect.w = 22;
	rect.y = 6; rect.h = 21;
	FillRect(rect, pressed ? color_69 : color_FF);

	rect.x = x + 1; rect.w = 21;
	rect.y = 7; rect.h = 20;
	FillRect(rect, pressed ? color_E3 : color_A0);

	rect.x = x + 1; rect.w = 20;
	rect.y = 7; rect.h = 19;
	FillRect(rect, pressed ? color_A0 : color_E3);


	SDL_Rect SRC;
	SRC.x = 256 + b * 16; SRC.y = 32;
	SRC.w = 16; SRC.h = 16;

	rect.x = 3 + x; rect.y = 9;
	rect.w = 16; rect.h = 16;

	SDL_RenderCopy(ren, MAP16_TEX, &SRC, &rect);
}

void draw_to_screen()
{
	DRAW_SIZE_X = p_w / 16;
	DRAW_SIZE_Y = (p_h - 32) / 16;
	DRAW_SIZE_Y++;

	SDL_Rect rect;
	SDL_Rect SRC;

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);


	if (current_file != "")
	{
		if (BG_TEX)
		{
			int bgs_x = (p_w / 512) + 2;
			int bgs_y = (p_h / 512) + 2;

			for (int b_x = 0; b_x < bgs_x; b_x++)
			{
				for (int b_y = 1; b_y <= bgs_y; b_y++)
				{
					rect.x = -(((camera_x - 1) * 16) & 0x1FF) + b_x * 512;
					rect.y = (((camera_y - 1) * 16) & 0x1FF) + p_h - (b_y * 512);
					rect.w = 512;
					rect.h = 512;
					SDL_RenderCopy(ren, BG_TEX, nullptr, &rect);
				}
			}
		}

		rect.x = p_w - 256 - (p_w % 16); rect.w = 256 + (p_w % 16);
		rect.y = 0; rect.h = p_h;
		FillRect(rect, 0xFF000000);

		for (int x = 0; x < DRAW_SIZE_X; x++) {
			for (int y = 0; y < DRAW_SIZE_Y; y++) {
				int spx = x << 4;
				int spy = p_h - 48 - (y << 4);

				int p_x = x - 1 + camera_x;
				int p_y = y - 1 + camera_y;

				uint_fast16_t map16tile = 0x25;
				if (x < (DRAW_SIZE_X-16))
				{
					if (p_y < L_SIZE_Y && p_x < L_SIZE_X)
					{
						map16tile = leveldata[p_x + p_y * L_SIZE_X];
					}
				}
				else
				{
					if (y < 32)
					{
						int tx = (x - ((p_w / 16) - 16)) % 16;
						map16tile = (tx) + y * 16;

						spy = (y << 4);
						spx = (p_w - 256 + (tx * 16));
					}
				}

				uint_fast16_t map16real = map16tile;
				//Shit animation code here
				if (map16tile == 0x124 || map16tile == 0x11F)
				{
					map16tile = 0x300 + ((frm_counter / 8) % 4);
				}
				if (map16tile == 0x2B)
				{
					map16tile = 0x310 + ((frm_counter / 8) % 4);
				}
				if (map16tile == 0x12F)
				{
					map16tile = 0x320 + ((frm_counter / 8) % 2);
				}
				if (map16tile == 0x00)
				{
					map16tile = 0x330 + ((frm_counter / 8) % 4);
				}

				if (map16real != 0x25)
				{
					updates += 1;
					uint_fast16_t map16_x = ((map16tile & 0xF) << 4) + (map16tile / 0x200) * 0x100;
					uint_fast16_t map16_y = ((map16tile % 0x200) >> 4) << 4;

					rect.x = spx; rect.y = spy + 32;
					rect.w = 16; rect.h = 16;

					SRC.x = map16_x; SRC.y = map16_y;
					SRC.w = 16; SRC.h = 16;

					if (x >= (DRAW_SIZE_X - 16) && map16real == picked_tile)
					{
						SRC.x += 512;
					}
					SDL_RenderCopy(ren, MAP16_TEX, &SRC, &rect);
				}
			}
		}

		for (int i = 0; i < 512; i++)
		{
			if (Sprite_data[i].exists)
			{
				if (Sprite_data[i].num > 255 || Sprite_data[i].num < 0)
				{
					//cout << "Sprite slot " << i << " is corrupted! Deleting.." << endl;
					Sprite_data[i].exists = false;
					continue;
				}

				int x_pos = 16 + Sprite_data[i].x - camera_x * 16;
				int y_pos = (p_h - 32) - Sprite_data[i].y + camera_y * 16;

				if (x_pos < (p_w - 256) && x_pos > 0)
				{

					rect.x = x_pos; rect.y = y_pos;
					rect.w = 16; rect.h = 16;

					SRC.x = 256 + (Sprite_data[i].type ? 16 : 0);
					SRC.y = (i == picked_sprite ? 16 : 0);
					SRC.w = 16; SRC.h = 16;

					SDL_RenderCopy(ren, MAP16_TEX, &SRC, &rect);
					draw_string((Sprite_data[i].type ? "Lua" : "ASM"), x_pos, y_pos);
					draw_string(int_to_hex(Sprite_data[i].num, true), x_pos, y_pos + 5);
					draw_string(to_string(Sprite_data[i].dir), x_pos, y_pos + 10);
				}
			}
		}


		if ((mouse_y >= 0 && picked_sprite < 0) && mouse_x < (p_w - 256))
		{
			int x = (mouse_x >> 4) << 4;
			int y = (((mouse_y - (p_h % 16)) >> 4) << 4) + (p_h % 16);

			rect.x = x - 48; rect.y = y - 16;
			rect.w = 112; rect.h = 112;

			SRC.x = 400; SRC.y = 112 * block_size;
			SRC.w = 112; SRC.h = 112;
			SDL_RenderCopy(ren, MAP16_TEX, &SRC, &rect);
		}


		int s_x = start_x - camera_x + 1;
		int s_y = 31 - (start_y - camera_y + 1);

		if ((!snap && keyboard_or_controller) && (mouse_x < 512))
		{
			SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
			SDL_RenderDrawPoint(ren, mouse_x, mouse_y);
		}
		if (s_x >= 0 && s_y >= 0 && s_x < 32 && s_y < 32)
		{

			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_ADD);
			rect.x = s_x << 4; rect.w = 16;
			rect.y = 32 + (s_y << 4); rect.h = 1;
			FillRect(rect, 0xFF00FFFF);

			rect.x = s_x << 4; rect.w = 1;
			rect.y = 32 + (s_y << 4); rect.h = 16;
			FillRect(rect, 0xFF00FFFF);

			rect.x = s_x << 4; rect.w = 16;
			rect.y = 47 + (s_y << 4); rect.h = 1;
			FillRect(rect, 0xFF00FFFF);

			rect.x = 15 + (s_x << 4); rect.w = 1;
			rect.y = 32 + (s_y << 4); rect.h = 16;
			FillRect(rect, 0xFF00FFFF);
			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
		}

		string display1 = "Mouse: " + to_string(mouse_x) + ", " + to_string(mouse_y) + " Camera: " + to_string(camera_x) + ", " + to_string(camera_y);
		string display2 = "FPS " + to_string(int(fps));

		draw_string(display1, 0, 0 + 32);
		draw_string(display2, 0, 5 + 32);

	}
	else
	{
		rect.x = 0; rect.w = p_w;
		rect.y = 0; rect.h = p_h;
		FillRect(rect, color_BG);
	}
	/*
		Draw toolbar
	*/

	rect.x = 0; rect.w = p_w;
	rect.y = 0; rect.h = 32;
	FillRect(rect, color_F0);

	rect.x = 0; rect.w = p_w;
	rect.y = 2; rect.h = 1;
	FillRect(rect, color_A0);

	rect.x = 0; rect.w = p_w;
	rect.y = 3; rect.h = 1;
	FillRect(rect, color_FF);

	rect.x = 0; rect.w = p_w;
	rect.y = 30; rect.h = 1;
	FillRect(rect, color_A0);

	rect.x = 0; rect.w = p_w;
	rect.y = 31; rect.h = 1;
	FillRect(rect, color_69);

	/*
	
	Draw toolbar buttons

	*/

	for (int i = 0; i < size(buttons); i++)
	{
		draw_button(buttons[i].x, buttons[i].b, buttons[i].pressed_look);
	}

}