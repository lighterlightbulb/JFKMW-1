#pragma once

/*
lua integration for jfk mario world
*/

// Lua is written in C, so compiler needs to know how to link its libraries

bool lua_loaded = false;

extern "C" {
#include "lua.hpp"
}

lua_State* LUA_STATE;

void lua_print(string text)
{
	cout << lua_color << "[Lua] " << text << white << endl;
}

void load_lua_libs(lua_State* L)
{
	luaL_openlibs(L);
}

//lua functions to bind to jfk mario world.
static int lua_write(lua_State* L) {
	string str = (string)lua_tostring(L, 1); // get function argument
	//cout << "Print called" << endl;
	lua_print(str);
	return 0; // nothing to return!
}

static int lua_write_ram(lua_State* L) {
	uint_fast32_t n = (uint_fast32_t)lua_tonumber(L, 1);
	uint_fast32_t p = (uint_fast16_t)lua_tointeger(L, 2);
	uint_fast8_t s = (uint_fast8_t)lua_tointeger(L, 3);

	ASM.Write_To_Ram(p, n, s);
	return 0; // nothing to return!
}


extern "C" int lua_get_ram(lua_State* L)
{
	uint_fast32_t p = (uint_fast16_t)lua_tointeger(L, 1); // First argument
	uint_fast8_t s = (uint_fast8_t)lua_tointeger(L, 2); // First argument

	double result = double(int_fast32_t(ASM.Get_Ram(p, s)));

	lua_pushnumber(L, result);
	//lua_pushinteger(L, result);

	return 1; // Count of returned values
}

static int lua_spawn_sprite(lua_State* L) {
	uint_fast8_t n = (uint_fast8_t)lua_tointeger(L, 1); // First argument
	uint_fast8_t s = (uint_fast8_t)lua_tointeger(L, 2); // First argument
	uint_fast16_t x = (uint_fast16_t)lua_tointeger(L, 3); // First argument
	uint_fast16_t y = (uint_fast16_t)lua_tointeger(L, 4); // First argument
	uint_fast8_t direction = (uint_fast8_t)lua_tointeger(L, 5); // First argument
	bool is_l = lua_toboolean(L, 6);

	lua_pushnumber(L, spawnSpriteJFKMarioWorld(n, s, x, y, direction, is_l));


	return 1;
}

static int draw_to_oam(lua_State* L)
{
	uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
	uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 2);
	uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 3);
	uint_fast8_t angle = (uint_fast8_t)lua_tonumber(L, 4);
	int offset_x = (int)lua_tonumber(L, 5);
	int offset_y = (int)lua_tonumber(L, 6);

	uint_fast8_t pal = (uint_fast8_t)lua_tonumber(L, 7);

	if (!(RAM[0x2A80 + sprite_index] & 2))
	{
		return 0;
	}
	uint_fast16_t oam_index = 0;
	while (oam_index < 0x400)
	{
		if (RAM[0x200 + oam_index] == 0 && RAM[0x206 + oam_index] == 0) //Founmd a empty OAM slot
		{
			break;
		}
		oam_index += 8;
	}


	RAM[0x200 + oam_index] = tile;
	RAM[0x201 + oam_index] = size;


	uint_fast16_t sprite_x_position = uint_fast16_t(int(offset_x + RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256));
	uint_fast16_t sprite_y_position = uint_fast16_t(int(offset_y + RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256));

	RAM[0x202 + oam_index] = sprite_x_position;
	RAM[0x203 + oam_index] = sprite_x_position >> 8;

	RAM[0x204 + oam_index] = sprite_y_position;
	RAM[0x205 + oam_index] = sprite_y_position >> 8;

	RAM[0x206 + oam_index] = pal;
	RAM[0x207 + oam_index] = angle;

	return 0;
}

static int slideDeathHandler(lua_State* L)
{
	uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
	uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 2);
	uint_fast8_t flags = (uint_fast8_t)lua_tonumber(L, 3);
	int offset_x = (int)lua_tonumber(L, 4);
	int offset_y = (int)lua_tonumber(L, 5);

	uint_fast16_t sprite_x_position = uint_fast16_t(int(RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256)) + offset_x;
	uint_fast16_t sprite_y_position = uint_fast16_t(int(RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256)) + offset_y;

	double sy = Calculate_Speed(512);
	double sx = -int_fast8_t(RAM[0x2680 + sprite_index]);

	
	createParticle(tile, 0x11, flags, 0xFF, sprite_x_position, sprite_y_position, sx, sy, Calculate_Speed(48), 0, 0, -4);

	//Hitspark
	createParticle(0x60, 0x11, 0x88, 5, sprite_x_position, sprite_y_position, 0, 0, 0);

	return 0;
}

