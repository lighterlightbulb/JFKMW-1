#pragma once
int L_SIZE_X = 0;
int L_SIZE_Y = 0;

class Sprite {
public:
	bool type = false;
	int num = 0;
	int x = 0;
	int y = 0;
	int dir = 0;
	bool exists = false;
};

Sprite Sprite_data[512];
uint_fast16_t leveldata[0x4000];
int start_x = 1;
int start_y = 1;

void InitializeLevel()
{
	for (int i = 0; i < 512; i++)
	{
		Sprite_data[i].exists = false;
	}
	for (int x = 0; x < 0x4000; x++)
	{
		leveldata[x] = 0x25;
	}
}