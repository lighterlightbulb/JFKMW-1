#pragma once

bool getKey(int what_want, int state = 0x7FFF)
{
	if (GetAsyncKeyState(what_want) & state)
	{
		return true;
	}
	return false;
}

void doNothing()
{
	//Do absolutely nothing lmao
}
void edit_config()
{
	if (current_file != "")
	{
		string level_config_entry;
		cout << "Level config entry to edit : ";
		cin >> level_config_entry;
		bool found = false;

		unordered_map<string, uint_fast32_t>::iterator it;

		for (it = lManager.level_data.begin(); it != lManager.level_data.end(); it++)
		{
			if (it->first == level_config_entry)
			{
				string new_value;
				cout << "New value : ";
				cin >> new_value;
				found = true;
				it->second = uint_fast32_t(stoi(new_value, nullptr, 16));
			}
		}
		if (found)
		{
			cout << green << "Set config value." << white << endl;
		}
		else
		{
			cout << "Config value not found! A list of valid config values is here :" << endl;
			for (it = lManager.level_data.begin(); it != lManager.level_data.end(); it++)
			{
				cout << lua_color << it->first << white << " = " << yellow << it->second << white << endl;
			}
		}
	}
}


void toggle_snap()
{
	snap = !snap;
	//cout << (snap ? "Activated" : "Deactivated") << " 16x16 snapping." << endl;
}

void insert_sprite_cam(int new_spr)
{
	int sprite_slot = 0;
	while (Sprite_data[sprite_slot].exists && sprite_slot < 512)
	{
		sprite_slot++;
	}
	if (sprite_slot > 511)
	{
		cout << red << "Too much sprites on level!" << white << endl;
	}
	else
	{
		Sprite_data[sprite_slot].num = new_spr;
		Sprite_data[sprite_slot].dir = 1;
		Sprite_data[sprite_slot].x = (camera_x + 8) * 16;
		Sprite_data[sprite_slot].y = (camera_y + 7) * 16;
		Sprite_data[sprite_slot].type = true;
		Sprite_data[sprite_slot].exists = true;
	}
}

int last_inserted = 0;
void insert()
{
	if (current_file != "")
	{

		string new_sprite = "00";
		if (!keyboard_or_controller)
		{
			cout << green << "List of sprites (" << installedSprites.size() << " installed)" << white << endl;
			for (int i = 0; i < installedSprites.size(); i++)
			{
				cout << yellow << int_to_hex(installedSprites[i].num, true) << " " << lua_color << installedSprites[i].name << white << endl;
			}
			cout << "Insert Sprite: ";
			cin >> new_sprite;
		}

		last_inserted = stoi(new_sprite, nullptr, 16);
		insert_sprite_cam(stoi(new_sprite, nullptr, 16));
	}
}

void insert_last()
{
	insert_sprite_cam(last_inserted);
}

void save()
{
	if (current_file != "")
	{

		std::ofstream out(current_file);
		out << ExportLevelToString();
		out.close();

		cout << green << "Saved edited level to " << current_file << white << endl;
	}
}

void resetCam()
{
	camera_x = 1;
	camera_y = 1;
}

void init_button(int i, int x, int b, void* DownEvent, void* UpEvent)
{
	buttons[i].x = x+23;
	buttons[i].b = b;
	buttons[i].pressed = false;
	buttons[i].onButtonDown = DownEvent;
	buttons[i].onButtonUp = UpEvent;
}


void playtest()
{
	save();
	if (current_file == "")
	{
		cout << red << "[SE] No" << white << endl;
		return;
	}
	if (jfkmw_executable == "")
	{
		wchar_t filename[MAX_PATH];

		OPENFILENAME ofn;
		ZeroMemory(&filename, sizeof(filename)); ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = NULL; ofn.lpstrFilter = L"Executable.\0*.exe\0";
		ofn.lpstrFile = filename; ofn.nMaxFile = MAX_PATH; ofn.lpstrTitle = L"Select the JFK mario world executable";
		ofn.Flags = OFN_FILEMUSTEXIST;

		if (!GetOpenFileName(&ofn)) {
			cout << red << "[SE] No" << white << endl;
		}
		else {
			std::wstring ws(filename);
			string fl = ws2s(ws);
			jfkmw_executable = fl;
		}
	}
	
	if (jfkmw_executable != "")
	{

		string command = "\"" + jfkmw_executable + "\" -l " + clevel;
		cout << yellow << command << white << endl;
		system(command.c_str());
	}
}