static int spriteDeathParticle(lua_State* L)
{
	uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
	uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 2);
	uint_fast8_t flags = (uint_fast8_t)lua_tonumber(L, 3);
	uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 4);
	int offset_x = (int)lua_tonumber(L, 5);
	int offset_y = (int)lua_tonumber(L, 6);

	uint_fast16_t sprite_x_position = uint_fast16_t(int(RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256)) + offset_x;
	uint_fast16_t sprite_y_position = uint_fast16_t(int(RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256)) + offset_y;

	createParticle(tile, 0x11, flags, 0xFF, sprite_x_position, sprite_y_position, 0, 0, Calculate_Speed(48), 0, 0, -4);

	return 0;
}

static int draw_to_oam_direct(lua_State* L)
{
	uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 1);
	uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 2);
	uint_fast8_t angle = (uint_fast8_t)lua_tonumber(L, 3);
	uint_fast16_t sprite_x_position = (uint_fast16_t)lua_tonumber(L, 4);
	uint_fast16_t sprite_y_position = (uint_fast16_t)lua_tonumber(L, 5);
	uint_fast8_t pal = (uint_fast8_t)lua_tonumber(L, 6);
	uint_fast16_t oam_index = 0;
	while (oam_index < 0x400)
	{
		if (RAM[0x200 + oam_index] == 0 && RAM[0x206 + oam_index] == 0) //Founmd a empty OAM slot
		{
			break;
		}
		oam_index += 8;
	}

	RAM[0x200 + oam_index] = tile;
	RAM[0x201 + oam_index] = size;

	RAM[0x202 + oam_index] = sprite_x_position;
	RAM[0x203 + oam_index] = sprite_x_position >> 8;

	RAM[0x204 + oam_index] = sprite_y_position;
	RAM[0x205 + oam_index] = sprite_y_position >> 8;

	RAM[0x206 + oam_index] = pal;
	RAM[0x207 + oam_index] = angle;

	return 0;
}



static int lua_bitand(lua_State* L) {
	uint_fast8_t a = (uint_fast8_t)lua_tointeger(L, 1); 
	uint_fast8_t b = (uint_fast8_t)lua_tointeger(L, 2);
	
	uint_fast8_t result = 0;
	uint_fast8_t bitval = 1;
	while (a > 0 && b > 0)
	{
		if ((a & 1) == 1 && (b & 1) == 1)
		{
			result = result + bitval;
		}
		bitval = bitval << 1;
		a = a >> 1;
		b = b >> 1;
	}
	lua_pushnumber(L, result);
	return 1;
}



static int lua_bitand_new(lua_State* L) {
	uint_fast8_t a = (uint_fast8_t)lua_tointeger(L, 1);
	uint_fast8_t b = (uint_fast8_t)lua_tointeger(L, 2);

	uint_fast8_t result = a & b;
	lua_pushnumber(L, result);
	return 1;
}

static int drawtohud(lua_State* L)
{
	uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 1) & 0x7F;
	uint_fast8_t prop = (uint_fast8_t)lua_tonumber(L, 2) & 0x0F;
	uint_fast8_t x = (uint_fast8_t)lua_tonumber(L, 3) & 0x1F;
	uint_fast8_t y = (uint_fast8_t)lua_tonumber(L, 4) & 0x1F;

	RAM[0x1B800 + ((x % 32) + (y * 32))*2] = tile;
	RAM[0x1B801 + ((x % 32) + (y * 32))*2] = prop;
	return 0;
}

static int killPlayer(lua_State* L)
{
	int plr = (int)lua_tointeger(L, 1);
	death_timer[plr - 1] = 16;
	return 0;
}

static int damagePlayer(lua_State* L)
{
	int plr = (int)lua_tointeger(L, 1);
	death_timer[plr - 1] = 0x80 + 16;
	return 0;
}

static int createParticleHook(lua_State* L)
{
	uint_fast8_t tile = (uint_fast8_t)lua_tointeger(L, 1);
	uint_fast8_t size = (uint_fast8_t)lua_tointeger(L, 2);
	uint_fast8_t prop = (uint_fast8_t)lua_tointeger(L, 3);
	uint_fast8_t anim_type = (uint_fast8_t)lua_tointeger(L, 4);
	double x = (double)lua_tonumber(L, 5);
	double y = (double)lua_tonumber(L, 6);
	double sx = (double)lua_tonumber(L, 7);
	double sy = (double)lua_tonumber(L, 8);
	double grav = (double)lua_tonumber(L, 9);
	int start_time = (int)lua_tointeger(L, 10);
	int delete_time = (int)lua_tointeger(L, 11);

	createParticle(tile, size, prop, anim_type, x, y, sx, sy, grav, start_time, delete_time);

	return 0;
}

static int discordMessageHook(lua_State* L)
{
	if (!networking) { return 0; }
	string text = (string)lua_tostring(L, 1);
	discord_message(text);
	return 0;
}

