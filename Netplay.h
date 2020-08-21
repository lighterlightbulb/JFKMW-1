#pragma once

#define Header_AttemptJoin 0x00
#define Header_UpdatePlayerData 0x01
#define Header_GlobalUpdate 0x02
#define Header_Connection 0x03
#define Header_ConnectData 0x04
#define Header_RAM 0x05
#define Header_MusicData 0x06
#define Header_FailedToConnect 0x07

#define Delay_Connection 200

string latest_error = "";

sf::Socket::Status last_network_status;
sf::Socket::Status receiveWithTimeout(JFKMWSocket& socket, sf::Packet& packet, sf::Time timeout, bool blocking)
{
	sf::SocketSelector selector;
	selector.add(socket);
	if (selector.wait(timeout)){
		socket.setBlocking(blocking);
		last_network_status = socket.receive(packet);
		return last_network_status;
	}
	else {
		return sf::Socket::NotReady;
	}
}

bool validated_connection = false;

uint_fast8_t GetAmountOfPlayers() {
	PlayerAmount = uint_fast8_t(clients.size()); CheckForPlayers();
	return PlayerAmount;
}

void PreparePacket(uint8_t header) {
	CurrentPacket.clear(); CurrentPacket << header;
	CurrentPacket_header = header;
}

/*

Mario Data

*/
void put_mario_data_in(MPlayer& CurrentMario, bool d = true)
{
	//copypaste
	CurrentPacket << float(CurrentMario.x); CurrentPacket << float(CurrentMario.y);
	CurrentPacket << int_fast8_t(CurrentMario.X_SPEED * 16.0); CurrentPacket << int_fast8_t(CurrentMario.Y_SPEED * 16.0);
	
	/*
		Deez are bools 
	*/
	uint_fast8_t bools_1 = 0;
	bools_1 += CurrentMario.ON_FL;
	bools_1 += CurrentMario.DEAD << 1;
	bools_1 += CurrentMario.jump_is_spin << 2;
	bools_1 += CurrentMario.in_pipe << 3;
	bools_1 += CurrentMario.SLIDING << 4;
	bools_1 += CurrentMario.STATE << 5;
	CurrentPacket << bools_1;
	
	if (d)
	{
		CurrentPacket << CurrentMario.GRABBED_SPRITE;
	}
	CurrentPacket << CurrentMario.INVINCIBILITY_FRAMES; CurrentPacket << CurrentMario.DEATH_TIMER;

	CurrentPacket << CurrentMario.to_scale; CurrentPacket << CurrentMario.SKIDDING; CurrentPacket << CurrentMario.P_METER;
	CurrentPacket << CurrentMario.WALKING_DIR;

	CurrentPacket << uint_fast8_t(CurrentMario.FRM);
	CurrentPacket << int_fast16_t(CurrentMario.CAMERA_X); CurrentPacket << int_fast16_t(CurrentMario.CAMERA_Y);

	CurrentPacket << CurrentMario.SLOPE_TYPE;
	CurrentPacket << CurrentMario.KO_counter; CurrentPacket << CurrentMario.WO_counter;

	CurrentPacket << CurrentMario.skin;
	CurrentPacket << CurrentMario.pipe_speed_x; CurrentPacket << CurrentMario.pipe_speed_y;

	uint_fast16_t m_state_1 = (CurrentMario.mouse_x & 0x3FFF) + (CurrentMario.mouse_state[0] << 15) + (CurrentMario.mouse_state[2] << 14);
	uint_fast16_t m_state_2 = (CurrentMario.mouse_y & 0x3FFF) + (CurrentMario.mouse_state[1] << 15) + (CurrentMario.mouse_state[3] << 14);
	CurrentPacket << m_state_1; CurrentPacket << m_state_2;

	uint_fast8_t input_d = 0;
	for (int inputs = 0; inputs < total_inputs; inputs++)
	{
		input_d |= (CurrentMario.p_pad[inputs] << inputs);
	}
	CurrentPacket << input_d;

	for (int plr_name = 0; plr_name < player_name_size; plr_name++)
	{
		CurrentPacket << uint_fast8_t(CurrentMario.player_name_cut[plr_name]);
	}

	CurrentPacket << CurrentMario.curr_chat_string;
	//copypaste
}

