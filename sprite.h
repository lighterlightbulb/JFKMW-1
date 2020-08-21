#pragma once


SDL_Surface* loadSurface(string pathD)
{
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(pathD.c_str());
	return loadedSurface;
}

class TextureManager
{
public:
	unordered_map<string, SDL_Texture*> Textures;
	SDL_Texture *loadTexture(string file)
	{
		auto entry = Textures.find(file);

		if (entry != Textures.end())
		{
			return entry->second;
		}
		SDL_Surface *s = loadSurface(file);
		SDL_Texture *t = SDL_CreateTextureFromSurface(ren, s);
		SDL_FreeSurface(s);
		Textures.insert(make_pair(file, t));

		return t;

	}

	void ClearManager()
	{
		if (Textures.size() > 0)
		{
			cout << purple_int << "[TexManager] Clearing TexManager" << white << endl;
			for (unordered_map<string, SDL_Texture*>::iterator it = Textures.begin(); it != Textures.end(); ++it)
			{
				SDL_DestroyTexture(it->second);
			}
			Textures.clear();
		}
	}
};
TextureManager TexManager;

SDL_Texture *bg_texture;

void CreateSprite(string sprite, int x, int y, int size_x, int size_y)
{
	SDL_Rect DestR;

	DestR.x = x;
	DestR.y = y;
	DestR.w = abs(size_x);
	DestR.h = abs(size_y);
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (size_x < 0) { flip = SDL_FLIP_HORIZONTAL; }
	SDL_RenderCopyEx(ren, TexManager.loadTexture(sprite), NULL, &DestR, 0, NULL, flip);
}

void RenderBackground(int x, int y)
{
	double bg_scale_x = 32.0 / double(RAM[0x38]);
	double bg_scale_y = 32.0 / double(RAM[0x39]);

	SDL_Rect SourceR;
	SourceR.x = x;
	SourceR.y = y;
	SourceR.w = int(512.0 * bg_scale_x);
	SourceR.h = int(512.0 * bg_scale_y);

	SDL_RenderCopyEx(ren, bg_texture, NULL, &SourceR, (double(ASM.Get_Ram(0x36,1))*360.0)/256.0, NULL, SDL_FLIP_NONE);
}