extern "C" {
int getPlayerX(lua_State* L)
{
	int plr = (int)lua_tointeger(L, 1); plr--;
	int result = RAM[0x5000 + plr] + RAM[0x5100 + plr] * 256;
	lua_pushnumber(L, result);
	return 1;
}

int getPlayerY(lua_State* L)
{
	int plr = (int)lua_tointeger(L, 1); plr--;
	int result = RAM[0x5200 + plr] + RAM[0x5300 + plr] * 256;
	lua_pushnumber(L, result);
	return 1;
}

int getSpriteX(lua_State* L)
{
	uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);

	uint_fast16_t sprite_x_position = uint_fast16_t(int(RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256));

	lua_pushinteger(L, sprite_x_position);
	return 1;
}

int getSpriteY(lua_State* L)
{
	uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
	uint_fast16_t sprite_y_position = uint_fast16_t(int(RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256) - 16);

	lua_pushinteger(L, sprite_y_position);
	return 1;
}



int lua_checkbit(lua_State* L)
{
	uint_fast32_t p = (uint_fast16_t)lua_tointeger(L, 1);
	uint_fast8_t s = (uint_fast8_t)lua_tointeger(L, 2);

	bool bit = ((RAM[p] >> s) & 1);

	//cout << "Bit " << int(s) << " of 0x" << hex << int(p) << " = " << int(bit) << endl;

	lua_pushboolean(L, bit);
	return 1;
}

int lua_chartosmw(lua_State* L)
{
	uint_fast8_t p = (uint_fast8_t)lua_tointeger(L, 1); // First argument
	lua_pushinteger(L, char_to_smw(p));
	return 1; // Count of returned values
}
}

/* functions end */

void lua_connect_functions(lua_State* L)
{
	//lua_print("Connected functions to 0x" + int_to_;

	lua_pushcfunction(L, lua_write); lua_setglobal(L, "marioPrint");
	lua_pushcfunction(L, lua_write_ram); lua_setglobal(L, "asmWrite");
	lua_pushcfunction(L, lua_spawn_sprite); lua_setglobal(L, "spawnSprite");
	lua_pushcfunction(L, draw_to_oam); lua_setglobal(L, "drawOam");
	lua_pushcfunction(L, draw_to_oam_direct); lua_setglobal(L, "drawOamDirect");
	lua_pushcfunction(L, lua_bitand); lua_setglobal(L, "oldBitand");
	lua_pushcfunction(L, lua_bitand_new); lua_setglobal(L, "bitand");
	lua_pushcfunction(L, drawtohud); lua_setglobal(L, "drawToHud");
	lua_pushcfunction(L, killPlayer); lua_setglobal(L, "killPlayer");
	lua_pushcfunction(L, damagePlayer); lua_setglobal(L, "damagePlayer");
	lua_pushcfunction(L, createParticleHook); lua_setglobal(L, "createParticle");
	lua_pushcfunction(L, slideDeathHandler); lua_setglobal(L, "deathBySlide");
	lua_pushcfunction(L, spriteDeathParticle); lua_setglobal(L, "deathByJump");
	lua_pushcfunction(L, discordMessageHook); lua_setglobal(L, "discordMessage");
	lua_register(L, "asmRead", lua_get_ram);
	lua_register(L, "charToSmw", lua_chartosmw);
	lua_register(L, "getPlayerX", getPlayerX);
	lua_register(L, "getPlayerY", getPlayerY);
	lua_register(L, "getSpriteX", getSpriteX);
	lua_register(L, "getSpriteY", getSpriteY);
	lua_register(L, "asmCheckBit", lua_checkbit);

}



/*
	LUA General
*/

string last_lua_file;
void lua_loadfile(string file)
{
	if (LUA_STATE)
	{
		lua_close(LUA_STATE);
	}

	last_lua_file = file;
	LUA_STATE = luaL_newstate();

	load_lua_libs(LUA_STATE);


	int ret = luaL_dofile(LUA_STATE, (path + file).c_str());
	if (ret != 0)
	{
		lua_print("Error occurred when calling luaL_dofile()");
		lua_print("Error: " + string(lua_tostring(LUA_STATE, -1)));
		lua_close(LUA_STATE);
		return;
	}



	//main connectors back to jfk mario world.
	lua_connect_functions(LUA_STATE);

	lua_print("loaded " + path + file);
}

void lua_run_init()
{
	lua_getglobal(LUA_STATE, "Init");
	lua_pcall(LUA_STATE, 0, 0, 0); // run script

	lua_getglobal(LUA_STATE, "Main");
	lua_pcall(LUA_STATE, 0, 0, 0); // run script
}

void lua_run_main()
{
	lua_getglobal(LUA_STATE, "Main");
	lua_pcall(LUA_STATE, 0, 0, 0); // run script
}

void lua_on_chatted(string message, int plr = 0)
{
	if (in_Overworld)
	{
		return;
	}
	if (lua_getglobal(LUA_STATE, "OnChatted"))
	{
		lua_pushstring(LUA_STATE, message.c_str());
		lua_pushinteger(LUA_STATE, plr);
		lua_pcall(LUA_STATE, 2, 0, 0);
	}
}