void take_mario_data(MPlayer& CurrentMario, bool d = true)
{
	//copypaste
	float x, y; CurrentPacket >> x; CurrentPacket >> y;
	int_fast8_t X_SPEED, Y_SPEED; CurrentPacket >> X_SPEED; CurrentPacket >> Y_SPEED; 
	CurrentMario.X_SPEED = double(X_SPEED) / 16.0; CurrentMario.Y_SPEED = double(Y_SPEED) / 16.0; CurrentMario.x = x; CurrentMario.y = y;

	/*
		Deez are bools
	*/
	uint_fast8_t bools;
	CurrentPacket >> bools;
	CurrentMario.ON_FL = bools & 1;
	CurrentMario.DEAD = bools & 2;
	CurrentMario.jump_is_spin = bools & 4;
	CurrentMario.in_pipe = bools & 8;
	CurrentMario.SLIDING = bools & 16;
	CurrentMario.STATE = bools >> 5;

	if (d)
	{
		CurrentPacket >> CurrentMario.GRABBED_SPRITE;
	}
	CurrentPacket >> CurrentMario.INVINCIBILITY_FRAMES; CurrentPacket >> CurrentMario.DEATH_TIMER;

	CurrentPacket >> CurrentMario.to_scale; CurrentPacket >> CurrentMario.SKIDDING; CurrentPacket >> CurrentMario.P_METER;
	CurrentPacket >> CurrentMario.WALKING_DIR;


	uint_fast8_t FRM; CurrentPacket >> FRM;
	CurrentMario.FRM = FRM;

	int_fast16_t CAMERA_X, CAMERA_Y; CurrentPacket >> CAMERA_X; CurrentPacket >> CAMERA_Y;
	CurrentMario.CAMERA_X = CAMERA_X; CurrentMario.CAMERA_Y = CAMERA_Y;

	CurrentPacket >> CurrentMario.SLOPE_TYPE;
	CurrentPacket >> CurrentMario.KO_counter; CurrentPacket >> CurrentMario.WO_counter;

	CurrentPacket >> CurrentMario.skin;
	CurrentPacket >> CurrentMario.pipe_speed_x; CurrentPacket >> CurrentMario.pipe_speed_y;

	uint_fast16_t m_state_1, m_state_2;
	CurrentPacket >> m_state_1; CurrentPacket >> m_state_2;
	CurrentMario.mouse_state[0] = (m_state_1 >> 15) & 1;
	CurrentMario.mouse_state[1] = (m_state_2 >> 15) & 1;
	CurrentMario.mouse_state[2] = (m_state_1 >> 14) & 1;
	CurrentMario.mouse_state[3] = (m_state_2 >> 14) & 1;
	CurrentMario.mouse_x = m_state_1 & 0x3FFF;
	CurrentMario.mouse_y = m_state_2 & 0x3FFF;


	uint_fast8_t input_d = 0;
	CurrentPacket >> input_d;
	for (int inputs = 0; inputs < total_inputs; inputs++)
	{
		CurrentMario.p_pad[inputs] = (input_d >> inputs) & 1;
	}

	for (int plr_name = 0; plr_name < player_name_size; plr_name++)
	{
		uint_fast8_t new_l = 0;
		CurrentPacket >> new_l;
		CurrentMario.player_name_cut[plr_name] = char(new_l);
	}

	CurrentPacket >> CurrentMario.curr_chat_string;
	//copypaste
}



/*

Pack Mario Data

*/

void pack_mario_data(uint_fast8_t skip = 1) {
	if (!isClient) {
		CurrentPacket << PlayerAmount;
		uint_fast8_t plrNum = 1;
		for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item) { 
			if (plrNum != skip) {
				put_mario_data_in(*item);
			}
			else
			{
				MPlayer& CurrentMario = *item;
				CurrentPacket << CurrentMario.GRABBED_SPRITE;
				bool received_mouse_status = CurrentMario.mouse_state[2] || CurrentMario.mouse_state[3];
				CurrentPacket << received_mouse_status;
			}
			plrNum += 1;
		}
	}
	else {
		CurrentPacket << SelfPlayerNumber;
		CurrentPacket << latest_sync;
		put_mario_data_in(get_mario(SelfPlayerNumber), false);
	}
}


/*

Disconnection handler

*/


void HandleDisconnection(JFKMWSocket* ToSend = nullptr) {
	if (ToSend != nullptr)
	{

		cout << blue << "[Server] " << ToSend->username << " (" << ToSend->getRemoteAddress() << ") has disconnected." << white << endl;
		if (find(clients.begin(), clients.end(), ToSend) != clients.end()) {
			discord_message("**" + ToSend->username + " just disconnected.**");

			Send_Chat(ToSend->username + " left");

			clients.erase(remove(clients.begin(), clients.end(), ToSend));

			selector.remove(*ToSend);

			ToSend->disconnect();
			last_network_status = sf::Socket::Error;

			delete ToSend;
		}
		else
		{
			ToSend->disconnect();
			last_network_status = sf::Socket::Error;
		}


	}
}


