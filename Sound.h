#pragma once

//The music that will be played
Mix_Music *music = NULL;

//The sound effects that will be used
Mix_Chunk *sfxPorts[0x300];
uint_fast16_t sound_table[3] = { 0x1DF9, 0x1DFC, 0x1DFA };
uint8_t old_1dfb = 0xFF;

//Audio Device
SDL_AudioDeviceID audio_device;

//Audio Spec (For SPC Playbkck)
SDL_AudioSpec audio_spec;

//SPC
SNES_SPC* snes_spc;
SPC_Filter* filter;

sf::Thread* music_thread = 0;

#define MUSIC_SPC 0
#define MUSIC_OGG 1
#define MUSIC_MID 2
uint_fast8_t music_type = MUSIC_SPC;

char* music_data;
int music_data_size;

void Terminate_Music()
{
	if (music_thread) { music_thread->terminate(); }
	old_1dfb = 0xFF;
	SDL_ClearQueuedAudio(audio_device);
	Mix_HaltMusic();
}

void EmulateSPC_Loop()
{
	short* buf = new short[spc_buffer_size];
	while (true)
	{
		if (music_type == MUSIC_SPC)
		{
			spc_play(snes_spc, spc_buffer_size, buf);
			spc_filter_run(filter, buf, spc_buffer_size);
			SDL_QueueAudio(audio_device, buf, spc_buffer_size * 2);
		}
		
		SDL_Delay(spc_delay);
	}
}

