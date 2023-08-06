#include "GeneralStuff.h"
#include "PacketFunctions.h"

//Quest stuff
DWORD* g_pdwQuestPointer = 0;
DWORD* g_pdwQuestNumberPointer = 0;
WORD g_wNextQuestOffset = 0x25;

std::vector<WORD> g_vecAutoQuestList =
{
90, //The Oppression of Ghosts
339, //Harvest 2
94, //Ugly Goblins 2
96, //A Bet with Marty 2
97, //Fantastic Items
379, //Athlete's Foot 3
100, //A Secret Request
198, //Mannequin Lips
111, //Grandma Myzen's Wrath
112, //The Unbreakable Chisel
116, //Delivery Delay
118, //Escape from Concealed Prison 2
199, //The Master Criminal
157, //Castle Lord's Goods 2
1127, //Reward for Dark Crystal 1
1128, //Reward for Dark Crystal 2
1129, //Reward for Dark Crystal 3
166, //Amnesia 2
1138, //Destruction of Giant Crystal
1131, //Tower Guardian 1
1133, //Tower Guardian 2
150, //Dreadful Forest Elf 2
200, //Hair Remover
125, //Navar Mercenaraies 2
2250, //Defeat the Lizard Men
2252, //Goodbye, Nightmare
2254, //Nina's Revenge
2272, //Hans' Request 1
217, //Monster Collector 2
388, //Slay the Fire Nixes
1150, //Ancient Heroes' Armory
391, //Contacting Wing Wing
392, //I Despise Moles
1157, //Guardian Monster
2279, //It's Them Again
395, //Hans in Love
2282, //Kidmon the Menace
399, //Tree of Jewels
2285, //Road To Riches
2226, //Attack of the Lava Gargoyles
2201, //My hunchs never proves wrong
2293, //Outlaw of the Swamp
2296, //The Threats of King Rhinoce
2298, //Food Thief!
2300, //Personal Revenge
4080, //Unextinguishable Fire
2227, //Black Bear Beatdown
2228, //Playing with Fire
2338, //Collecting Twister Crystals
2413, //Get Mysterious Herbs
2376, //Threats for Temporary Settlement 1
2378, //Threats for Temporary Settlement 2
3077, //Special Wrenches
3083, //Noisy Animals 2
3089, //The Best Technicians
3149, //Workshop master is making a weapon
3101, //Revenge of the Bera Guards
3155, //Selling Junk
3534, //Transportation in the Snow
3548, //Continues Suspicion
3557, //In need more firewood
3592, //Vale Megantereon's threat
20122, //Their health is in danger
20126, //Nighttime Disturbance
20136, //Protecting the Camp
20138, //Mushrooms for Ursula
20134, //Serben and his Wood
65209, //Help Yuna 1
20221, //The Predator Leader
65210, //Help Yuna 2
20228, //Finding Peace
65211, //Help Yuna 3
65249, //A Dark Source of Energy
65273, //Healing the Creatures
65305, //Corrupt Intervention
65274, //Collecting the Corrupted Souls
65283, //Invasion from the Other Side
65331, //Prune the Garden
65327, //Rotten Leafs
65322, //Bug Infestation
};
void turninQuest(char id[2]) {
	char* packet = new char[5];
	packet[0] = 0x04;
	packet[1] = 0x14;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];

	sendEncryptPacketFunc(*g_pdwSendNormal, packet, 5);

	delete packet;
}

void acceptQuestReward(char id[2]) {
	char* packet = new char[9];
	packet[0] = 0x08;
	packet[1] = 0x11;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];
	packet[5] = 0x00;
	packet[6] = 0x00;
	packet[7] = 0x00;
	packet[8] = 0x00;

	sendEncryptPacketFunc(*g_pdwSendNormal, packet, 9);

	delete packet;
}

