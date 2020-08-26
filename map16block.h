#pragma once
#define tile_1 0
#define tile_2 2
#define tile_3 4
#define tile_4 6
#define tile_palette_1 8
#define tile_palette_2 9
#define tile_flips 10
#define collision 11
#define ram_level_low 0x8000
#define ram_level_high 0xC000
#define BOUNCE_TIME 6

uint_fast8_t map16_entries[0x2000];
uint_fast8_t spawned_grabbable = 0xFF;


void initialize_map16()
{
	string file = path + "Map16/Global.Cmap16";

	cout << blue << "[MAP16] Loading " << file << white << endl;
	ifstream input(file, ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});


	uint8_t temp[16];
	int current_byte = 0;
	for (auto &v : buffer)
	{
		temp[current_byte] = uint8_t(v);
		current_byte += 1;
		if (current_byte >= 16) {
			uint16_t replace_p = temp[1] + temp[0] * 256; //this is actually a thing.
			for (int i = 0; i < 14; i++)
			{
				map16_entries[(replace_p << 4) + i] = temp[i + 2];
			}
			int integer = map16_entries[collision + (replace_p << 4)];
			current_byte = 0;
			
		}
	}
	input.close();
}

class block_timer
{
public:
	uint_fast16_t set_to = 0x025;
	uint_fast16_t x = 0;
	uint_fast16_t y = 0;
	uint_fast16_t time = 0xFF;
	

	bool has_spr = false;
	uint_fast8_t spr_tile = 0;
	uint_fast8_t pal_props = 0;
	double spr_x = 0;
	double spr_y = 0;
	double spr_sx = 0;
	double spr_sy = 0;

	void draw()
	{
		spr_sy -= Calculate_Speed(256);
		spr_x += spr_sx;
		spr_y += spr_sy;

		uint_fast16_t oam_index = 0;
		uint_fast16_t s_x = uint_fast16_t(spr_x);
		uint_fast16_t s_y = uint_fast16_t(spr_y);
		while (oam_index < 0x400)
		{
			if (RAM[0x200 + oam_index] == 0 && RAM[0x206 + oam_index] == 0) { //Empty OAM slot found
				break;
			}
			oam_index += 8;
		}
		RAM[0x200 + oam_index] = spr_tile;
		RAM[0x201 + oam_index] = 0x11;
		RAM[0x202 + oam_index] = s_x;
		RAM[0x203 + oam_index] = s_x >> 8;
		RAM[0x204 + oam_index] = s_y;
		RAM[0x205 + oam_index] = s_y >> 8;
		RAM[0x206 + oam_index] = pal_props;
		RAM[0x207 + oam_index] = 0;
	}
};
vector<block_timer> blocks_processing;


class map16blockhandler //Map16 loaded block
{
public:
	uint_fast16_t tile;
	bool logic[8];
	vector<uint_fast32_t> blocks_d;

	void get_map_16_details()
	{
		uint_fast8_t integer = map16_entries[(tile << 4) + collision];
		logic[0] = integer & 0b10000000;
		logic[1] = integer & 0b01000000;
		logic[2] = integer & 0b00100000;
		logic[3] = integer & 0b00010000;
		logic[4] = integer & 0b00001000;
		logic[5] = integer & 0b00000100;
		logic[6] = integer & 0b00000010;
		logic[7] = integer & 0b00000001;



		if (tile >= 0x166 && tile <= 0x167)
		{
			bool solid = tile == 0x166 ? (!RAM[0x14AF]) : (RAM[0x14AF]);
			logic[0] = solid; logic[1] = solid; logic[2] = solid; logic[3] = solid;
		}
	}

	/*
		Update Map Tile
	*/
	void update_map_tile(uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast32_t index = (x % mapWidth) + (y % mapHeight) * mapWidth;
		tile = RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
		get_map_16_details();
	}

	/*
		Replace Map tile with anything
	*/
	void replace_map_tile(uint16_t tile, uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast32_t index = (x % mapWidth) + (y % mapHeight) * mapWidth;
		RAM[ram_level_low + index] = uint_fast8_t(tile); RAM[ram_level_high + index] = tile >> 8;

		RAM_decay_time_level[index] = level_ram_decay_time * PlayerAmount;
	}

