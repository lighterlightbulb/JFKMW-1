#pragma once
/*
	Sprite system, RAM values documented in RAM map.lua
*/
class sprite_system
{
public:
	double x, y;
	lua_State* SPR_STATE[128];

	void process_sprite_logic(uint_fast8_t entry = 0)
	{
		x = double(RAM[0x2100 + entry] + double(int_fast8_t(RAM[0x2180 + entry])) * 256.0) + double(RAM[0x2200 + entry]) / 256.0;
		y = double(RAM[0x2280 + entry] + double(int_fast8_t(RAM[0x2300 + entry])) * 256.0) + double(RAM[0x2380 + entry]) / 256.0;
		double x_size = double(RAM[0x2500 + entry]);
		double y_size = double(RAM[0x2580 + entry]);
		uint_fast16_t check_x_1 = uint_fast16_t((x + x_size / 2.0) / 16.0);
		uint_fast16_t check_y_1 = uint_fast16_t((y + y_size / 2.0) / 16.0);

		bool IN_WT = RAM[0x85] != 0 || map16_handler.get_tile(check_x_1, check_y_1) < 4;

		if (RAM[0x2600 + entry] & 0b100000) //if gravity bit is on
		{
			int_fast8_t sprgravity = IN_WT ? -16 : -82;

			if (IN_WT)
			{
				if (int_fast8_t(RAM[0x2480 + entry]) > 32)
				{
					RAM[0x2480 + entry] = 32;
				}
			}
			if (int_fast8_t(RAM[0x2480 + entry]) > sprgravity)
			{
				int grav;
				if (IN_WT)
				{
					grav = RAM[0x2880 + entry] & 0b1000 ? 0 : 1;
				}
				else
				{
					grav = RAM[0x2880 + entry] & 0b1000 ? 2 : 3;
				}

				RAM[0x2480 + entry] = max(sprgravity, RAM[0x2480 + entry] - grav);
			}
			else
			{
				RAM[0x2480 + entry] = sprgravity;
			}
		}

		if (RAM[0x2980 + entry])
		{
			RAM[0x2980 + entry]--;
		}
		double xMove = double(double(int_fast8_t(RAM[0x2400 + entry])) * 16) / ((IN_WT && (RAM[0x2600 + entry] & 0b100000)) ? 384 : 256);
		double yMove = double(double(int_fast8_t(RAM[0x2480 + entry])) * 16) / 256.0;

		if (RAM[0x2600 + entry] & 0b1000000)
		{
			for (uint_fast8_t spr = 0; spr < 0x80; spr++)
			{
				if (spr != entry && RAM[0x2000 + spr])
				{
					double t_x = double(RAM[0x2100 + spr] + double(RAM[0x2180 + spr]) * 256.0) + double(RAM[0x2200 + spr]) / 256.0;
					double t_y = double(RAM[0x2280 + spr] + double(RAM[0x2300 + spr]) * 256.0) + double(RAM[0x2380 + spr]) / 256.0;

					double t_x_size = double(RAM[0x2500 + spr]);
					double t_y_size = double(RAM[0x2580 + spr]);

					if (
						x > (t_x - x_size) &&
						x < (t_x + t_x_size) &&
						y > (t_y - y_size) &&
						y < (t_y + t_y_size)
						)
					{
						if (RAM[0x2000 + entry] == 4 || (RAM[0x2000 + entry] == 2 && (abs(int_fast8_t(RAM[0x2400 + entry])) > 3 || abs(int_fast8_t(RAM[0x2480 + entry])) > 3)))
						{
							if (RAM[0x2000 + spr] != 3)
							{
								//2 handlers just incase
								RAM[0x2B00 + spr] = 0x1;
								RAM[0x2700 + spr] = 0xFF;
							}
						}
						else
						{
							if (RAM[0x2000 + entry] < 3 && !(RAM[0x2880 + spr] & 0b00010000) && RAM[0x2600 + entry] & 0b1000000 && RAM[0x2600 + spr] & 0b1000000)
							{
								if (RAM[0x2000 + spr] == 1 && !RAM[0x2980 + spr])
								{
									RAM[0x2680 + spr] *= -1;
									RAM[0x2400 + spr] *= -1;

									RAM[0x2980 + spr] = 16;
									RAM[0x2980 + entry] = 16;
									RAM[0x2400 + entry] *= -1;
								}
							}
						}
					}

				}
			}
		}

		
		if (RAM[0x2600 + entry] & 0b1000000) //if solid bit is on
		{
			
			bool g = RAM[0x2000 + entry] == 2 || RAM[0x2000 + entry] == 4;
			if (!Move(xMove, 0.0, x_size, y_size, g, entry))
			{
				RAM[0x2780 + entry] ^= 0b00000001;
			}

			if (!Move(0.0, yMove, x_size, y_size, g, entry))
			{
				RAM[0x2780 + entry] ^= 0b00000010;

			}
		}
		else
		{
			RAM[0x2780 + entry] = 0;
			x += xMove;
			y += yMove;
		}


		if (y < (-y_size-16))
		{
			for (int i = 0; i < 32; i++)
			{
				RAM[0x2000 + entry + (i * 128)] = 0;
			}
			return;
		}

		RAM[0x2100 + entry] = uint_fast8_t(int(x));
		RAM[0x2180 + entry] = uint_fast8_t(int(x) / 256);
		RAM[0x2200 + entry] = uint_fast8_t(int(x*256.0));
		RAM[0x2280 + entry] = uint_fast8_t(int(y));
		RAM[0x2300 + entry] = uint_fast8_t(int(y) / 256); 
		RAM[0x2380 + entry] = uint_fast8_t(int(y*256.0));
		if (x < 0)
		{
			RAM[0x2180 + entry] -= 1;
		}
		if (y < 0)
		{
			RAM[0x2300 + entry] -= 1;
		}
	}