/*

Packet Sending

*/

void send_not_blocking(JFKMWSocket* ToSend = nullptr) {
	if (ToSend->send(CurrentPacket) == sf::Socket::Disconnected) {
		if (!isClient) {
			HandleDisconnection(ToSend);
		}
		else {
			disconnected = true;
		}
	}
}

void SendPacket(JFKMWSocket* ToSend = nullptr) {
	//Server will send a packet.
	//Players will receive it.
	//Client will send a packet to server, server receives it.

	if (!isClient) {
		data_size_now += int(CurrentPacket.getDataSize());
		if (ToSend != nullptr) {
			ToSend->setBlocking(false);
			send_not_blocking(ToSend);
			ToSend->setBlocking(true);
		}
		else
		{
			for (int i = 0; i < clients.size(); ++i) {
				ToSend = clients[i];	
				ToSend->setBlocking(false);
				send_not_blocking(ToSend);
				ToSend->setBlocking(true);
			}
		}
	}
	else
	{
		send_not_blocking(&socketG);
	}
}



void ReceivePacket(JFKMWSocket &whoSentThis, bool for_validating = false)
{
	//cout << last_network_status << endl;
	CurrentPacket >> CurrentPacket_header;


	data_size_current += int(CurrentPacket.getDataSize());

	if (for_validating == true)
	{
		validated_connection = false;
		if (CurrentPacket_header == Header_AttemptJoin && CurrentPacket.getDataSize() < 64) //Why would the verification packet be bigger than 64 bytes? It's only username and checksum so.
		{
			cout << blue << "[Client] Receiving verification.." << white << endl;

			string validation;

			CurrentPacket >> username;
			CurrentPacket >> validation;
			if (validation == GAME_VERSION)
			{
				cout << blue << "[Client] " << username << " has passed verification." << white << endl;
				validated_connection = true;
				return;
			}
			cout << blue << "[Client] " << username << " failed verification. Outdated version or invalid chk? V/C = " << validation << white << endl;
			latest_error = "Outdated version";
		}
		return;
	}
	if (!isClient && CurrentPacket.getDataSize() < player_expected_packet_size) //Player only sends things to update their data, so they shouldn't send stuff that big.
	{
		cout << blue << "[Network] Something's weird, " << whoSentThis.getRemoteAddress() << " sent a packet that wasn't " + to_string(player_expected_packet_size) + " bytes! (" << dec << CurrentPacket.getDataSize() << " bytes) Disconnecting!" << white << endl;
		HandleDisconnection(&whoSentThis);
		return;
	}

	if (!isClient && CurrentPacket.getDataSize() > (player_expected_packet_size + 70)) //lol They're trying to crash the server
	{
		cout << blue << "[Network] " << whoSentThis.getRemoteAddress() << " Get the fuck out of my porch bitch. Disconnecting." << white << endl;

		PreparePacket(Header_FailedToConnect);
		CurrentPacket << "Kicked.";
		SendPacket(&whoSentThis);

		HandleDisconnection(&whoSentThis);

		return;
	}

	/*
		SERVER BEHAVIOUR
	*/
	if (!isClient)
	{

		/*

		UPDATE PLAYER DATA

		*/
		if (CurrentPacket_header == Header_UpdatePlayerData)
		{
			uint_fast8_t PlrNum = 1; CurrentPacket >> PlrNum;
			uint_fast8_t sync; CurrentPacket >> sync; whoSentThis.latest_sync_p = sync;

			if (PlrNum > (clients.size()+1) || PlrNum == 0)
			{
				cout << blue << "[Network] Something's weird, " << whoSentThis.getRemoteAddress() << " sent a invalid player packet. Disconnecting!" << white << endl;
				HandleDisconnection(&whoSentThis);
				return;
			}
			
			PlayerAmount = int(clients.size()); CheckForPlayers();
			take_mario_data(get_mario(PlrNum), false);
			//cout << "attempting to update player " << PlrNum << endl;
		}
	}
	else
	/*
	CLIENT BEHAVIOUR
	*/
	{
		/*

		UPDATE PLAYER DATA

		*/

		if (CurrentPacket_header == Header_MusicData)
		{
			ReceiveMusic();
			Sync_Server_RAM(false);
		}

		if (CurrentPacket_header == Header_GlobalUpdate)
		{
			int_fast32_t timestamp = 0;
			CurrentPacket >> timestamp;
			chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(
				chrono::system_clock::now().time_since_epoch()
				);
			latest_server_response = int_fast32_t(ms.count()) - timestamp;
			CurrentPacket >> pvp;
			CurrentPacket >> SelfPlayerNumber; //Me
			CurrentPacket >> death_timer[SelfPlayerNumber - 1];


			CurrentPacket >> PlayerAmount; //Update Plr Amount
			CheckForPlayers(); //have to update the mario list. so it fits.



			uint_fast8_t num = 1;
			for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item) {
				if (num != SelfPlayerNumber) {
					take_mario_data(*item);
				}
				else
				{
					MPlayer& CurrentMario = *item;
					CurrentPacket >> CurrentMario.GRABBED_SPRITE;


					bool received_mouse_status = false;
					CurrentPacket >> received_mouse_status;
					if (received_mouse_status)
					{
						//cout << "Server received mouse status. Resetting" << endl;
						mouse_w_up = false;
						mouse_w_down = false;
					}
				}
				num += 1;
			}

			CurrentPacket >> recent_big_change;
			Sync_Server_RAM(!recent_big_change);

			CurrentPacket >> Curr_PChatString;

		}

		if (CurrentPacket_header == Header_ConnectData)
		{
			//cout << blue << "[Client] Received connection data." << white << endl;


			CurrentPacket >> PlayerAmount; //Update Plr Amount
			CheckForPlayers(); //have to update the mario list. so it fits.
			//cout << blue << "[Client] Receiving server RAM" << white << endl;
			Sync_Server_RAM(false);
			//cout << blue << "[Client] Receiving music" << white << endl;
			ReceiveMusic(true);
			validated_connection = true;
			cout << blue << "[Client] Received." << white << endl;
		}

		if (CurrentPacket_header == Header_FailedToConnect)
		{
			string msg;
			CurrentPacket >> msg;
			latest_error = msg;
			cout << red << "[Network] Received disconnection reason from server : " << msg << white << endl;
			validated_connection = false;
		}

	}
}