	/*
		Get ground
	*/
	double ground_y(double x_relative, uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast16_t tile = get_tile(x, y);
		if (tile == 0x1AA || tile == 0x1AB) //45* slope Right
		{
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return x_relative;
		}
		if (tile == 0x1AF || tile == 0x1B0) //45* slope Left
		{
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 16.0 - x_relative;
		}
		if (tile == 0x196) //23* slope Right P1
		{
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return x_relative / 2;
		}
		if (tile == 0x19B) //23* slope Right P2
		{
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 8.0 + x_relative / 2;
		}
		if (tile == 0x1A0) //23* slope Left P1
		{
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 16.0 - x_relative / 2;
		}
		if (tile == 0x1A5) //23* slope Left P2
		{
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 8.0 - x_relative / 2;
		}
		return 16.0;
	}

	double ground_s(uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast16_t tile = get_tile(x, y);
		if ((tile == 0x1AA || tile == 0x1AB) || (tile == 0x1AF || tile == 0x1B0)) //45* slope Right/Left
		{
			return 16.0;
		}

		if ((tile == 0x1A0 || tile == 0x1A5) || (tile == 0x196 || tile == 0x19B)) //23* slope Right/Left
		{
			return 16.0;
		}
		return 15.0;
	}
	/*
		Check if a tile is sloped
	*/
	uint_fast8_t get_slope(uint_fast16_t x, uint_fast16_t y)
	{
		/*
			45
		*/
		if (tile == 0x1AA || tile == 0x1AB) { return 1; }
		if (tile == 0x1AF || tile == 0x1B0) { return 2; }
		/*
			23
		*/

		if (tile == 0x196) { return 3; }
		if (tile == 0x19B) { return 4; }

		if (tile == 0x1A0) { return 5; }
		if (tile == 0x1A5) { return 6; }
		return 0;
	}

	/*
		Process block hit.
	*/
	void process_block(uint_fast16_t x, uint_fast16_t y, uint8_t side, bool pressing_y = false, bool shatter = false)
	{
		if (!isClient)
		{
			uint_fast32_t index = (x % mapWidth) + (y * mapWidth);


			uint_fast16_t t = RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
			if (t == 0x11E && side == bottom)
			{
				blocks_processing.push_back(block_timer{ 0x48, x, y, BOUNCE_TIME, true, 0x40, 0x8, double(x * 16), double(y * 16) - 17.0, 0.0, 4.0 });
				blocks_processing.push_back(block_timer{ 0x11E, x, y, 0x100+4 });
				replace_map_tile(0xFF, x, y);
			}

			if (t == 0x11E && shatter)
			{
				replace_map_tile(0x25, x, y);
				RAM[0x1DFC] = 7;

				for (int x_p = 0; x_p < 2; x_p++)
				{
					for (int y_p = 0; y_p < 2; y_p++)
					{
						createParticle(0x3C, 0x00, 0x8, 1, (x * 16) + x_p * 8, -24 + (y * 16) + y_p * 8, ((x_p * 2) - 1), 2 + y_p * 2, Calculate_Speed(64), int(rand() % 16));
					}
				}
			}


			if (t == 0x0124 && side == bottom)
			{
				replace_map_tile(0xFF, x, y);
			}
			if (t == 0x011F && side == bottom)
			{
				replace_map_tile(0xFF, x, y);
				uint_fast8_t spr = spawnSpriteJFKMarioWorld(0x74, 5, x * 16, 8 + y * 16, 1, true);
				RAM[0x2480 + spr] = 0x20;
			}
			if ((t == 0x11F || t == 0x124) && side == bottom) //With bounce sprites
			{
				blocks_processing.push_back(block_timer{ 0x132, x, y, BOUNCE_TIME, true, 0x2A, 0x8, double(x * 16), double(y * 16)-17.0, 0.0, 4.0});
			}
			if (t == 0x0112 && side == bottom)
			{
				RAM[0x14AF] = !RAM[0x14AF];
				RAM[0x1DF9] = 0xB;

				blocks_processing.push_back(block_timer{ 0x112, x, y, BOUNCE_TIME, true, 0xCE, 0xB, double(x * 16), double(y * 16) - 17.0, 0.0, 4.0 });
				replace_map_tile(0xFF, x, y);
			}

			/*
				Midway Point
			*/
			if (t == 0x0038)
			{
				replace_map_tile(0x0025, x, y);
				RAM[0x1DF9] = 5;

				ASM.Write_To_Ram(0x3F0B, x * 16, 2);
				ASM.Write_To_Ram(0x3F0D, y * 16, 2);
				midway_activated = true;
			}

			/*
				Coins
			*/
			if (t == 0x002B)
			{
				replace_map_tile(0x0025, x, y);
				RAM[0x1DFC] = 1;
				RAM[0x0DBF] += 1;

				createParticle(0x7D, 0x00, 0x8, 2, x * 16, -20 + y * 16, 0, 0, 0, 0);
				createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -28 + y * 16, 0, 0, 0, -4);
				createParticle(0x7D, 0x00, 0x8, 2, 8 + x * 16, -20 + y * 16, 0, 0, 0, -8);
				createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -12 + y * 16, 0, 0, 0, -12);
			}