	/*
		Shitty Movement Code
	*/
	bool Move(double xMove, double yMove, double x_size, double y_size, bool kickedgrabbed, uint_fast8_t entry)
	{
		bool finna_return = true;
		double NewPositionX = x + xMove;
		double NewPositionY = y + yMove;

		int_fast16_t PosXBlock = int_fast16_t(NewPositionX / 16); 
		int_fast16_t PosYBlock = int_fast16_t(NewPositionY / 16);

		int_fast8_t TotalBlocksCollisionCheckSpriteX = int_fast8_t(x_size / 16.0) + 2;
		int_fast8_t TotalBlocksCollisionCheckSpriteY = int_fast8_t(y_size / 16.0) + 2;

		int_fast16_t startX = max(0, PosXBlock - 1);
		int_fast16_t startY = max(0, PosYBlock - 1);

		for (int_fast16_t xB = startX; xB < PosXBlock + TotalBlocksCollisionCheckSpriteX; xB++)
		{
			for (int_fast16_t yB = startY; yB < PosYBlock + TotalBlocksCollisionCheckSpriteY; yB++)
			{
				map16_handler.update_map_tile(xB, yB);
				double f_h = map16_handler.ground_y(NewPositionX + (x_size / 2.0) - (xB * 16), xB, yB);
				double BelowBlock = double(yB * 16) + (f_h - 16.0) - y_size;
				double AboveBlock = double(yB * 16) + f_h;
				double RightBlock = double(xB * 16) + 16.0;
				double LeftBlock = double(xB * 16) - x_size;

				bool checkLeft = map16_handler.logic[3];
				bool checkRight = map16_handler.logic[2];
				bool checkBottom = map16_handler.logic[1];
				bool checkTop = map16_handler.logic[0];

				if (NewPositionX < RightBlock && NewPositionX > LeftBlock && NewPositionY < AboveBlock && NewPositionY > BelowBlock)
				{

					if (xMove < 0.0 && checkRight)
					{
						if (NewPositionX < RightBlock && NewPositionX > RightBlock - bounds_x)
						{
							NewPositionX = RightBlock;
							finna_return = false;

							RAM[0x2780 + entry] ^= 0b00000100;

							if (kickedgrabbed)
							{
								map16_handler.process_block(xB, yB, bottom);
							}
						}
					}
					if (xMove > 0.0 && checkLeft)
					{
						if (NewPositionX > LeftBlock && NewPositionX < LeftBlock + bounds_x)
						{
							NewPositionX = LeftBlock;
							finna_return = false;

							RAM[0x2780 + entry] ^= 0b00001000;

							if (kickedgrabbed)
							{
								map16_handler.process_block(xB, yB, bottom);
							}
						}
					}
					if (yMove < 0.0 && checkTop)
					{
						double bound_y = bounds_y;
						uint_fast8_t new_s = map16_handler.get_slope(xB, yB);
						if (new_s != 0)
						{
							bound_y += 8;
						}

						RAM[0x2780 + entry] ^= 0b00010000;

						if (NewPositionY < AboveBlock && NewPositionY > (AboveBlock - bound_y))
						{
							NewPositionY = AboveBlock;
							finna_return = false;
						}

					}
					if (yMove > 0.0 && checkBottom)
					{
						if (NewPositionY > BelowBlock && NewPositionY < BelowBlock + bounds_y)
						{
							NewPositionY = BelowBlock;
							finna_return = false;

							RAM[0x2780 + entry] ^= 0b00100000;

							if (kickedgrabbed)
							{
								map16_handler.process_block(xB, yB, bottom);
							}
						}
					}

				}
			}
		}

		x = NewPositionX;
		y = NewPositionY;
		return finna_return;
	}