bool receive_all_packets(JFKMWSocket& socket, bool slower = false, bool for_validating = false)
{
	int current_pack = 0;

	while (receiveWithTimeout(socket, CurrentPacket, sf::milliseconds(slower ? 2000 : packet_wait_time), !for_validating) != sf::Socket::NotReady)
	{
		if (last_network_status == sf::Socket::Disconnected)
		{
			if (!isClient) {
				HandleDisconnection(&socket);
				return false;
			}
			else {
				disconnected = true;
				return false;
			}
		}
		current_pack += 1;
		ReceivePacket(socket, for_validating);
		if (isClient && validated_connection)
		{
			return true;
		}
	}
	if (for_validating && !validated_connection)
	{
		return false;
	}
	if (!isClient)
	{
		return !((last_network_status == sf::Socket::Error) || (last_network_status == sf::Socket::Disconnected));
	}
	return !disconnected;
}

void PendingConnection()
{
	// The listener is ready: there is a pending connection
	//cout << blue << "[Server] Pending connection..." << white << endl;


	JFKMWSocket* client = new JFKMWSocket;
	if (listener.accept(*client) == sf::Socket::Done)
	{
		uint_fast8_t NewPlayerNumber = GetAmountOfPlayers() + 1;
		username = "Unknown";

		cout << blue << "[Server] " << client->getRemoteAddress() << " (assigned to Player " << int(NewPlayerNumber) << ") is trying to connect.. " << white << endl;


		PreparePacket(Header_Connection); CurrentPacket << NewPlayerNumber; SendPacket(client);
		sf::sleep(sf::milliseconds(500));

		validated_connection = false;
		receive_all_packets(*client, true, true);

		if (validated_connection)
		{

			// Add the new client to the clients list
			client->username = username;

			clients.push_back(client); selector.add(*client); GetAmountOfPlayers();

			PreparePacket(Header_ConnectData);
			CurrentPacket << PlayerAmount;
			Push_Server_RAM(false);
			SendMusic();
			SendPacket(client);


			GetAmountOfPlayers(); //Update player amount.

			//Send msg to discord
			discord_message("**" + username + " has connected.**");

			Send_Chat(username + " joined");
		}
		else
		{
			latest_error = "Invalid information sent";

			cout << "Timed out or sent invalid information. Disconnecting." << white << endl;
			PreparePacket(Header_FailedToConnect);
			CurrentPacket << latest_error;
			SendPacket(client);


			latest_error = "Unknown";
			client->disconnect();

			delete client;

		}

	}
	else
	{
		// Error, we won't get a new connection, delete the socket
		delete client;
	}
}


