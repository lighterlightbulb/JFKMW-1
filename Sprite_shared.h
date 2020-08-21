#pragma once


string SPR_CODE[256]; //Cache

void preloadSpriteCache()
{
	for (int i = 0; i < 256; i++) {
		SPR_CODE[i] = "";
	}

	int sprs = 0;
	for (int i = 0; i < 256; i++)
	{
		ifstream SprCode(path + "Code/Sprites/" + int_to_hex(i, true) + ".lua");
		if (SprCode.is_open()) {
			std::stringstream buffer;
			buffer << SprCode.rdbuf();
			SPR_CODE[i] = buffer.str();
			sprs++;
		}
		SprCode.close();
	}
	cout << lua_color << "[Lua] Preloaded " << sprs << " sprites." << white << endl;
}

void CreateNewSpr(uint_fast8_t i, uint_fast8_t sprite_num, uint_fast8_t new_state, uint_fast16_t x, uint_fast16_t y, uint_fast8_t direction, bool is_lua)
{
	RAM[0x2000 + i] = new_state;
	RAM[0x2080 + i] = sprite_num;
	RAM[0x2F80 + i] = 0;

	RAM[0x2100 + i] = uint_fast8_t(x & 0xFF);
	RAM[0x2180 + i] = uint_fast8_t(x >> 8);
	RAM[0x2200 + i] = 0;

	RAM[0x2280 + i] = uint_fast8_t(y & 0xFF);
	RAM[0x2300 + i] = uint_fast8_t(y >> 8);
	RAM[0x2380 + i] = 0;
	RAM[0x2600 + i] = 0;
	RAM[0x2400 + i] = 0;
	RAM[0x2480 + i] = 0;

	RAM[0x2700 + i] = 0;
	RAM[0x2780 + i] = 0;

	RAM[0x2680 + i] = direction;

	RAM[0x2700 + i] = 0;

	RAM[0x2E00 + i] = 0x01;
	RAM[0x2F00 + i] = 0x00;
	RAM[0x2A00 + i] = 0x00;
	RAM[0x2A80 + i] = 0x02;
	RAM[0x2B00 + i] = 0;


	RAM[0x2800 + i] = is_lua;
	RAM[0x2880 + i] = 0;
}

uint_fast8_t spawnSpriteJFKMarioWorld(uint_fast8_t sprite_num, uint_fast8_t new_state, uint_fast16_t x, uint_fast16_t y, uint_fast8_t direction, bool is_lua)
{
	for (uint_fast8_t i = 0; i < 128; i++)
	{
		if (RAM[0x2000 + i] == 0)
		{
			CreateNewSpr(i, sprite_num, new_state, x, y, direction, is_lua);
			return i;
		}
	}
	for (uint_fast8_t i = 0; i < 128; i++)
	{
		if (RAM[0x2A80 + i] & 2 && !(RAM[0x2A80 + i] & 1))
		{
			CreateNewSpr(i, sprite_num, new_state, x, y, direction, is_lua);
			return i;
		}
	}
	return 0xFF;
}