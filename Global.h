#pragma once

string GAME_VERSION = "2.1.2b";
string CHANGELOG = "-Minor changes and\noptimizations\n-Added worldpeace's\nretry system\n-Added Message Boxes";

#define rom_asm_size 0x008000 //32kb, 1 bank ($00:8000 to $00:FFFF)
#define location_rom_levelasm 0x008000 //this will put LevelASM on the start of the ROM, this is a SNES PC btw


/*
	These conversions will do for now, but when we expand the ROM, we have to fix these.
*/
uint_fast32_t snestopc(uint_fast32_t snes) {
	return snes - 0x8000;
}

uint_fast32_t pctosnes(uint_fast32_t pc) {
	return pc + 0x8000;
}

#define TotalBlocksCollisionCheck 2
#define bounds_x 6.0 //for collision crap
#define bounds_y 6.0
#define button_b 0
#define button_y 1
#define button_a 2
#define button_left 3
#define button_right 4
#define button_up 5
#define button_down 6
#define button_start 7

#define camBoundX 32.0
#define camBoundY 32.0

#define total_inputs 8

#define player_expected_packet_size 54 //Strings apparently add 4 so we have to make sure of this so it wont crash.
#define player_name_size 11

#define MAX_L3_TILES 0x80

#define level_ram_decay_time 40 //Server ticks before level data RAM becomes invalid to send

uint_fast16_t mapWidth = 256;
uint_fast16_t mapHeight = 32;

uint_fast16_t audio_format = AUDIO_S16;
int ogg_sample_rate = 44100;
int spc_delay = 4;
int spc_buffer_size = 320;

#define RAM_Size 0x20000
#define VRAM_Size 0x10000
#define VRAM_Location 0x10000

bool asm_loaded = false;

uint_fast8_t RAM[RAM_Size];
uint_fast8_t RAM_old[0x8000];
uint_fast8_t RAM_decay_time_level[0x4000]; //for multiplayer
uint_fast16_t hdma_size[8];
uint_fast32_t palette_array[256]; //These are cached lol
uint_fast8_t VRAM[VRAM_Size];
uint_fast8_t pipe_colors[4] = { 3, 5, 6, 7 };

#define top 8
#define bottom 4
#define left 2
#define right 1
#define inside 0

//DMA/DHMA
bool layer1mode_x = false;
int_fast16_t layer1_shiftX[224];
bool layer1mode_y = false;
int_fast16_t layer1_shiftY[224];
bool layer2mode_x = false;
int_fast16_t layer2_shiftX[224];
bool layer2mode_y = false;
int_fast16_t layer2_shiftY[224];

//threads

#if not defined(DISABLE_NETWORK)
sf::Thread* thread = 0;
#endif

//ASM
bool in_Overworld = false;
bool use_Overworld = false;
bool need_sync_music = false;
bool kill_music = false;
uint_fast8_t my_skin = 0;
uint_fast32_t global_frame_counter = 0; //Like 0x13
uint_fast32_t ingame_frame_counter = 0; //Like 0x14

chrono::duration<double> total_time_ticks;
int latest_server_response;

unsigned int network_update_rate = 16;
unsigned int packet_wait_time = 16;
unsigned int network_update_rate_c = 16;
unsigned int packet_wait_time_c = 16;
int mouse_x, mouse_y;
bool mouse_down;
bool mouse_down_r;
bool mouse_w_up;
bool mouse_w_down;

int data_size_current = 0;
int data_size_now = 0;

int rendering_device = -1;
int controller = 0;
int haptic = 0;

string testing_level = "";

#define spawn_bound_x 256 //Sprite spawn bound for camera. This should be good for now.

//Stuff hud toggles here and hud stuff
uint_fast8_t hudMode = 0;
bool drawDiag = false;
bool drawBg = true;
bool drawSprites = true;
bool drawL1 = true;

int fps_diag[128];
int ping_diag[112];
int block_diag[112];
int ram_diag[16];
int kbs_diag[112];
int blocks_on_screen = 0;

//Yep

string username = "No username";


/*
	Input related stuff
*/
SDL_Scancode input_settings[18] = {
	SDL_SCANCODE_S,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_X,
	SDL_SCANCODE_A,
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_DOWN,
	SDL_SCANCODE_UP,
	SDL_SCANCODE_RSHIFT,
	SDL_SCANCODE_RETURN,
	SDL_SCANCODE_T,
	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_4,
	SDL_SCANCODE_5,
	SDL_SCANCODE_6,
	SDL_SCANCODE_7
};
bool s_pad[total_inputs];
bool pad_p[total_inputs];
bool pad_s[total_inputs];

