#pragma once

void player_code()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		cout << red << "[SDL] SDL audio/video initialization failed. SDL Error: " << SDL_GetError() << white << endl; return;
	}

	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		cout << red << "[SDL] SDL event initialization failed. SDL Error: " << SDL_GetError() << white << endl; return;
	}

	if (controller > -1)
	{
		if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
			cout << red << "[SDL] Controller init failed. SDL Error: " << SDL_GetError() << white << endl; return;
		}
	}

	if (haptic > -1)
	{
		if (SDL_Init(SDL_INIT_HAPTIC) != 0) {
			cout << red << "[SDL] Haptic initialization failed. SDL Error: " << SDL_GetError() << white << endl; return;
		}
	}


	//Initialize ZSNES UI
	load_zsnes_font();
	zsnes_ui.add_button("SINGLEPLAYER", 4, 2, 82, 13);
	zsnes_ui.add_button("MULTIPLAYER", 84, 2, 84 + 82, 13);
	zsnes_ui.add_button("RELOAD", 84 + 84, 2, 84 + 84 + 48, 13);
	zsnes_ui.add_button("X",  256-13, 2, 256-2, 13);

	//Load SDL part
	screen(resolution_x, resolution_y);
	init_input();
	init_audio();
	Mix_HaltMusic();
	string file = path + "Sounds/music/_boot.ogg"; music = Mix_LoadMUS(file.c_str()); Mix_PlayMusic(music, -1);

	string level = "";
	global_frame_counter = 0;

	zsnes_ui.message = "First boot";

	while (true)
	{
		show_full_screen = true;
		use_Overworld = false;
		LevelSprites.clear();
		particles.clear();
		zsnes_ui.hint = "";
		if (latest_error == "Kicked.")
		{
			zsnes_ui.message = "Kicked from server";
			string file = path + "Sounds/1df9/D3.ogg"; music = Mix_LoadMUS(file.c_str()); Mix_PlayMusic(music, 1);
			latest_error = "";
		}
		disconnected = false;
		PlayerAmount = 0; SelfPlayerNumber = 1; CheckForPlayers();
		quit = false;

		/* Options Loop */
		string s_or_c;
		while (true)
		{

			quit = false;
			if (done())
			{
				return;
			}
			global_frame_counter += 1;

			cls();

			if (testing_level != "" && level != "")
			{
				return;
			}
			if (testing_level != "" && level == "") {
				s_or_c = "t";
				level = testing_level;
				zsnes_ui.message = "Loading level " + level + "..";
				midway_activated = false;
			}


			if (state[SDL_SCANCODE_O]) {
				s_or_c = "t";
				
				use_Overworld = true;
				zsnes_ui.message = "Loading overworld";
				midway_activated = false;
			}
			if (zsnes_ui.hint != "")
			{
				if (zsnes_ui.button_pressed == "SINGLEPLAYER" || state[SDL_SCANCODE_Q]) {
					s_or_c = "t";
					level = zsnes_ui.hint;
					zsnes_ui.message = "Loading level " + level + "..";
					midway_activated = false;
				}
				if (zsnes_ui.button_pressed == "MULTIPLAYER" || state[SDL_SCANCODE_W]) {
#if not defined(DISABLE_NETWORK)
					ip = zsnes_ui.hint;
					zsnes_ui.message = "Connecting to " + ip + ":" + to_string(PORT);
					s_or_c = "c";
					midway_activated = false;
#else
					zsnes_ui.message = "MP Not supported";
#endif
				}
			}
			else
			{
				if (zsnes_ui.button_pressed != "none" || ((state[SDL_SCANCODE_Q] || state[SDL_SCANCODE_R]) || state[SDL_SCANCODE_W]))
				{
					zsnes_ui.message = "bruh type something";
				}
			}

			if ((zsnes_ui.button_pressed == "RELOAD" || state[SDL_SCANCODE_R]) || SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
			{

				if (level != "")
				{
					zsnes_ui.message = "Reloading";
					s_or_c = "t";
				}
			}


			zsnes_ui.process();
			zsnes_ui.finish_processing(ren);

			SDL_Rect DestR;
			DestR.x = (int_res_x - 256) / 2;
			DestR.y = (int_res_y - 224) / 2;
			DestR.w = 256;
			DestR.h = 224;
			SDL_RenderCopy(ren, zsnes_ui.texture, NULL, &DestR);

			redraw();
			check_input();

			if (zsnes_ui.button_pressed == "X")
			{
				return;
			}

			if (s_or_c != "")
			{
				break;
			}
		}
		if (!use_Overworld)
		{
			in_Overworld = false;
		}
		global_frame_counter = 0;
		/* Load Shit */

		networking = s_or_c != "t";
		initialize_map16();

		if (s_or_c == "c")
		{
			isClient = true;

#if not defined(DISABLE_NETWORK)

			if (!ConnectClient()) {
				cout << red << "[Network] Failed to connect. Falling back to normal mode." << white << endl;
				networking = false;
				isClient = false;
				disconnected = false;
				zsnes_ui.message = "Failed to connect";
				continue;
			}
#else
			cout << "Multiplayer is not supported in this build!" << endl;
			networking = false;
			isClient = false;
			s_or_c = "t";
			zsnes_ui.message = "Not supported";
#endif
		}


		if (s_or_c == "t" && !use_Overworld)
		{
			
			isClient = false;
			if (level == "")
			{
				cout << "Enter a level : "; cin >> level;
			}
			LevelManager.LoadLevel(stoi(level, nullptr, 16));
		}

		if (!isClient && !use_Overworld)
		{
			game_init();
		}
		
		if (use_Overworld)
		{
			overworld.Initialize();
		}

		//Initialize Singleplayer
		if (s_or_c == "t") {
			PlayerAmount = 1 + splitscreen; SelfPlayerNumber = 1; CheckForPlayers();
		}

#if not defined(DISABLE_NETWORK)
		//Initialize Multiplayer Client
		if (networking)
		{
			thread = new sf::Thread(&NetWorkLoop); thread->launch();
		}
#endif

		cout << yellow << "[JFKMW] Waiting for player..." << white << endl;
		while (Mario.size() == 0) {
			sf::sleep(sf::milliseconds(1));
		}

		while (!done())
		{

			chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
			chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

			while (doing_read) {
				SDL_Delay(1);
				
				
				t2 = chrono::high_resolution_clock::now();
				double t = chrono::duration_cast<chrono::duration<double>>(t2 - t1).count()*1000.0;
				if (t > 1000.0)
				{
					cout << red << "[Network] Recovered from a freeze during doing_read (it took more than 1000ms, " << dec << t << " ms)" << white << endl;
					break;
				}
			}

			doing_write = true;
			cls();
			zsnes_ui.message = "Idle";
			t1 = chrono::high_resolution_clock::now();
			if (!gGameController)
			{
				check_input();
			}
			game_loop(); SoundLoop();
			render();
			doing_write = false;

			t2 = chrono::high_resolution_clock::now();
			total_time_ticks = chrono::duration_cast<chrono::duration<double>>(t2 - t1);

			redraw();
			

			if (disconnected) {
				quit = true; cout << red << "[Network] Disconnected." << white << endl; zsnes_ui.message = "Disconnected from server"; doing_write = false; doing_read = false; break;
			}

		}

		//We quit the game go back to the ZSNES ui

#if not defined(DISABLE_NETWORK)
		if (networking) { socketG.disconnect();  thread->terminate(); }
#endif
		TexManager.ClearManager();
		Terminate_Music();
		curr_bg = 0xFF;

		cout << yellow << "[JFKMW] Returning to main screen.." << white << endl;
		quit = true;
	}
}