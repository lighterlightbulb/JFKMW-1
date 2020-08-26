#pragma once

/*
	TO-DO: Make chat more secure and unspammable (For now you can't send similar messages, which is good.)
*/
#define chat_onscreen_timer 420
string Curr_ChatString[6] = {"","","","","",""};
string Curr_PChatString = "";
string Old_ChatString = "";
int Time_ChatString[6] = {0,0,0,0,0,0}; //How long it will take for this string to disappear, set to 300 every time a message is sent. In frames

//Add chat line
void Add_Chat(string c)
{
	/*
		This is only used in the client to push the chat back. This is a very rough implementation and it doesn't use a for loop since I couldn't get it to work.
	*/
	Curr_ChatString[5] = Curr_ChatString[4];
	Curr_ChatString[4] = Curr_ChatString[3];
	Curr_ChatString[3] = Curr_ChatString[2];
	Curr_ChatString[2] = Curr_ChatString[1];
	Curr_ChatString[1] = Curr_ChatString[0];

	Time_ChatString[5] = Time_ChatString[4];
	Time_ChatString[4] = Time_ChatString[3];
	Time_ChatString[3] = Time_ChatString[2];
	Time_ChatString[2] = Time_ChatString[1];
	Time_ChatString[1] = Time_ChatString[0];

	Curr_ChatString[0] = c;
	Time_ChatString[0] = chat_onscreen_timer;
}

void Send_Chat(string c)
{
	Curr_PChatString = c;
	Time_ChatString[0] = chat_onscreen_timer;
}

//Serverside chat handler
void Chat_ServerSide()
{
	int p = 1;
	for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
	{
		MPlayer& CurrPlayer = *item;
		if (CurrPlayer.last_chat_string != CurrPlayer.curr_chat_string && CurrPlayer.curr_chat_string.length() > 0)
		{
			//Update curr pchatstring to send later
			Curr_PChatString = CurrPlayer.curr_chat_string;
			Curr_PChatString = Curr_PChatString.substr(0, min(64, int(Curr_PChatString.length())));
			Time_ChatString[0] = chat_onscreen_timer;

			//Update strings and output to console
			CurrPlayer.last_chat_string = CurrPlayer.curr_chat_string;
			cout << lua_color << "[Chat S] " << Curr_PChatString << white << endl;
			lua_on_chatted(CurrPlayer.curr_chat_string, p);


			//Forward message to discord.
			if (!isClient && networking)
			{
				string New_ChatString = CurrPlayer.curr_chat_string;
				replaceAll(New_ChatString, "<", "");
				replaceAll(New_ChatString, ">", ":");
				replaceAll(New_ChatString, ",", "");

				discord_message(New_ChatString);
			}
		}
		p++;
	}
	
	//Reset the chatstring after a while lol
	if (Time_ChatString[0] > 0 && networking)
	{
		Time_ChatString[0]--;
		if (!Time_ChatString[0])
		{
			Curr_PChatString = "";
		}
	}
}

void Chat_ClientSide()
{
	//Add a new line to chat. We have to check that it's different than hte previous one, and also, make sure it's not blank.
	if (Old_ChatString != Curr_PChatString && Curr_PChatString != "")
	{
		Add_Chat(Curr_PChatString);
		Old_ChatString = Curr_PChatString;

		cout << lua_color << "[Chat C] " << Curr_PChatString << white << endl;
	}

	//Pinnacle of shitcode
	for (int i = 0; i < 6; i++)
	{
		if (Time_ChatString[i] > 0)
		{
			Time_ChatString[i]--;
			if (!Time_ChatString[i])
			{
				Curr_ChatString[i] = "";
			}
		}
	}

	//If we are chatting
	if (Chatting)
	{
		//Letters
		for (int i = 0x41; i <= 0x5A; i++)
		{
			if (getKey(i))
			{
				char new_l = i + (state[SDL_SCANCODE_LSHIFT] ? 0x0 : 0x20);

				Typing_In_Chat += new_l;
			}
		}
		//Numbers
		if (!state[SDL_SCANCODE_LSHIFT])
		{
			for (int i = 0x30; i <= 0x39; i++)
			{
				if (getKey(i))
				{
					Typing_In_Chat += (i - 0x30) + '0';
				}
			}
		}
		else
		{
			if (getKey(0x31))
			{
				Typing_In_Chat += "!";
			}
		}
		//Spaces
		if (getKey(0x20))
		{
			Typing_In_Chat += " ";
		}

		//Special chars 21 2E 2C 2D 3C 3E
		if (getKey(VK_OEM_PERIOD)) { Typing_In_Chat += "."; }
		if (getKey(VK_OEM_COMMA)) { Typing_In_Chat += ","; }
		if (getKey(VK_OEM_MINUS)) { Typing_In_Chat += "-"; }
		if (getKey(VK_OEM_PLUS)) { Typing_In_Chat += "+"; }

		//Delete last chat
		if (getKey(0x08) && Typing_In_Chat.size() > 0) { Typing_In_Chat.pop_back(); }

		//Typing in chat limiter
		Typing_In_Chat = Typing_In_Chat.substr(0, min(49, int(Typing_In_Chat.length())));
	}

	//Da status :flushed:
	bool stat = Chatting ? (state[input_settings[9]]) : state[input_settings[10]];
	if (stat != pressed_start)
	{
		pressed_start = stat;
		if (stat)
		{
			Chatting = !Chatting;
			if (Chatting == false)
			{

				MPlayer& LocalPlayer = get_mario(SelfPlayerNumber);
				LocalPlayer.Chat(Typing_In_Chat);
			}
			else
			{
#if defined(_WIN32)
				KeyStates[0x54] = true;
#endif
			}
			Typing_In_Chat = "";
		}
	}
}

void ProcessChat()
{
	if (!isClient)
	{
		Chat_ServerSide();
	}

	if (isClient || !networking)
	{
		Chat_ClientSide();
	}
}

void Render_Chat()
{
	//Render chat
	if (Chatting || Time_ChatString[0] > 0)
	{
		memset(&VRAM[0xBC00], 0xFF, 896); //Clear bottom

		int y = 27;
		string Curr_Typing = (Typing_In_Chat + ((global_frame_counter % 20) > 10 ? "_" : ""));
		int Typing_Len = int(Curr_Typing.length());
		if (Chatting)
		{


			y = (Typing_Len > 31) ? 25 : 26;

			for (int i = 0; i < Curr_Typing.length(); i++)
			{

				uint_fast8_t new_l = char_to_smw(Curr_Typing.at(i));

				VRAM[0xB802 + (i * 2) + (y << 6)] = new_l;
				VRAM[0xB803 + (i * 2) + (y << 6)] = 6;

			}

		}
		for (int cc = 0; cc < 6; cc++)
		{
			string C_String = Curr_ChatString[cc];
			int C_len = int(Curr_ChatString[cc].length());
			y -= ((C_len > 31) ? 2 : 1);
			for (int i = 0; i < C_String.length(); i++)
			{

				uint_fast8_t new_l = char_to_smw(C_String.at(i));

				VRAM[0xB802 + (i * 2) + (y << 6)] = new_l;
				VRAM[0xB803 + (i * 2) + (y << 6)] = 6;

			}
		}

	}
}