bool BUTTONS_GAMEPAD[10];

const Uint8* state = SDL_GetKeyboardState(NULL);
SDL_GameController* gGameController;
SDL_Haptic* haptic_device;

double controller_mouse_x = 0.0;
double controller_mouse_y = 0.0;

bool left_st_pr = false;
bool right_st_pr = false;

//Yes
vector<string> split(const string &s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

void error(const char* str)
{
	if (str)
	{
		fprintf(stderr, "Error: %s\n", str);
		exit(EXIT_FAILURE);
	}
}

unsigned char* load_file(const char* path, long* size_out)
{
	size_t size;
	unsigned char* data;


	FILE* in;
	fopen_s(&in, path, "rb");

	fseek(in, 0, SEEK_END);
	size = ftell(in);
	if (size_out)
		*size_out = long(size);
	rewind(in);

	data = (unsigned char*)malloc(size);
	if (!data) error("Out of memory");

	if (fread(data, 1, size, in) < size) error("Couldn't read file");
	fclose(in);

	return data;
}

void replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

bool is_file_exist(const char* fileName)
{
	ifstream infile(fileName);
	return infile.good();
}

int dirExists(const char* const path)
{
	struct stat info;

	int statRC = stat(path, &info);
	if (statRC != 0)
	{
		if (errno == ENOENT) { return 0; } // something along the path does not exist
		if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
		return -1;
	}

	return (info.st_mode & S_IFDIR) ? 1 : 0;
}

#define Calculate_Speed(x) double(x) / 256.0
/*double Calculate_Speed(double speed)
{
	return speed / 256.0;
}*/

string int_to_hex(int T, bool add_0 = false)
{
	stringstream stream;
	if (!add_0)
	{
		stream << hex << T;
	}
	else
	{
		if(T < 16)
		{
			stream << "0";
		}
		stream << hex << T;
	}
	
	return stream.str();
}

uint_fast8_t char_to_smw(char t)
{
	uint_fast8_t new_l = uint_fast8_t(t);

	if (t < 0x3A) { new_l = new_l - 0x30; }
	if (t >= 0x41 && t < 0x61) { new_l = new_l - 0x41 + 0xA; }
	if (t >= 0x61 && t < 0x81) { new_l = new_l - 0x61 + 0x50; }

	if (t == '<') { new_l = 0x2C; }
	if (t == '>') { new_l = 0x2D; }
	if (t == '!') { new_l = 0x28; }
	if (t == '.') { new_l = 0x24; }
	if (t == ',') { new_l = 0x25; }
	if (t == '-') { new_l = 0x27; }
	if (t == '+') { new_l = 0x29; }
	if (t == ' ') { new_l = 0xFF; }
	if (t == '_') { new_l = 0x3D; }
	if (t == '?') { new_l = 0x38; }


	return new_l;
}

#ifdef NDEBUG
string path = "";
#else
string path = "E:/JFKMarioWorld/Debug/";
#endif

bool quit = false;
bool actuallyquitgame = false;
//NET
bool doing_write = false;
bool doing_read = false;
bool recent_big_change = false;
bool disconnected = false;
bool isClient = false;


//Video
int sp_offset_x = 0;
int sp_offset_y = 0;
double scale = 1.0;
bool integer_scaling = true;
bool forced_scale = false;
string scale_quality = "nearest";

bool fullscreen = false;
bool opengl = false;
bool networking = false;
bool renderer_accelerated = true;
bool v_sync = true;
double monitor_hz = 60.0;



//Game
bool pvp = true;
bool Chatting = false;

//WP Retry
bool useRetry = true;
bool retryPromptOpen = false;
bool doRetry = true;

//Stuff
string Typing_In_Chat = "";
int_fast16_t CameraX, CameraY;
uint_fast8_t curr_bg = 0xFF;

bool showing_player_list;
bool pressed_select;
bool pressed_start;


bool smooth_camera = false;
bool midway_activated = false;
double smooth_camera_speed = 0;
uint_fast8_t SelfPlayerNumber = 1;
int PlayerAmount = 0;

uint_fast8_t death_timer[256];

//Messages

string Messages[15];
int MessageBoxTimer = 0;


//Network
string ip = "127.0.0.1"; int PORT = 0;

#if not defined(DISABLE_NETWORK)
class JFKMWSocket : public sf::TcpSocket
{
public:
	string username = "";
	uint_fast8_t latest_sync_p;
};

JFKMWSocket socketG; sf::SocketSelector selector; //no idea how this works
sf::TcpListener listener; vector<JFKMWSocket*> clients;
uint_fast8_t latest_sync;

uint_fast8_t CurrentPacket_header;
//TO-DO : add packet compression.
sf::Packet CurrentPacket;
#endif

//Rendering
uint_fast16_t int_res_x = 256;
uint_fast16_t int_res_y = 224;
int resolution_x = 320;
int resolution_y = 240;

SDL_Surface screen_s_l1;
SDL_Texture* screen_t_l1;
SDL_Texture* cached_l3_tiles[8];
SDL_Texture* target_texture_p1;
SDL_Texture* target_texture_p2;
SDL_Texture* global_texture_s;

int w; //width of the screen
int h; //height of the screen
SDL_Window* win; //The window
SDL_Renderer* ren; //The renderer
SDL_Event event = { 0 };
bool splitscreen = false;
bool show_full_screen = true;



unordered_map<uint_fast32_t, SDL_Texture*> SpriteTextures;
SDL_Texture* loadSprTexture(uint_fast32_t fl)
{
	auto entry = SpriteTextures.find(fl);
	if (entry != SpriteTextures.end())
	{
		return entry->second;
	}
	return NULL;
}

void addSprTexture(uint_fast32_t fl, SDL_Texture* newTex)
{
	SpriteTextures.insert(make_pair(fl, newTex));
}

void ClearSpriteCache()
{
	if (SpriteTextures.size() > 0)
	{
		cout << yellow << "[GFX] Clearing Sprite Cache" << white << endl;
		for (unordered_map<uint_fast32_t, SDL_Texture*>::iterator it = SpriteTextures.begin(); it != SpriteTextures.end(); ++it)
		{
			SDL_DestroyTexture(it->second);
		}
		SpriteTextures.clear();
	}
}

/*

Discord Logging

*/


#if defined(_WIN32)
string discord_webhook;

void do_d_msg(string msg)
{
	replaceAll(msg, "@", "");
	time_t currentTime;
	struct tm localTime;

	time(&currentTime);                   // Get the current time
	localtime_s(&localTime, &currentTime);  // Convert the current time to the local time

	int Hour = localTime.tm_hour;
	int Min = localTime.tm_min;
	int Sec = localTime.tm_sec;

	string H, M, S;
	H = Hour < 10 ? ("0" + to_string(Hour)) : to_string(Hour);
	M = Min < 10 ? ("0" + to_string(Min)) : to_string(Min);
	S = Sec < 10 ? ("0" + to_string(Sec)) : to_string(Sec);

	msg = "[" + H + ":" + M + ":" + S + "] " + msg;
	string cmd = "curl --silent -o nul -i -H \"Accept: application/json\" -H \"Content-Type:application/json\" -X POST --data \"{\\\"content\\\": \\\"" + msg + "\\\"}\" " + discord_webhook;
	system(cmd.c_str());
	return;
}


void discord_message(string msg)
{
	if (discord_webhook != "")
	{
		//cout << lua_color << "[Logging] Thread started with \"" << msg << "\"" << white << endl;

		sf::Thread t1(do_d_msg, msg);
		t1.launch();
	}
}
#else
string discord_webhook;
void discord_message(string msg)
{
}
#endif
/*
	Palette
*/
void ConvertPalette()
{
	/*
		0x3D00-0x3DFF Palette, low b
		0x3E00-0x3EFF Palette, high b
	*/
	if (!in_Overworld)
	{
		uint_fast8_t b = global_frame_counter;
		if ((global_frame_counter & 0x1F) > 0x0F)
		{
			b = 0x10 - (global_frame_counter - 0x10);
		}
		b = b << 4;
		uint_fast16_t col = 0x3FF + ((b >> 3) << 10);
		RAM[0x3D64] = col;
		RAM[0x3E64] = col >> 8;
	}
	else
	{
		uint_fast8_t b = global_frame_counter;
		if ((global_frame_counter & 0x1F) > 0x0F)
		{
			b = 0x10 - (global_frame_counter - 0x10);
		}
		b = b << 4;
		uint_fast16_t col = 0x3FF + ((b >> 3) << 10);
		RAM[0x3D6D] = col;
		RAM[0x3E6D] = col >> 8;

		col = 0x1F + ((b >> 3) << 5);
		RAM[0x3D7D] = col;
		RAM[0x3E7D] = col >> 8;

	}

	//Plr Name Color
	switch (my_skin % 3)
	{
	case 0:
		palette_array[0x0E] = 0xFF1838D8; break;
	case 1:
		palette_array[0x0E] = 0xFF58F858; break;
	case 2:
		palette_array[0x0E] = 0xFF505050; break;
	}

	//Hud border colors
	palette_array[0x09] = 0xFF000000;
	palette_array[0x19] = 0xFF000000;
	palette_array[0x0D] = 0xFF000000;
	palette_array[0x1D] = 0xFF000000;
	palette_array[0x0A] = 0xFF000000;
	palette_array[0x0B] = 0xFF000000;

	//Itembox
	palette_array[0x0F] = 0xFFF0A858;

	//Text 1 (White)
	palette_array[0x1A] = 0xFFFFFFFF;
	palette_array[0x1B] = 0xFFFFFFFF;

	//Text 2 (Yellow)
	if (in_Overworld)
	{
		palette_array[0x1D] = 0x00000000;
		palette_array[0x1E] = 0xFF000000;
		palette_array[0x1F] = 0xFF000000;
	}
	else
	{
		palette_array[0x1E] = 0xFF70D8F8;
		palette_array[0x1F] = 0xFF70D8F8;
	}

	/*
		Convert 16-bit palette to 32-bit palette
	*/
	for (uint_fast16_t i = 0; i < 256; i++)
	{
		if ((i >= 0x08 && i < 0x10) || (i >= 0x18 && i < 0x20)) {
			continue;
		}
		uint_fast16_t c = RAM[0x3D00 + i] + (RAM[0x3E00 + i] << 8);
		palette_array[i] =
			0xFF000000 + (((c & 0x1F) << 3)) +
			((((c >> 5) & 0x1F) << 3) << 8) +
			(((c >> 10) << 3) << 16);
	}
}

/*
	Layer 3 Caching
*/
void PreloadL3()
{
	//This makes palette
	ConvertPalette();

	//Byte masks idk
	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00; amask = 0x000000ff;
#else
	rmask = 0x000000ff; gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;
#endif

	for (uint_fast16_t e = 0; e < 8; e++)
	{
		SDL_DestroyTexture(cached_l3_tiles[e]);

		//Create a surface and lock it
		SDL_Surface* cached_l3_surf = SDL_CreateRGBSurface(0, 128, 64, 32,
			rmask, gmask, bmask, amask);
		SDL_LockSurface(cached_l3_surf);

		//Draw all L3 tiles
		uint_fast8_t color1;
		uint_fast8_t graphics_array[16];

		uint_fast8_t palette_offs = e << 2;
		for (uint_fast8_t t = 0; t < 0x80; t++)
		{
			uint_fast16_t tile = t;
			uint_fast16_t x = (tile & 0xF) << 3;
			uint_fast16_t y = ((tile >> 4) << 3);
			tile = tile << 4;
			memcpy(graphics_array, &RAM[VRAM_Location + 0xB000 + tile], 16 * sizeof(uint_fast8_t));

			for (uint_fast8_t index = 0; index < 16; index += 2)
			{
				for (uint_fast8_t i = 0; i < 8; i++)
				{
					color1 = ((graphics_array[0 + index] >> i) & 1) + (((graphics_array[1 + index] >> i) & 1) << 1);
					if (color1 != 0) {

						Uint32* p_screen = (Uint32*)(cached_l3_surf)->pixels + (x + 7 - i) + ((y + (index >> 1)) << 7);
						*p_screen = palette_array[color1 + palette_offs];
					}
				}
			}
		}

		//Unlock surface then create texture and destroy the surface to free memory.
		SDL_UnlockSurface(cached_l3_surf);
		cached_l3_tiles[e] = SDL_CreateTextureFromSurface(ren, cached_l3_surf);
		SDL_FreeSurface(cached_l3_surf);
	}
}