			/*
				Dragon Coins
			*/
			if (t == 0x002D)
			{
				replace_map_tile(0x0025, x, y);
				replace_map_tile(0x0025, x, y - 1);

				createParticle(0x7D, 0x00, 0x8, 2, x * 16, -28 + y * 16, 0, 0, 0, 0);
				createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -35 + y * 16, 0, 0, 0, -4);
				createParticle(0x7D, 0x00, 0x8, 2, 8 + x * 16, -28 + y * 16, 0, 0, 0, -8);
				createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -20 + y * 16, 0, 0, 0, -12);
			}
			if (t == 0x002E)
			{
				replace_map_tile(0x0025, x, y);
				replace_map_tile(0x0025, x, y + 1);

				createParticle(0x7D, 0x00, 0x8, 2, x * 16, -12 + y * 16, 0, 0, 0, 0);
				createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -20 + y * 16, 0, 0, 0, -4);
				createParticle(0x7D, 0x00, 0x8, 2, 8 + x * 16, -12 + y * 16, 0, 0, 0, -8);
				createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -4 + y * 16, 0, 0, 0, -12);
			}

			if (t == 0x002D || t == 0x002E)
			{
				RAM[0x1DF9] = 0x1C;
				RAM[0x1420]++;
			}

			/*
				Get block
			*/
			if (t == 0x012E && pressing_y)
			{
				replace_map_tile(0x0025, x, y);
				x *= 16;
				y *= 16;
				spawned_grabbable = spawnSpriteJFKMarioWorld(0x53, 2, x, y, 0, true);
			}
		}

	}

	/*
		Get tile on map.
	*/
	uint_fast16_t get_tile(uint_fast16_t x, uint_fast16_t y)
	{
		if (x >= mapWidth || y >= mapHeight) { return 0x25; }
		uint_fast32_t index = x + y * mapWidth;
		return RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
	}

	/*
		process all global
	*/

	void process_global()
	{
		for (int i = 0; i < blocks_processing.size(); i++)
		{
			block_timer& b = blocks_processing[i];
			if (b.has_spr)
			{
				b.draw();
			}
			b.time--;
			if (b.time == 0)
			{
				replace_map_tile(b.set_to, b.x, b.y);
				blocks_processing.erase(blocks_processing.begin() + i);
				i--;
			}
		}
	}


};
map16blockhandler map16_handler;

void reset_map()
{
	for (int i = ram_level_low; i < ram_level_high; i++) {
		RAM[i] = 0x25;
		RAM[i + 0x4000] = 0x00;
	}

	for (int i = 0; i < 0x1000; i++) {
		RAM[0x2000 + i] = 0;
	}

	blocks_processing.clear();
}