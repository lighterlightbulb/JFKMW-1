#pragma once

SDL_Surface* MAP16_SURF;
SDL_Texture* MAP16_TEX;

SDL_Surface* BG_SURF;
SDL_Texture* BG_TEX;

void load_background(uint_fast8_t num)
{
	if (BG_TEX) { SDL_DestroyTexture(BG_TEX); }

	string str = path + "Sprites/backgrounds/Background" + to_string(int(num)) + ".png";
	cout << "[SE] Loading BG " << str << endl;

	BG_SURF = IMG_Load(str.c_str());
	BG_TEX = SDL_CreateTextureFromSurface(ren, BG_SURF);
	SDL_FreeSurface(BG_SURF);
}

void InitializeMap16()
{
	if (MAP16_TEX) { SDL_DestroyTexture(MAP16_TEX); }

	string str = path + "Editor/map16data.png";
	cout << "[SE] Loading assets..." << endl;

	MAP16_SURF = IMG_Load(str.c_str());
	MAP16_TEX = SDL_CreateTextureFromSurface(ren, MAP16_SURF);
	SDL_FreeSurface(MAP16_SURF);
}