void newFile()
{
	cout << yellow << "Creating a new level, we will need you to input a few things:" << white << endl;
	cout << lua_color << "Size X: " << white;
	cin >> L_SIZE_X;
	cout << lua_color << "Size Y: " << white;
	cin >> L_SIZE_Y;
	if ((L_SIZE_X * L_SIZE_Y) > 16384)
	{
		cout << red << "[SE] Level data surpasses max levelram size! " << L_SIZE_X << " * " << L_SIZE_X << " = " << (L_SIZE_X * L_SIZE_Y) << " > 16384" << white << endl;
		return;
	}
	cout << green << "[SE] Created level. It will be saved to the editor's folder. After that, create your own folder with it and move it there." << white << endl;

	start_x = 1;
	start_y = 1;
	InitializeLevel();

	lManager.level_data.clear();
	lManager.add_entry("music", 1);
	lManager.add_entry("background", 0);
	lManager.add_entry("size_x", L_SIZE_X);
	lManager.add_entry("size_y", L_SIZE_Y);
	lManager.add_entry("gfx_1", 0);
	lManager.add_entry("gfx_2", 1);
	lManager.add_entry("gfx_3", 3);
	lManager.add_entry("gfx_4", 3);
	lManager.add_entry("gfx_5", 5);
	lManager.add_entry("gfx_6", 6);
	lManager.add_entry("gfx_7", 0);
	lManager.add_entry("gfx_8", 0);
	lManager.add_entry("sp_1", 5);
	lManager.add_entry("sp_2", 6);
	lManager.add_entry("sp_3", 19);
	lManager.add_entry("sp_4", 20);
	lManager.add_entry("start_x", 1);
	lManager.add_entry("start_y", 1);

	lManager.scripts = "level_lua.lua = main\n";


	std::ofstream out(path + "Editor/new_l/level_lua.lua");
	out << "function Init()\n	asm_write(1, 0x1412, 1)\n	asm_write(1, 0x1411, 1)\n\n	asm_write(8, 0x3F06, 1)\n	asm_write(8, 0x3F07, 2)\nend\n\nfunction Main()\nend";
	out.close();

	current_file = path + "Editor/new_l/level_data.txt";

	save();
}

void init_controls()
{
	init_button(0, 8-23, 8, newFile, doNothing);
	init_button(1, 8, 0, LoadFile, doNothing);
	init_button(2, 8 + 23, 1, save, doNothing);
	init_button(3, 70, 2, edit_config, doNothing);
	init_button(4, 93, 4, insert, doNothing);
	init_button(5, 93 + 23, 5, insert_last, doNothing);
	init_button(6, 93 + 56 + 6, 6, resetCam, doNothing);
	init_button(7, 93 + 56 + 23 + 6, 3, toggle_snap, doNothing);
	init_button(8, 93 + 56 + 56 + 6, 7, playtest, doNothing);

}

/*

Actual code part :D

*/