bool init_audio()
{
	//Initialize SDL_mixer
	if (Mix_OpenAudio(ogg_sample_rate, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
	{
		cout << purple << "[Audio] Error : " << Mix_GetError() << white << endl;
		return false;
	}

	cout << purple << "[Audio] Initialized audio port (" << dec << ogg_sample_rate << "hz)." << white << endl;
	/*
		Initialize Audio Spec (For SPC)
	*/
	SDL_zero(audio_spec);
	audio_spec.freq = 32000;
	audio_spec.format = audio_format;
	audio_spec.channels = 2; audio_spec.samples = 1024;
	audio_spec.callback = NULL;

	/*
		Open Audio Device
	*/
	audio_device = SDL_OpenAudioDevice(
		NULL, 0, &audio_spec, NULL, 0);

	snes_spc = spc_new();
	filter = spc_filter_new();

	cout << purple << "[Audio] Initialized audio spec and SPC playback (32000hz)." << white << endl;
	
	/* Clear filter before playing */
	spc_filter_clear(filter);
	spc_filter_set_gain(filter, 300);

	SDL_PauseAudioDevice(audio_device, 0);

	int snds = 0;
	for (uint_fast8_t i = 0; i < 3; i++)
	{
		for (uint_fast16_t sounds = 0; sounds < 0x100; sounds++)
		{
			string sfx_to_play = path + "Sounds/" + int_to_hex(sound_table[i]) + "/" + int_to_hex(sounds, true) + ".ogg";
			const char* cstr = sfx_to_play.c_str();
			sfxPorts[sounds + (i << 8)] = Mix_LoadWAV(cstr);
			if (sfxPorts[sounds + (i << 8)] != NULL)
			{
				snds++;
			}
		}
	}

	cout << purple << "[Audio] Pre-loaded " << dec << snds << " sounds." << white << endl;

	

	return true;
}

#if not defined(DISABLE_NETWORK)
void SendMusic()
{
	while (doing_write) {
		sf::sleep(sf::milliseconds(1));
	}
	doing_read = true;
	cout << green << "[Network] Packing music data.." << endl;
	CurrentPacket << music_type;
	CurrentPacket << music_data_size;
	for (int i = 0; i < music_data_size; i++)
	{
		CurrentPacket << (uint_fast8_t)music_data[i];
	}
	doing_read = false;
}

void ReceiveMusic(bool dont_care = false)
{
	if (!dont_care)
	{
		while (doing_write) {
			sf::sleep(sf::milliseconds(1));
		}
		kill_music = true;
		while (kill_music) {
			Sleep(1);
		}
	}
	doing_read = true;
	cout << green << "[Network] Downloading music from server.." << endl;

	CurrentPacket >> music_type;
	CurrentPacket >> music_data_size;
	delete[] music_data;
	music_data = new char[music_data_size];
	for (int i = 0; i < music_data_size; i++)
	{
		uint_fast8_t g;
		CurrentPacket >> g;
		music_data[i] = (char)g;
	}
	cout << green << "[Network] Received new music. " << dec << music_data_size / 1024 << "kb big." << endl;

	doing_read = false;
	need_sync_music = true;
}
#endif

void SoundLoop()
{
	if (!networking || isClient)
	{
		for (uint_fast8_t i = 0; i < 3; i++)
		{
			uint_fast16_t RAM_P = sound_table[i];
			if (RAM[RAM_P] != 0)
			{

				uint_fast16_t MixChunk = RAM[RAM_P] + (i << 8);

				if (sfxPorts[MixChunk] != NULL) {
					if (Mix_PlayChannel(1 + i, sfxPorts[MixChunk], 0) == -1) {
						cout << purple << "[Audio] Port " << dec << (i + 1) << " Error : " << Mix_GetError() << white << endl;
					}
				}
				else
				{
					cout << purple << "[Audio] Port " << dec << (i + 1) << " Error : Doesn't exist" << white << endl;
				}

				RAM[RAM_P] = 0;
			}

		}
		if (!networking)
		{
			if (ASM.Get_Ram(0x1DFB, 1) != old_1dfb)
			{
				if (ASM.Get_Ram(0x1DFB, 1) == 0xFF)
				{
					old_1dfb = ASM.Get_Ram(0x1DFB, 1);
					Mix_FadeOutMusic(1000);
					spc_write_port(snes_spc, 32, 3, 0);

					cout << purple << "[Audio] Fading music" << dec << white << endl;
				}
				else
				{
					Terminate_Music();
					old_1dfb = ASM.Get_Ram(0x1DFB, 1);



					string file1 = path + "Sounds/music/" + int_to_hex(old_1dfb, true) + ".spc";
					string file2 = path + "Sounds/music/" + int_to_hex(old_1dfb, true) + ".ogg";
					string file3 = path + "Sounds/music/" + int_to_hex(old_1dfb, true) + ".mid";
					if (is_file_exist(file1.c_str())) {

						//SPC File loading
						long spc_size;
						void* spc = load_file(file1.c_str(), &spc_size);
						spc_load_spc(snes_spc, spc, spc_size);
						free(spc); /* emulator makes copy of data */

						music_type = MUSIC_SPC;
					}
					if (is_file_exist(file2.c_str())) {
						music = Mix_LoadMUS(file2.c_str());
						music_type = MUSIC_OGG;
					}
					if (is_file_exist(file3.c_str())) {
						music = Mix_LoadMUS(file3.c_str());
						music_type = MUSIC_MID;
					}

					if (music_type == MUSIC_MID || music_type == MUSIC_OGG)
					{
						if (music == NULL)
						{
							cout << purple << "[Audio] Failed to change music : " << Mix_GetError() << white << endl;
						}
						else
						{
							Mix_PlayMusic(music, -1);
						}
					}
					else
					{
						music_thread = new sf::Thread(&EmulateSPC_Loop); music_thread->launch();
					}
					cout << purple << "[Audio] Playing song 0x" << hex << int_to_hex(old_1dfb, true) << dec << white << endl;
				}

			}
		}
		else
		{
			if (kill_music)
			{
				Terminate_Music();

				kill_music = false;
			}
			if (need_sync_music)
			{
				need_sync_music = false;

				if (music_type == MUSIC_SPC)
				{
					spc_load_spc(snes_spc, music_data, music_data_size);
					music_thread = new sf::Thread(&EmulateSPC_Loop); music_thread->launch();
				}

				if (music_type == MUSIC_OGG || music_type == MUSIC_MID)
				{
					SDL_RWops* rw = SDL_RWFromMem(music_data, music_data_size);
					Mix_Music* music = Mix_LoadMUSType_RW(rw, music_type == MUSIC_OGG ? MUS_OGG : MUS_MID, 0);
					if (music == NULL)
					{
						Mix_HaltMusic();
						cout << purple << "[Audio] Failed to change music : " << Mix_GetError() << white << endl;
					}
					else
					{
						if (Mix_PlayingMusic() == 1)
						{
							Mix_HaltMusic();
						}
						Mix_PlayMusic(music, -1);
					}
				}
				cout << purple << "[Audio] Playing music" << white << endl;
			}
		}
	}
	else
	{
		//Music additions for serverside
		if (ASM.Get_Ram(0x1DFB, 1) != old_1dfb)
		{
			old_1dfb = ASM.Get_Ram(0x1DFB, 1);
			string file1 = path + "Sounds/music/" + int_to_hex(old_1dfb, true) + ".spc";
			string file2 = path + "Sounds/music/" + int_to_hex(old_1dfb, true) + ".ogg";
			string file3 = path + "Sounds/music/" + int_to_hex(old_1dfb, true) + ".mid";

			string file_picked;
			if (is_file_exist(file1.c_str())) {
				file_picked = file1;
				music_type = MUSIC_SPC;
			}
			if (is_file_exist(file2.c_str())) {
				file_picked = file2;
				music_type = MUSIC_OGG;
			}
			if (is_file_exist(file3.c_str())) {
				file_picked = file3;
				music_type = MUSIC_MID;
			}

			ifstream input(file_picked, ios::in | ios::binary | ios::ate);
			if (input.is_open())
			{
				delete[] music_data;
				music_data_size = int(input.tellg());
				music_data = new char[music_data_size];
				input.seekg(0, ios::beg);
				input.read(music_data, music_data_size);
				input.close();
				cout << purple << "[Audio] Loaded " << file_picked << white << endl;
				need_sync_music = true;
			}
			else
			{
				cout << purple << "[Audio] Failed to load " << file_picked << white << endl;
			}
		}
	}
}