void progressQuestText(char id[2]) {
	char* packet = new char[10];
	packet[0] = 0x09;
	packet[1] = 0x02;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];
	packet[5] = 0x02;
	packet[6] = 0x01;
	packet[7] = 0x00;
	packet[8] = 0x00;
	packet[9] = 0x00;

	sendEncryptPacketFunc(*g_pdwSendNormal, packet, 10);

	delete packet;
}
void acceptQuest(char id[2]) {
	char* packet = new char[5];
	packet[0] = 0x04;
	packet[1] = 0x14;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];

	sendEncryptPacketFunc(*g_pdwSendNormal, packet, 5);

	delete packet;
}
bool CheckQuests()
{
	IDtoBYTE umm;
	bool questTurnedIn = false;
	//06 40 24 b4 0f 33 14
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG tickNow, tickElapsed;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	//found near bottom, like add ecx,25

	//float CDmod = 3.0; for zombies?
	float CDmod = 1.7;

	WORD wCurrentQuestID = 0;
	DWORD dwNumberOfQuests = 0;
	DWORD dwCurrentQuestPointer = 0;
	if (isMemReadable((LPCVOID)(g_pdwQuestNumberPointer), sizeof(DWORD), (void*)(&dwNumberOfQuests)) == false)
	{
		return false;
	}
	if (isMemReadable((LPCVOID)(g_pdwQuestPointer), sizeof(DWORD), (void*)(&dwCurrentQuestPointer)) == false)
	{
		return false;
	}

	for (int i = 0; i < dwNumberOfQuests; i++) {
		BYTE byQuestStatus = 0;
		if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
		{
			return false;
		}
		if (byQuestStatus == 0x08)
		{
			//cout << i << "\t" << *questNumberPointer << "\t" << *(BYTE*)((*questPointer) + (g_wNextQuestOffset * i) + 2) << endl;
			//getchar();
			char* id = new char[2];
			if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i)), sizeof(WORD), (void*)(&wCurrentQuestID)) == false)
			{
				return false;
			}
			if (g_vecAutoQuestList.end() == std::find(g_vecAutoQuestList.begin(), g_vecAutoQuestList.end(), wCurrentQuestID))
			{
				continue;
			}
			id[0] = wCurrentQuestID & 0xFF;
			id[1] = wCurrentQuestID >> 8;
			//getchar();
			turninQuest(id);
			//getchar();
			//Sleep(1000);

			int j = 0;
			Timer tQuestTimeout;
			if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
			{
				return false;
			}
			while (byQuestStatus == 0x08)
			{
				//accept quest again if too much time passed
				if (tQuestTimeout.HasDurationPassed(1)) {
					turninQuest(id);
					QueryPerformanceCounter(&tcounter);
					tQuestTimeout.Reset();
				}
				acceptQuestReward(id);
				progressQuestText(id);
				j++;
				Sleep(10);
				if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
					if (GetAsyncKeyState(VK_HOME) & 0x8000) {
						//cout << "stopping..." << endl;
						Sleep(1000);
						return false;
					}
				}
				if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
				{
					return false;
				}
			}

			questTurnedIn = true;
			//return true;
			//Sleep(250);
			umm.bytes[0] = id[0]; //need to reverse endianness
			umm.bytes[1] = id[1];
			int questCounter = 0, loopCounter = 0;
			Sleep(400);
			tQuestTimeout.Reset();
			while (questCounter == 0 && loopCounter < 2000)
			{
				if (true == tQuestTimeout.HasDurationPassed(3))
				{
					//cout << "Quest timer timed out" << endl;
					return false;
				}
				if (GetAsyncKeyState(VK_INSERT) & 0x8000)
				{
					if (GetAsyncKeyState(VK_HOME) & 0x8000)
					{
						//cout << "Stopping quest search..." << endl;
						Sleep(1000);
						return false;
					}
				}

				if (isMemReadable((LPCVOID)(g_pdwQuestNumberPointer), sizeof(DWORD), (void*)(&dwNumberOfQuests)) == false)
				{
					return false;
				}

				for (int j = 0; j < dwNumberOfQuests; j++)
				{ //turned in quest, check again from beginning in case of levelup
					//if it is somehow already accepted then break;
					if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
					{
						return false;
					}
					if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i)), sizeof(WORD), (void*)(&wCurrentQuestID)) == false)
					{
						return false;
					}
					//if (*(BYTE*)((*questPointer) + (g_wNextQuestOffset * i) + 2) == 0x06 && *(WORD*)(*questPointer + (g_wNextQuestOffset * i)) == umm.id){
					if (byQuestStatus == 0x06 && wCurrentQuestID == *(WORD*)id)
					{
						questCounter++;

						break;

					}
					//if (*(BYTE*)((*questPointer) + (g_wNextQuestOffset * i) + 2) != 0x06 && *(WORD*)(*questPointer + (g_wNextQuestOffset * i)) == umm.id){
					if (byQuestStatus != 0x06 && wCurrentQuestID == *(WORD*)id)
					{
						questCounter++;
						for (int w = 0; w < 2; w++)
						{
							if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
							{
								return false;
							}

							if (byQuestStatus == 0x06) break;
							acceptQuest(id);

							int counter = 0;

							if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
							{
								return false;
							}

							//while (*(BYTE*)((*questPointer) + (g_wNextQuestOffset * i) + 2) != 0x06){
							while (byQuestStatus != 0x06)
							{
								if (true == tQuestTimeout.HasDurationPassed(3))
								{
									return false;
								}
								progressQuestText(id);
								//cout << "3\n";
								//Sleep(10);
								counter++;
								if (GetAsyncKeyState(VK_INSERT) & 0x8000)
								{
									if (GetAsyncKeyState(VK_HOME) & 0x8000)
									{
										Sleep(1000);
										return false;
									}
								}
								Sleep(10);
								if (isMemReadable((LPCVOID)(dwCurrentQuestPointer + (g_wNextQuestOffset * i) + 2), sizeof(BYTE), (void*)(&byQuestStatus)) == false)
								{
									return false;
								}
							}
						}
					}
				}
			}
			delete[] id;
		}
		if (questTurnedIn == true) break;
	}
	return questTurnedIn;
}
DWORD WINAPI CheckQuestsThread(LPVOID param)
{
	while (1)
	{
		if (IsDlgButtonChecked((HWND)param, DLG_AUTOTI))
		{
			CheckQuests();
			Sleep(100);
		}
		if (true == g_bLeftRingEquipper)
		{
			if (0xFF != g_bySlotToEquip)
			{
				// left from inventory, right to equip
				char carrSwapEquipSlots[] = { 0x04, 0x10, 0x30, g_bySlotToEquip, 0x10 };
				sendCrypt(*g_pdwSendNormal, carrSwapEquipSlots, sizeof(carrSwapEquipSlots), 0);
				g_bySlotToEquip = 0xFF;
				Sleep(200);
			}
		}
		if (false == g_strCharToTarget.empty())
		{
			WORD dwTargetID = GetTargetIDByName(g_strCharToTarget);
			if (0xFFFF == dwTargetID)
			{
				SetWindowText((HWND)param, L"Not found!");
			}
			else
			{
				SetWindowText((HWND)param, L"Found!");
				TargetEntity(dwTargetID);
			}
			g_strCharToTarget.clear();
		}
		if (false == g_strMassInviteFile.empty())
		{
			std::ifstream ifInviteFile;
			std::string sCurrentLine;
			ifInviteFile.open(g_strMassInviteFile.c_str(), std::ios::in);

			while (ifInviteFile.peek() != EOF)
			{
				getline(ifInviteFile, sCurrentLine);
				ExpoInvite(sCurrentLine);
				Sleep(10);
			}
			g_strMassInviteFile.clear();
		}
		if (false == g_strMassBanFile.empty())
		{
			std::ifstream ifBanFile;
			std::string sCurrentLine;
			ifBanFile.open(g_strMassBanFile.c_str(), std::ios::in);

			while (ifBanFile.peek() != EOF)
			{
				getline(ifBanFile, sCurrentLine);
				ExpoBan(sCurrentLine);
				Sleep(10);
			}
			g_strMassBanFile.clear();
		}
		if (true == g_bStartLHBot)
		{
			if (*g_pdwLHCoins < 1000)
			{
				MessageBox(g_hwndMain, L"Not enough coins!", L"Doh!", MB_OK);
			}
			else if (*g_pdwLHCoins < 5000 && g_wCapsuleIndex == CapsuleIDs::Red2)
			{
				MessageBox(g_hwndMain, L"Not enough coins!", L"Doh!", MB_OK);
			}
			else
			{
				for (auto ButtonID : g_vecLHButtons)
				{
					EnableWindow(GetDlgItem(g_hwndMain, ButtonID), false);
				}
				for (auto ButtonID : g_vecMainButtons)
				{
					EnableWindow(GetDlgItem(g_hwndMain, ButtonID), false);
				}
				EnableWindow(GetDlgItem(g_hwndMain, RBTN_LH), false);
				EnableWindow(GetDlgItem(g_hwndMain, RBTN_MAIN), false);
				AutoLH();
				for (auto ButtonID : g_vecLHButtons)
				{
					EnableWindow(GetDlgItem(g_hwndMain, ButtonID), true);
				}
				EnableWindow(GetDlgItem(g_hwndMain, RBTN_LH), true);
				EnableWindow(GetDlgItem(g_hwndMain, RBTN_MAIN), true);
			}

			g_bStartLHBot = false;
			SetWindowText(g_hwndLHRBTN, L"Start LH Bot");
		}
		if (true == g_bRaidHealer)
		{
			useSkill(&Heal, targetID, 0, 0);
			useSkill(&Rejuv, targetID, 0, 0);
			if (*g_pdwMana < 2000)
			{
				useSPStone();
				Sleep(500);
			}
		}
		if (true == g_bDeleteBuffs)
		{
			muxNoBuff.lock();
			if (vecBuffsGotten.empty() != true)
			{
				for (auto buff : vecBuffsGotten)
				{
					vecBuffsGottenCopy.push_back(buff);
				}
				vecBuffsGotten.clear();
			}
			muxNoBuff.unlock();

			if (vecBuffsGottenCopy.empty() != true)
			{
				for (auto buff : vecBuffsGottenCopy)
				{
					if (std::find(vecNoBuffList.begin(), vecNoBuffList.end(), buff) != vecNoBuffList.end())
					{
						//MessageBox(g_hwndMain, L"FOUND", std::to_wstring(buff).c_str(), MB_OK);
						srand((unsigned)time(0));
						char carrDeleteBuff[] = { 0x04, 0x54, 0x24, 0x14, 0x00 };
						*(WORD*)(&carrDeleteBuff[3]) = buff;
						int iRandomDelay = (rand() % 601);
						Sleep(800 + iRandomDelay);
						sendCrypt(*g_pdwSendNormal, carrDeleteBuff, sizeof(carrDeleteBuff), 0);
					}
				}
				vecBuffsGottenCopy.clear();
			}
		}
		Sleep(1);
	}

	return 0;
}