void handle_controls()
{
	if (!keyboard_or_controller)
	{
		SDL_GetMouseState(&mouse_x, &mouse_y);
		mouse_y -= 32;
		mouse_l = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
		mouse_r = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
		input_down = getKey(VK_DOWN);
		input_up = getKey(VK_UP);
		input_left = getKey(VK_LEFT);
		input_right = getKey(VK_RIGHT);
		input_del = getKey(VK_DELETE);


	}
	else
	{
		int stick_move_x = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_LEFTX) + SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_RIGHTX) / 2;
		int stick_move_y = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_LEFTY) + SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_RIGHTY) / 2;
		if (abs(stick_move_x) < 2048) {
			stick_move_x = 0;
		}
		if (abs(stick_move_y) < 2048) {
			stick_move_y = 0;
		}
		stick_x += double(stick_move_x) / 6000.0;
		stick_y += double(stick_move_y) / 6000.0;
		mouse_x = int(stick_x);
		mouse_y = int(stick_y);
		mouse_l = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 24576;
		mouse_r = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 24576;

		input_down = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		input_up = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_DPAD_UP);
		input_left = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		input_right = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		input_del = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_B);
	}

	mouse_p = false;
	if (mouse_l != mouse_l_old)
	{
		mouse_l_old = mouse_l;
		if (mouse_l == true)
		{
			mouse_p = true;
		}
	}

	int bound_x = p_w - 8;
	int bound_y = p_h - 8 - 32;
	int selection_loc_x = p_w - 256;

	if (mouse_y < -32) { mouse_y = -32; stick_y = -32.0; }
	if (mouse_x < 8) { mouse_x = 8; stick_x = 8.0; }
	if (mouse_y > bound_y) { mouse_y = bound_y; stick_y = bound_y; }
	if (mouse_x > bound_x) { mouse_x = bound_x; stick_x = bound_x; }


	if (current_file != "")
	{
		//Mouse Press Event
		if (mouse_p) {
			if (picked_sprite < 0) {
				//Select tile from map16
				if (mouse_x > selection_loc_x && mouse_x < p_w && mouse_y > 0 && mouse_y < 512) {
					int p_x = ((mouse_x - selection_loc_x) >> 4);
					int p_y = ((512 - mouse_y) >> 4);
					picked_tile = (p_x % 16) + (31 - p_y) * 16;
				}

				//pick up a sprite
				if (mouse_x < selection_loc_x)
				{
					for (int i = 0; i < 512; i++) {
						if (Sprite_data[i].exists) {
							int x_pos = 16 + Sprite_data[i].x - camera_x * 16;
							int y_pos = -32 + (p_h - 32) - Sprite_data[i].y + camera_y * 16;
							if (mouse_x > x_pos && mouse_x < (x_pos + 16) && mouse_y > y_pos && mouse_y < (y_pos + 16)) {
								picked_sprite = i;
							}
						}
					}
				}
			}
		}
		if (just_dropped)
		{
			just_dropped--;
		}

		if (picked_sprite >= 0)
		{
			int p_x = snap ? ((mouse_x >> 4) << 4) + 8 : mouse_x;
			int p_y = snap ? ((mouse_y >> 4) << 4) + (8 + (p_h & 0xF)) : mouse_y;
			Sprite_data[picked_sprite].x = -24 + p_x + camera_x * 16;
			Sprite_data[picked_sprite].y = -24 + (p_h - 32) - p_y + camera_y * 16;

			last_inserted = Sprite_data[picked_sprite].num;
		}

		//Mouse  is no longer down
		if (!mouse_l && picked_sprite >= 0)
		{
			picked_sprite = -1;
			just_dropped = 32;
		}
		//Mouse D event
		if (just_dropped == 0)
		{
			if ((picked_sprite < 0 && mouse_l) && (mouse_x > 0 && mouse_x < selection_loc_x && mouse_y > 0 && mouse_y < p_h))
			{
				for (int lx = -block_size; lx <= block_size; lx++)
				{
					for (int ly = -block_size; ly <= block_size; ly++)
					{
						int p_x = camera_x - 1 + (mouse_x >> 4) + lx;
						int p_y = camera_y - 1 + (((p_h - 32) - mouse_y) >> 4) + ly;

						if (p_x > -1 && p_y > -1 && p_y < L_SIZE_Y && p_x < L_SIZE_X)
						{
							leveldata[p_x + p_y * L_SIZE_X] = picked_tile;
						}
					}
				}
			}
		}

		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE))
		{
			int p_x = camera_x - 1 + (mouse_x >> 4);
			int p_y = camera_y - 1 + (((p_h - 32) - mouse_y) >> 4);
			start_x = p_x;
			start_y = p_y;
		}
		//Mouse R event
		if (picked_sprite < 0)
		{
			if (mouse_r && (mouse_x > 0 && mouse_x < (p_w - 256) && mouse_y > 0 && mouse_y < (p_h - 32)))
			{
				int p_x = camera_x - 1 + (mouse_x >> 4);
				int p_y = camera_y - 1 + (((p_h - 32) - mouse_y) >> 4);

				if (p_y < L_SIZE_Y && p_x < L_SIZE_X)
				{
					picked_tile = leveldata[p_x + p_y * L_SIZE_X];
				}
			}
		}
		if (input_down) {
			camera_y = max(1, camera_y - 1);
		}
		if (input_up) {
			camera_y = min(max(1, L_SIZE_Y - (DRAW_SIZE_Y-1)), camera_y + 1);
		}
		if (input_left) {
			camera_x = max(1, camera_x - 1);
		}
		if (input_right) {
			camera_x = min(max(1, L_SIZE_X - (DRAW_SIZE_X-17)), camera_x + 1);
		}


		if (picked_sprite < 0)
		{
			if (mouse_w_up)
			{
				block_size = min(3, block_size + 1);
			}
			if (mouse_w_down)
			{
				block_size = max(0, block_size - 1);
			}
		}
		else
		{
			if (mouse_w_up)
			{
				Sprite_data[picked_sprite].dir++;
			}
			if (mouse_w_down)
			{
				Sprite_data[picked_sprite].dir--;
			}
		}
		if (input_del && picked_sprite > -1) {
			Sprite_data[picked_sprite].exists = false;
			picked_sprite = -1;
		}
	}
	if (getKey(0x4F))
	{
		keyboard_or_controller = !keyboard_or_controller;
		if (SDL_NumJoysticks() > 0)
		{
			controller = 0;
			gGameController = SDL_GameControllerOpen(controller);
			if (gGameController == NULL)
			{
				cout << red << "[SDL] Controller " << controller << " error : " << SDL_GetError() << white << endl;
				keyboard_or_controller = false;
			}
			else
			{
				cout << yellow << "[SDL] Controller " << controller << " is plugged in." << white << endl;

			}
		}
		else
		{
			keyboard_or_controller = false;
		}
		cout << green << (keyboard_or_controller ? "Switching to controller input." : "Switching to keyboard & mouse.") << white << endl;
	}

	/*
	
	Buttons and mouse input
	
	*/

	for (int i = 0; i < size(buttons); i++)
	{
		int x = buttons[i].x;
		int y = 6 - 32;
		buttons[i].pressed = false;
		if (mouse_p && mouse_x > x && mouse_x < (x + 23) && mouse_y > y && mouse_y < (y + 22))
		{
			buttons[i].pressed = true;
		}

		if (buttons[i].pressed != buttons[i].pressed_old)
		{
			buttons[i].pressed_old = buttons[i].pressed;
			if (buttons[i].pressed)
			{
				((void(*)(void)) buttons[i].onButtonDown)();
			}
			else
			{
				((void(*)(void)) buttons[i].onButtonUp)();
			}
		}

		buttons[i].pressed_look = buttons[i].pressed;
	}

	buttons[7].pressed_look = snap;

}