#pragma once


void load_configuration()
{
	ifstream cFile(path + "game_config.cfg");
	if (cFile.is_open())
	{
		string line;
		while (getline(cFile, line)) {
			line.erase(remove_if(line.begin(), line.end(), ::isspace),
				line.end());
			if (line[0] == '#' || line.empty())
				continue;
			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);
			cout << cyan << "[CONFIG] Loading " << name << " = " << value << white << '\n';
			if (name == "resolution_x") { resolution_x = stoi(value); }
			if (name == "resolution_y") { resolution_y = stoi(value); }
			if (name == "internal_res_x") { int_res_x = stoi(value) * 16; }
			if (name == "internal_res_y") { int_res_y = stoi(value) * 16; }
			if (name == "fullscreen") { fullscreen = value == "true"; }
			if (name == "opengl") { opengl = value == "true"; }
			if (name == "network_update_rate") { network_update_rate = max(16, stoi(value)); network_update_rate_c = network_update_rate; }
			if (name == "packet_wait_time") { packet_wait_time = max(16, stoi(value)); packet_wait_time_c = packet_wait_time; }
			if (name == "rendering_device") { rendering_device = stoi(value); }
			if (name == "renderer_accelerated") { renderer_accelerated = value == "true"; }
			if (name == "v_sync") { v_sync = value == "true"; }
			if (name == "smooth_camera") { smooth_camera = value == "true"; }
			if (name == "smooth_camera_speed") { smooth_camera_speed = double(stoi(value)); }
			if (name == "monitor_hz") { monitor_hz = stoi(value); }
			if (name == "scale") { scale = stoi(value); if (scale > 0) { forced_scale = true; }}
			if (name == "integer_scaling") { integer_scaling = value == "true"; }
			if (name == "quality") { scale_quality = value; }

			if (name == "spc_delay") { spc_delay = stoi(value); }
			if (name == "spc_buffer_size") { spc_buffer_size = stoi(value); }
			if (name == "sample_rate") { ogg_sample_rate = stoi(value); }
			if (name == "audio_format") { audio_format = value == "AUDIO_S16" ? AUDIO_S16 : AUDIO_U16; }


			if (name == "username") {
				username = value;
				username = username.substr(0, player_name_size);
			}
			if (name == "skin") { my_skin = uint_fast8_t(stoi(value)); }
			if (name == "port") { PORT = stoi(value); }

			if (value == "LeftShift") value = "Left Shift";
			if (value == "RightShift") value = "Right Shift";

			if (value == "LeftCtrl") value = "Left Ctrl";
			const char *v = value.c_str();
			if (name == "button_y") { input_settings[0] = SDL_GetScancodeFromName(v); }
			if (name == "button_b") { input_settings[1] = SDL_GetScancodeFromName(v); }
			if (name == "button_a") { input_settings[2] = SDL_GetScancodeFromName(v); }
			if (name == "button_x") { input_settings[3] = SDL_GetScancodeFromName(v); }
			if (name == "button_left") { input_settings[4] = SDL_GetScancodeFromName(v); }
			if (name == "button_right") { input_settings[5] = SDL_GetScancodeFromName(v); }
			if (name == "button_down") { input_settings[6] = SDL_GetScancodeFromName(v); }
			if (name == "button_up") { input_settings[7] = SDL_GetScancodeFromName(v); }
			if (name == "button_select") { input_settings[8] = SDL_GetScancodeFromName(v); }
			if (name == "button_start") { input_settings[9] = SDL_GetScancodeFromName(v); }
			if (name == "button_chat") { input_settings[10] = SDL_GetScancodeFromName(v); }
			if (name == "button_togglehud") { input_settings[11] = SDL_GetScancodeFromName(v); }
			if (name == "button_togglediag") { input_settings[12] = SDL_GetScancodeFromName(v); }
			if (name == "button_togglelayer1") { input_settings[13] = SDL_GetScancodeFromName(v); }
			if (name == "button_togglebg") { input_settings[14] = SDL_GetScancodeFromName(v); }
			if (name == "button_togglesprites") { input_settings[15] = SDL_GetScancodeFromName(v); }
			if (name == "button_dumpram") { input_settings[16] = SDL_GetScancodeFromName(v); }
			if (name == "button_dumplevel") { input_settings[17] = SDL_GetScancodeFromName(v); }

			if (name == "joystick_num") { controller = stoi(value); }
			if (name == "haptic_num") { haptic = stoi(value); }
			if (name == "discord_webhook") { discord_webhook = value; }

			if (name == "use_retry") { useRetry = value == "true"; }
		}

	}
	cFile.close();
}