	void call_sprite_lua(int index)
	{
		lua_getglobal(SPR_STATE[index], "Main");
		lua_pushinteger(SPR_STATE[index], index);
		lua_pcall(SPR_STATE[index], 1, 0, 0); // run script
	}

	void init_sprite_lua(int index, int type)
	{
		if (SPR_STATE[index])
		{
			lua_close(SPR_STATE[index]);
		}

		SPR_STATE[index] = luaL_newstate();

		load_lua_libs(SPR_STATE[index]);


		int ret = luaL_dostring(SPR_STATE[index], SPR_CODE[type].c_str());
		if (ret != 0)
		{
			lua_print("Error: " + string(lua_tostring(SPR_STATE[index], -1)));
			RAM[0x2000 + index] = 0;
			return;
		}

		lua_connect_functions(SPR_STATE[index]);
		lua_getglobal(SPR_STATE[index], "Init");
		lua_pushinteger(SPR_STATE[index], index);
		lua_pcall(SPR_STATE[index], 1, 0, 0); // run script
	}

	void process_all_sprites()
	{
		for (uint_fast8_t i = 0; i < 128; i++)
		{
			
			if (RAM[0x2000 + i] != 0) //If sprite exists..
			{
				if (RAM[0x2A80 + i] & 4 && !(RAM[0x2A80 + i] & 2))
				{
					RAM[0x2000 + i] = 0;
					continue;
				}
				if (!(RAM[0x2A80 + i] & 1) && !(RAM[0x2A80 + i] & 2)) //Don't process offscreen, useless.
				{
					continue;
				}
				if (RAM[0x2F80 + i] == 0)
				{
					if (RAM[0x2800 + i])
					{
						init_sprite_lua(int(i), RAM[0x2080 + i]);
					}
					RAM[0x2F80 + i] = 1;
				}
				else
				{
					process_sprite_logic(uint_fast8_t(i));

					if (RAM[0x2000 + i] == 0)
					{
						continue;
					}
				}
				if (RAM[0x2800 + i])
				{
					call_sprite_lua(i);
				}
				else
				{
					ASM.load_asm("Code/Sprites/" + to_string(RAM[0x2080 + i]) + ".oasm");
					ASM.x = i;
					ASM.start_JFK_thread();
					if (ASM.crashed)
					{
						RAM[0x2000 + i] = 0;
					}

				
				}
				
			}
		}
	}
};


sprite_system Sprites;