void Server_To_Clients()
{
	GetAmountOfPlayers();

	if (clients.size() > 0)
	{
		uint_fast8_t PlrNumber = 1;

		for (int i = 0; i < clients.size(); ++i) {
			JFKMWSocket& client = *clients[i];
			receive_all_packets(client);
			
			if (clients.size() < 1 || last_network_status == sf::Socket::Error || last_network_status == sf::Socket::Disconnected)
			{
				//cout << blue << "[Server] Attempting to recover from player failure. Will stop communication." << white << endl;
				break;
			}


			PreparePacket(Header_GlobalUpdate);
			chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(
				chrono::system_clock::now().time_since_epoch()
				);

			CurrentPacket << int_fast32_t(ms.count());
			CurrentPacket << pvp;
			CurrentPacket << PlrNumber;
			CurrentPacket << death_timer[i];


			pack_mario_data(PlrNumber);

			bool d_change = client.latest_sync_p != latest_sync || recent_big_change;
			CurrentPacket << d_change;
			Push_Server_RAM(!d_change);
			CurrentPacket << Curr_PChatString;
			SendPacket(&client);




			if (clients.size() < 1 || last_network_status == sf::Socket::Error || last_network_status == sf::Socket::Disconnected)
			{
				//cout << blue << "[Server] Attempting to recover from player failure. Will stop communication." << white << endl;
				break;
			}

			PlrNumber++;

		}

		if (need_sync_music)
		{
			PreparePacket(Header_MusicData);
			SendMusic();
			Push_Server_RAM(false);
			SendPacket();
			need_sync_music = false;
		}

		if ((int(PlrNumber)-1) == clients.size()) //Yea we did it.
		{
			ASM.Reset_ASM_Variables_Server();
			Set_Server_RAM();
		}

		if (recent_big_change)
		{
			recent_big_change = false;
			cout << green << "[Network] Synced RAM and game." << endl;
		}


	}
}


void NetWorkLoop()
{
	if (!isClient)
	{
		listener.listen(PORT); selector.add(listener);
		cout << blue << "[Server] Server is running on port " << dec << PORT << white << endl;

		//Discord
		discord_message("There's a JFK mario world server version " + GAME_VERSION + " being hosted, the ip is **" + sf::IpAddress::getPublicAddress(sf::seconds(5.f)).toString() + "**, port is " + to_string(PORT));

		// Endless loop that waits for new connections
		while (!quit)
		{
			if (selector.wait(sf::milliseconds(network_update_rate)))
			{
				if (clients.size() > 0)
				{
					network_update_rate = network_update_rate_c / (unsigned int)clients.size();
					packet_wait_time = packet_wait_time_c / (unsigned int)clients.size();
				}

				// Test the listener
				if (selector.isReady(listener))
				{
					PendingConnection();
				}
				else
				{
					Server_To_Clients();
				}
			}
		}
	}
	else
	{
		/*
		CLIENT MODE
		*/
		while (Mario.size() == 0) {
			receive_all_packets(socketG);
		}

		cout << blue << "[Network] Connected to server. " << int(PlayerAmount) << " player(s) connected." << endl;
		while (!quit && !disconnected)
		{
			receive_all_packets(socketG);

			PreparePacket(Header_UpdatePlayerData); pack_mario_data(); SendPacket();
		}
		receive_all_packets(socketG);
	}
}

bool ConnectClient(void)
{
	if (socketG.connect(ip, PORT) != sf::Socket::Disconnected)
	{
		sf::sleep(sf::milliseconds(250));
		PreparePacket(Header_AttemptJoin);
		CurrentPacket << username; //CurrentPacket << da_epical_function_lol();
		CurrentPacket << GAME_VERSION;
		SendPacket();
		sf::sleep(sf::milliseconds(1000));
		receive_all_packets(socketG, true);
		validated_connection = false;
		if (disconnected)
		{
			return false;
		}
		CheckForPlayers();
		cout << blue << "[Network] Connected to " << ip << ":" << dec <<  PORT << white << endl;
		return true;
	}
	else
	{
		socketG.disconnect();
		return false;
	}
}