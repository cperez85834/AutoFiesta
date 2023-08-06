#pragma once
//Send stuff
BYTE* pbSendPacketBuffer = nullptr;
BYTE* pbRecvPacketBuffer = nullptr;
volatile int iSendBufferSize = 0;
volatile int iRecvBufferSize = 0;
unsigned char ucUnencryptedSendPacket[16384] = { 0 };
unsigned char ucRecvPacket[0x100000] = { 0 };
DWORD g_dwCurrentSendingSocket = 0;
DWORD g_dwCurrentRecvingSocket = 0;

std::atomic<bool> bLockEncryption = false;
DWORD* g_pdwSendNormal = 0;
DWORD* g_pdwCurrentEncryptionIndex = 0;
WORD* g_pwCurrentEncryptionIndex = 0;
WORD g_wCurrentEncryptionIndex = 0;
DWORD* g_pdwSendSpecial = 0;
DWORD* g_pdwCurrentEncryptionIndexSpecial = 0;
WORD* g_pwCurrentEncryptionIndexSpecial = 0;
//WORD g_wCurrentEncryptionIndex = 0;
unsigned char targetID[2] = { 0 };

// buff stuff
volatile bool g_bDeleteBuffs = false;
volatile bool g_bRaidHealer = false;

// LH stuff
WORD g_wCapsuleItem = 0;
WORD g_warrCapsuleIDs[] = { 58062, 63886, 58063, 872 };
WORD g_wCapsuleIndex = 0;
bool g_bStartLHBot = false;
enum CapsuleIDs
{
	Red1,
	Red2,
	Blue1,
	Blue2
};

//Base Pointers
DWORD g_dwFiestaBase = 0;
DWORD g_dwEntityPointer;
DWORD* g_pdwPlayerBase;
DWORD* g_pdwMana;
BYTE* pbyPlayerID;
DWORD g_dwCurrentWindow;
DWORD* g_pdwLHCoins;
char* g_pchCharacterClass;
DWORD dwEntityFirstOffset = 0x28;
DWORD dwEntitySecondOffset = 0x8;
DWORD dwEntityThirdOffset = 0x2C8;
DWORD dwEntityMobIdOffset = 0x3BA;
DWORD dwEntityMobHealthOffset = 0x288;
DWORD dwEntityInvisibleOffset = 0x24; //also 0x44?
DWORD dwEntityTitleOffset = 0xdc;
DWORD dwEntityMobMaxHealthOffset = 0x290;
DWORD dwEntityMobTargetIdOffset = 0x234;
DWORD dwEntityMobNameOffset = 0x236;
unsigned char playerID[2] = { 0xFF, 0xFF };

std::string g_strCharToTarget;
std::string g_strMassInviteFile;
std::string g_strMassBanFile;
typedef void(__stdcall* _encryptPacketFunc)(char* buffer, int numBytes);
_encryptPacketFunc encryptPacketFunc;
void sendEncryptPacketFunc(SOCKET socSendSocket, char* buffer, int numBytes) {
	WORD* pwPacketEncryptionIndex = nullptr;
	if (socSendSocket == *g_pdwSendNormal)
	{
		pwPacketEncryptionIndex = g_pwCurrentEncryptionIndex;
	}
	else
	{
		pwPacketEncryptionIndex = g_pwCurrentEncryptionIndexSpecial;
	}

    //encryption index for normal packet stuff
    char* packetToEncrypt = new char[numBytes];
    for (int i = 0; i < numBytes; i++)
    {
        packetToEncrypt[i] = buffer[i];
    }

    //cout << "Current encryption index: " << hex << (int)*pwPacketEncryptionIndex << endl;
    WORD newIndex = *pwPacketEncryptionIndex + numBytes - 1;
    if (newIndex >= 0x1F3) newIndex -= 0x1F3;
    //cout << "New encryption index should be: " << hex << newIndex << endl;
    __asm {
        pushad
        pushfd
        backy :  //cmp bInQueue, 1
                //je backy
                //mov bInQueue, 1
        mov ecx, pwPacketEncryptionIndex
    }
    //buffer should start at first byte to start encrypting
    encryptPacketFunc(packetToEncrypt + 1, numBytes - 1);
    __asm {
        popfd
        popad
    }
    newIndex = *pwPacketEncryptionIndex;

    send(socSendSocket, packetToEncrypt, numBytes, 0);
    // MUST RELEASE LOCK MANUALLY!!
    bLockEncryption = false;

    delete[] packetToEncrypt;

}
void GetPlayerID()
{
	pbyPlayerID = (BYTE*)(*(DWORD*)(*(DWORD*)(g_dwEntityPointer + dwEntityFirstOffset) + dwEntitySecondOffset) + dwEntityMobTargetIdOffset);
}
void sendCrypt(SOCKET s, char* buf, int len, int flags)
{
	UNREFERENCED_PARAMETER(flags);
	sendEncryptPacketFunc(s, buf, len);
}

void sendWhisper(std::string user, std::string message) {
	int packetLength = 0x18 + message.length();
	//1a 0c 20 00 43 61 73 68 61 64 65 72 00 00 00 00 00 ..Cashader.....
	//00 00 00 00 00 00 00 02 64 63 dc

	char* whisperPacket = new char[packetLength + 1];
	whisperPacket[0x00] = packetLength;
	whisperPacket[0x01] = 0x0c;
	whisperPacket[0x02] = 0x20;
	whisperPacket[0x03] = 0x00;
	for (int i = 0; i < user.length(); i++) {
		whisperPacket[i + 0x04] = user[i];
		//if at last character
		if (i == user.length() - 1) {
			//fill rest with zeros
			for (int j = user.length(); j < 20; j++) {
				whisperPacket[j + 4] = 0x00;
			}
		}
	}
	whisperPacket[0x18] = message.length();
	for (int i = 0; i < message.length(); i++) {
		whisperPacket[i + 0x19] = message[i];
	}
	/*whisperPacket[0x04] = 0x43;
	whisperPacket[0x05] = 0x61;
	whisperPacket[0x06] = 0x73;
	whisperPacket[0x07] = 0x68;
	whisperPacket[0x08] = 0x61;
	whisperPacket[0x09] = 0x64;
	whisperPacket[0x0a] = 0x65;
	whisperPacket[0x0b] = 0x72;
	whisperPacket[0x0c] = 0x00;
	whisperPacket[0x0d] = 0x00;
	whisperPacket[0x0e] = 0x00;
	whisperPacket[0x0f] = 0x00;
	whisperPacket[0x10] = 0x00;
	whisperPacket[0x11] = 0x00;
	whisperPacket[0x12] = 0x00;
	whisperPacket[0x13] = 0x00;
	whisperPacket[0x14] = 0x00;
	whisperPacket[0x15] = 0x00;
	whisperPacket[0x16] = 0x00;
	whisperPacket[0x17] = 0x00;
	whisperPacket[0x18] = 0x02;
	whisperPacket[0x19] = 0x64;
	whisperPacket[0x1a] = 0x63;*/

	sendCrypt(*g_pdwSendNormal, whisperPacket, packetLength + 1, 0);
	delete whisperPacket;
}

int GetEntityList(DWORD dwEntityList[1024])
{
	DWORD entityName, entityID;

	dwEntityList[0] = *(DWORD*)(g_dwEntityPointer + dwEntityFirstOffset);
	if (playerID[0] == 0xFF && playerID[1] == 0xFF) {
		DWORD player = *(DWORD*)(dwEntityList[0] + dwEntitySecondOffset);
		playerID[0] = *(BYTE*)(player + dwEntityMobTargetIdOffset);
		playerID[1] = *(BYTE*)(player + dwEntityMobTargetIdOffset + 1);
	}

	int entityCounter = 1;
	int i = 0;
	bool unique = true;
	while (1) {
		if (*(DWORD*)dwEntityList[i] != 0) {
			// Checks if already in list
			for (int j = 0; dwEntityList[j] != 0; j++) {
				if (*(DWORD*)dwEntityList[j] == dwEntityList[j]) {
					unique = false;
					break;
				}
			}

			// If unique, store it
			if (unique == true) {
				dwEntityList[entityCounter] = *(DWORD*)dwEntityList[i];
				entityCounter++;
			}
		}

		i++;
		if (dwEntityList[i] == 0) break;
		unique = true;
	}

	return entityCounter;
}
void TargetEntityList(int IDs[20], int optionalIDs[20], DWORD dwEntityList[1024], bool bKillAreaOverride = false, bool bIgnoreHP = false, bool bBoolSkipTarget = false)
{
	//char* carrTargetPacket = new char[5];
	DWORD dwEntityBase = 0;
	for (int i = 0; dwEntityList[i] != 0 && i < 1024; i++) {

		if (isMemReadable((LPCVOID)(dwEntityList[i] + dwEntitySecondOffset), sizeof(DWORD))) dwEntityBase = *(DWORD*)(dwEntityList[i] + dwEntitySecondOffset);
		else continue;

		WORD wEntityMobID = 0;
		if (isMemReadable((LPCVOID)(dwEntityBase + dwEntityMobIdOffset), sizeof(WORD))) wEntityMobID = *(WORD*)(dwEntityBase + dwEntityMobIdOffset);
		else continue;

		int iEntityHealth = 0;
		if (isMemReadable((LPCVOID)(dwEntityBase + dwEntityMobHealthOffset), sizeof(int))) iEntityHealth = *(int*)(dwEntityBase + dwEntityMobHealthOffset);
		else continue;


		//if (bKillAreaOverride == false)
		//{
		//	// checks if mob is within kill radius before targetting
		//	DWORD* dwEntityDeref = (DWORD*)(dwEntityBase + dwEntityThirdOffset);
		//	if (dwEntityBase == 0) continue;
		//	//cout << "bap5" << endl;
		//	DWORD dwEntityDetails = 0;
		//	if (isMemReadable((LPCVOID)dwEntityDeref, sizeof(DWORD))) dwEntityDetails = *(DWORD*)dwEntityDeref;
		//	else continue;
		//	//cout << "bap6" << endl;
		//	if (dwEntityDetails < g_dwFiestaBase) {
		//		//i++;
		//		continue;
		//	}
		//	float currentEntityX = 0;
		//	float currentEntityY = 0;
		//	if (isMemReadable((LPCVOID)(dwEntityDetails + 0x58), sizeof(float))) currentEntityX = *(float*)(dwEntityDetails + 0x58);
		//	else continue;
		//	if (isMemReadable((LPCVOID)(dwEntityDetails + 0x5C), sizeof(float))) currentEntityY = *(float*)(dwEntityDetails + 0x5C);
		//	else continue;

		//	//cout << "bap7" << endl;
		//	if (distanceFunc(
		//		(poiTargetCoords == nullptr) ? initialX : poiTargetCoords->x,
		//		(poiTargetCoords == nullptr) ? initialY : poiTargetCoords->y,
		//		currentEntityX,
		//		currentEntityY) > killRadius) {
		//		//cout << "Entity " << barrSelectionID << " too far" << endl;
		//		dwEntityList[i] = -1;
		//		continue;
		//	}
		//}

		//cout << "bap5" << endl;
		bool found = false;
		for (int j = 0; j < 20 && IDs[j] != 0; j++) {
			if (wEntityMobID == IDs[j]) {
				found = true;
				break;
			}
		}
		//cout << "bap6" << endl;
		if (found == false) {
			for (int j = 0; j < 20 && optionalIDs[j] != 0; j++) {
				if (wEntityMobID == optionalIDs[j]) {
					found = true;
					break;
				}
			}
		}
		//cout << "bap7" << endl;
		// Don't need to target in this GUI version
		//if (found == true) 
		//{
		//	if ((iEntityHealth == 0 || bIgnoreHP == true) && bBoolSkipTarget == false) {
		//		carrTargetPacket[0] = 0x04;
		//		carrTargetPacket[1] = 0x01;
		//		carrTargetPacket[2] = 0x24;
		//		carrTargetPacket[3] = *(BYTE*)(dwEntityBase + dwEntityMobTargetIdOffset);
		//		carrTargetPacket[4] = *(BYTE*)(dwEntityBase + dwEntityMobTargetIdOffset + 1);

		//		sendCrypt(*g_pdwSendNormal, carrTargetPacket, 5, 0);
		//		//getchar();
		//		Sleep(10);
		//	}
		//}
		//cout << "bap8" << endl;
	}
	//Sleep(1000);
	//delete carrTargetPacket;
}

WORD GetTargetIDByName(std::string strNameWanted, POINT* poiTarget = nullptr, bool bCheckInvis = false)
{
	int ids[20] = { 0 };
	int optionalIDs[20] = { 0 };
	ids[0] = 0xFFFF;
	DWORD entities[1024] = { 0 };
	int iEntityCounter = GetEntityList(entities);
	//cout << "Starting targetID by name" << endl;
	//TargetEntityList(ids, optionalIDs, entities, false, true, true);

	//cout << "Number of Entities: " << iEntityCounter << endl;
	Sleep(200);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++)
	{
		if (entities[i] == -1) continue;
		WORD membuf = 0;
		float coordbuff = 0;
		//cout << "sup ";
		//cout << hex << "entities[i]: " << entities[i] << endl;
		//cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;

		DWORD entityBase = 0;
		//cout << "umm1" << endl;
		if (isMemReadable((LPCVOID)(entities[i] + dwEntitySecondOffset), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + dwEntitySecondOffset);
		else continue;
		BYTE byIsInvisible = 0;
		if (isMemReadable((LPCVOID)(entityBase + 0x24), sizeof(BYTE), (void*)&byIsInvisible));

		//cout << "ummb" << endl;

		WORD currentMobID = 0;
		if (isMemReadable((LPCVOID)(entityBase + dwEntityMobIdOffset), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + dwEntityMobIdOffset);
		else continue;

		bool foundMob = false;
		for (int j = 0; j < 1; j++) {
			if (ids[j] > 0) {
				if (currentMobID == ids[j]) {
					foundMob = true;
					break;
				}
			}
		}

		BYTE byTitleFirstByte = 0xFF;
		if (isMemReadable((LPCVOID)(entityBase + dwEntityTitleOffset), sizeof(BYTE))) byTitleFirstByte = *(BYTE*)(entityBase + dwEntityTitleOffset);
		else continue;

		//Check the title of entity. 0x00 means its a mount
		if (byTitleFirstByte == 0x00)
		{
			continue;
		}


		//cout << "ugh sucky" << endl;
		if (foundMob == false) continue;
		//cout << "umm5" << endl;
		//cout << "wtf now dude" << endl;
		//cout << hex << "currentMobID: " << currentMobID << endl;

		//cout << "Getting Details" << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + dwEntityThirdOffset);
		if (entityDeref == 0) continue;

		/*		BYTE entityDetails1 = static_cast<unsigned>(*(BYTE*)entityDeref);
		cout << "1: " << entityDetails1 << endl;
		BYTE entityDetails2 = static_cast<unsigned>(*(BYTE*)(entityDeref + 1));
		cout << "2: " << entityDetails2 << endl;
		BYTE entityDetails3 = static_cast<unsigned>(*(BYTE*)(entityDeref + 2));
		cout << "3: " << entityDetails3 << endl;
		BYTE entityDetails4 = static_cast<unsigned>(*(BYTE*)(entityDeref + 3));
		cout << "4: " << entityDetails4 << endl;*/
		DWORD entityDetails = 0;
		if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;

		//DWORD entityDetails = *(DWORD*)entityDeref;
		//cout << hex << "entities[i]: " << entities[i] << endl;
		//cout << hex << "entityBase: " << entityBase << endl;
		//cout << hex << "entityDetails: " << entityDetails << endl;
		/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
		//i++;
		continue;
		}*/
		if (entityDetails < g_dwFiestaBase) {
			//i++;
			continue;
		}

		//cout << "umm3" << endl;
		//cout << "umma" << endl;
		WORD currentSelectionId = *(WORD*)(entityBase + dwEntityMobTargetIdOffset);
		//cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234 + ENTITY_OFFSET) << endl;
		//if (*(WORD*)(entityBase + 0x238) == 0) continue;
		////////////////////////////////////////////////////
		int* entityHealth = (int*)(entityBase + dwEntityMobHealthOffset);
		int* entityMaxHealth = (int*)(entityBase + dwEntityMobMaxHealthOffset);
		std::string strNameFound = reinterpret_cast<char*>((entityBase + dwEntityMobNameOffset));

		float* currentEntityX = (float*)(entityDetails + 0x58);
		float* currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;


		//float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		//cout << "umm1" << endl;
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		//float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
		//cout << "umm2" << endl;


		if (foundMob == true)
		{
			//if (distToPlayer >= killRadius) continue;
			if (strNameFound.size() > 40) continue;
			//cout << "Found name: " << strNameFound << "\tExpecting: " << strNameWanted << endl;
			if (strNameFound.compare(strNameWanted) == 0)
			{
				//cout << hex << "Distance to player: " << distToPlayer << endl;
				if (poiTarget != nullptr)
				{
					poiTarget->x = *currentEntityX;
					poiTarget->y = *currentEntityY;
					//cout << "X target: " << poiTarget->x << "\tY target: " << poiTarget->y << endl;
				}
				CloseHandle(hProcess);
				//cout << "Target ID: " << currentSelectionId << endl;
				return currentSelectionId;
			}

		}

	}
	//cout << "lollers" << endl;
	CloseHandle(hProcess);
	//cout << "lol 1\n";
	return 0xFFFF;
}

void TargetEntity(WORD wEntityTargetID)
{
	char target[] = { 0x04, 0x01, 0x24, 0, 0 };
	*reinterpret_cast<WORD*>(target + 3) = wEntityTargetID;

	sendCrypt(*g_pdwSendNormal, target, sizeof(target), 0);
}
void UseSlot(int slot)
{
	char packet[5] = { 0x04, 0x15, 0x30, slot, 0x09 };
	sendCrypt(*g_pdwSendNormal, packet, 5, 0);
}
void AutoSort()
{
	char carrAutosort[] = { 0x02, 0x4a, 0x30 };
	sendCrypt(*g_pdwSendNormal, carrAutosort, sizeof(carrAutosort), 0);
	Sleep(3000);
}
void DropItem(int iSlot)
{
	char carrDropItem[] = { 0x10, 0x07, 0x30, 0x10, 0x24, 0xff, 0x00, 0x00, 0x00, 0x8c, 0x23, 0x00, 0x00, 0x8f, 0x0e, 0x00, 0x00 };
	char carrAcceptDrop[] = { 0x03, 0x02, 0x3c, 0x00 };

	carrDropItem[3] = iSlot;
	sendCrypt(*g_pdwSendNormal, carrDropItem, sizeof(carrDropItem), 0);
	Sleep(200);
	for (int j = 0; j < 2; j++)
	{
		sendCrypt(*g_pdwSendNormal, carrAcceptDrop, sizeof(carrAcceptDrop), 0);
		Sleep(10);
	}
	Sleep(100);
}

void SellItem(int iSlot)
{
	char sellItem[8] = { 0x07, 0x06, 0x30, 0x26, 0x01, 0x00, 0x00, 0x00 };

	sellItem[3] = iSlot;
	sendCrypt(*g_pdwSendNormal, sellItem, sizeof(sellItem), 0);
	Sleep(300);
}
bool ReadInventoryIcon(int iSlot, DWORD dwInventoryWindow, WORD* wOutput)
{
	DWORD dwFirstSlotOffset = 0x15C; // Get this from the AOB instruction or search up icon what writes to
	DWORD dwIconOffset = 0x180;
	DWORD dwTemp = 0;
	WORD wItemIcon = 0;
	if (false == isMemReadable((LPCVOID)(dwInventoryWindow + (iSlot * 4) + dwFirstSlotOffset), 4, &dwTemp))
	{
		//sendWhisper("ReadyToWork", "yuh2 " + std::to_string(dwInventoryWindow + (iSlot * 4) + dwFirstSlotOffset));
		return false;
	}
	//sendWhisper("yuhyeetmage", "yuh3 " + std::to_string(dwTemp));

	if (false == isMemReadable((LPCVOID)(dwTemp + dwIconOffset), 2, &wItemIcon))
	{
		//sendWhisper("ReadyToWork", "yuh4 " + std::to_string(dwTemp + dwFirstSlotOffset));
		return false;
	}
	*wOutput = wItemIcon;
	return true;
}

int FindFirstEmptySlot(int iPageNum, DWORD dwInventoryWindow)
{
	WORD wItemIcon = 0;
	for (int i = 0; i < 24 * iPageNum; i++)
	{
		if (false == ReadInventoryIcon(i, dwInventoryWindow, &wItemIcon))
		{
			return -1;
		}
		if (0xFFFF == wItemIcon)
		{
			return i;
		}
		//DWORD dwTemp = 0;
		//if (false == isMemReadable((LPCVOID)(dwCurrentWindow + (i * 4) + dwFirstSlotOffset), 4, &dwTemp))
		//{
		//	//sendWhisper("yuhyeetmage", "yuh2 " + std::to_string(dwCurrentWindow + (i * 4) + dwFirstSlotOffset));
		//	break;
		//}
		////sendWhisper("yuhyeetmage", "yuh3 " + std::to_string(dwTemp));

		//if (false == isMemReadable((LPCVOID)(dwTemp + dwFirstSlotOffset), 4, &dwTemp))
		//{
		//	//sendWhisper("yuhyeetmage", "yuh4 " + std::to_string(dwTemp + dwFirstSlotOffset));
		//	break;
		//}
		//if (0xFFFF == *(WORD*)(*(DWORD*)(dwCurrentWindow + (i * 4) + dwFirstSlotOffset) + dwIconOffset))
		//{
		//	iFirstEmptySlot = i;
		//	//sendWhisper("yuhyeetmage", std::to_string(iFirstEmptySlot));
		//	break;
		//}
	}

	return -1;
}
DWORD GetInventoryWindow()
{
	DWORD dwFirstSlotOffset = 0x15C; // Get this from the AOB instruction or search up icon what writes to
	DWORD dwIconOffset = 0x180;
	DWORD dwTemp = 0;
	WORD wItemIcon = 0;
	DWORD dwInventoryWindow = *(DWORD*)g_dwCurrentWindow;


	if (false == isMemReadable((LPCVOID)(dwInventoryWindow + (0 * 4) + dwFirstSlotOffset), 4, &dwTemp))
	{
		return 0xFFFFFFFF;
	}
	//sendWhisper("yuhyeetmage", "yuh3 " + std::to_string(dwTemp));

	if (false == isMemReadable((LPCVOID)(dwTemp + dwIconOffset), 2, &wItemIcon))
	{
		return 0xFFFFFFFF;
	}

	return dwInventoryWindow;
}
void AutoLH()
{
	std::vector<WORD> vecKeepItems{62942,
		62943,
		62944,
		62945,
		62946,
		62947,
		62948,
		62949,
		62950,
		62951,
		62952,
		62953,
		62954};
	std::vector<WORD> vecDropItems{30106, 30107};

	if (true == IsDlgButtonChecked(g_hwndMain, DLG_SAVEHPPOTS))
	{
		vecKeepItems.push_back(3502);
		vecKeepItems.push_back(3503);
		vecKeepItems.push_back(3504);
	}
	if (true == IsDlgButtonChecked(g_hwndMain, DLG_SAVESPPOTS))
	{
		vecKeepItems.push_back(3507);
		vecKeepItems.push_back(3508);
		vecKeepItems.push_back(3509);
	}
	char buyFirstRedCapsule[9] = { 0x08, 0x03, 0x30, 0xce, 0xe2, 0x01, 0x00, 0x00, 0x00 };
	char buyFirstBlueCapsule[9] = { 0x08, 0x03, 0x30, 0xcf, 0xe2, 0x01, 0x00, 0x00, 0x00 };
	char buySecondRedCapsule[9] = { 0x08, 0x03, 0x30, 0x8e, 0xf9, 0x01, 0x00, 0x00, 0x00 };
	char buySecondBlueCapsule[9] = { 0x08, 0x03, 0x30, 0x68, 0x03, 0x01, 0x00, 0x00, 0x00 };
	char* carrCapsulesToBuy[]{ buyFirstRedCapsule, buySecondRedCapsule, buyFirstBlueCapsule, buySecondBlueCapsule };
	
	//hover mouse over inventory window and press spacebar
	int iMinCoins = ((g_wCapsuleIndex == CapsuleIDs::Red2) ? 5000 : 1000);
	DWORD dwCurrentWindow = 0;
	DWORD dwFirstSlotOffset = 0x15C; // Get this from the AOB instruction or search up icon what writes to
	DWORD dwIconOffset = 0x180;
	WORD wItemIcon = 0;
	bool bKeepItem = false;
	int ItemIndex = SendMessage(g_hwndComboBags, (UINT)CB_GETCURSEL,
		(WPARAM)0, (LPARAM)0);
	TCHAR  ListItem[256];
	(TCHAR)SendMessage(g_hwndComboBags, (UINT)CB_GETLBTEXT,
		(WPARAM)ItemIndex, (LPARAM)ListItem);
	//MessageBox(g_hwndMain, (LPCWSTR)ListItem, TEXT("Item Selected"), MB_OK);
	std::wstring wstrNumBags(ListItem);
	int iPageNum = wcstol(wstrNumBags.c_str(), nullptr, 10);
	EnableWindow(GetDlgItem(g_hwndMain, BTN_STARTLH), false);
	SetWindowText(g_hwndMain, L"Waiting for inventory window...");
	while (false == CheckForKey(VK_SPACE))
	{
		Sleep(10);
	}
	dwCurrentWindow = GetInventoryWindow();
	while (dwCurrentWindow == 0xFFFFFFFF)
	{
		dwCurrentWindow = GetInventoryWindow();
	}
	SetWindowText(g_hwndMain, L"AutoFiesta");
	SetWindowText(g_hwndLHRBTN, L"Stop LH Bot");
	EnableWindow(GetDlgItem(g_hwndMain, BTN_STARTLH), true);
	//sendWhisper("yuhyeetmage", "yuh1 " + std::to_string(dwCurrentWindow));
	//autosort inventory
	while (iMinCoins <= *g_pdwLHCoins)
	{
		AutoSort();

		//find first slot
		// assume number of pages = 2
		int iFirstEmptySlot = FindFirstEmptySlot(iPageNum, dwCurrentWindow);

		if (-1 == iFirstEmptySlot)
		{
			return;
		}

		for (int i = iFirstEmptySlot; i < (24 * iPageNum) - 1; i++)
		{
			//Buy LH capsule
			if (iMinCoins > *g_pdwLHCoins)
			{
				break;
			}
			sendCrypt(*g_pdwSendNormal, carrCapsulesToBuy[g_wCapsuleIndex], 9, 0);
			Sleep(500);
			if (!g_bStartLHBot) return;
		}
		Sleep(1000);
		wItemIcon = 0;
		// Keep buying until slot is filled
		if (false == ReadInventoryIcon((24 * iPageNum) - 2, dwCurrentWindow, &wItemIcon))
		{
			return;
		}
		while (0xFFFF == wItemIcon)
		{
			if (iMinCoins > *g_pdwLHCoins)
			{
				break;
			}
			sendCrypt(*g_pdwSendNormal, carrCapsulesToBuy[g_wCapsuleIndex], 9, 0);
			Sleep(850);
		
			if (false == ReadInventoryIcon((24 * iPageNum) - 2, dwCurrentWindow, &wItemIcon))
			{
				return;
			}
			if (!g_bStartLHBot) return;
		}
		int iCapsuleStartPoint = FindFirstEmptySlot(iPageNum, dwCurrentWindow) - 1;
		for (int i = iCapsuleStartPoint; i >= iFirstEmptySlot; i--)
		{
			int iOpenedItem = i + 1;
			// i is the slot where we are currently opening a capsule, so since we are starting backwards
			// then i + 1 is the slot where the opened item goes
			// Use LH capsule
			g_wCapsuleItem = 0;
			UseSlot(i);
			Timer tTimeout;
			int iAttempts = 0;
			while (0 == g_wCapsuleItem)
			{
				if (true == tTimeout.HasDurationPassed(2))
				{
					UseSlot(i);
					//SellItem(i);
					tTimeout.Reset();
					iAttempts++;
				}
				Sleep(1);
				if (!g_bStartLHBot) return;
				if (iAttempts == 5)
				{
					return;
				}
			}
			for (auto dropItem : vecDropItems)
			{
				if (dropItem == g_wCapsuleItem)
				{
					//sendWhisper("ReadyToWork", "Yay! " + std::to_string(g_wCapsuleItem) + " in slot " + std::to_string(iOpenedItem));
					Sleep(100);
					DropItem(iOpenedItem);
					break;
				}
				if (!g_bStartLHBot) return;
			}
			for (auto KeepItem : vecKeepItems)
			{
				if (KeepItem == g_wCapsuleItem)
				{
					//sendWhisper("ReadyToWork", "Woo! " + std::to_string(g_wCapsuleItem) + " in slot " + std::to_string(iOpenedItem));
					bKeepItem = true;
					break;
				}
				if (!g_bStartLHBot) return;
			}
			if (false == bKeepItem)
			{
				Sleep(100);
				SellItem(iOpenedItem);
			}
			bKeepItem = false;
			//Sleep(500);
		}
	}
}
void ExpoBan(std::string strName) {
	//16 18 b0 5f 5f 5f 79 75 63 6b 79 5f 5f 5f 00 00 ..°___yucky___..
	//00 00 00 00 00 00 00

	char* packet = new char[23];
	packet[0] = 0x16;
	packet[1] = 0x18;
	packet[2] = 0xb0;
	for (int i = 3; i < 23; i++) {
		if ((i - 3) < strName.length()) packet[i] = strName[i - 3];
		else packet[i] = 0x00;
	}

	sendCrypt(*g_pdwSendSpecial, packet, 23, 1);
	delete[] packet;

}
void ExpoInvite(std::string strName) {
	//16 0c b0 50 65 6e 63 69 6c 76 65 73 74 65 72 00 00 .8Pencilvester.. send invite
	//00 00 00 00 00 00

	char* packet = new char[23];
	packet[0] = 0x16;
	packet[1] = 0x0c;
	packet[2] = 0xb0;
	for (int i = 3; i < 23; i++) {
		if ((i - 3) < strName.length()) packet[i] = strName[i - 3];
		else packet[i] = 0x00;
	}

	sendCrypt(*g_pdwSendSpecial, packet, 23, 1);
	delete[] packet;

}
bool useSPStone() {
	char packet[3] = { 0x02, 0x09, 0x50 };

	sendCrypt(*g_pdwSendNormal, packet, 3, 0);

	return true;
}
void EnableBattleState()
{
	char carrBattleState[4] = { 0x03, 0x08, 0x20, 0x02 };
	sendCrypt(*g_pdwSendNormal, carrBattleState, 4, 0);
}
bool useSkill(skill* toUse, unsigned char mobId[2], float X, float Y) {

	//06 40 24 b4 0f 33 14
	bool used = 0;
	if (*(WORD*)toUse->id == 0)
	{
		//MessageBox(g_hwndMain, L"Bruh", L"yuh1", MB_OK);
		return true;
	}
	QueryPerformanceFrequency(&g_liSkillTimerCounter);
	g_llSkillFreq = g_liSkillTimerCounter.QuadPart;

	QueryPerformanceCounter(&g_liSkillTimerCounter);
	g_llSkillCastTick = g_liSkillTimerCounter.QuadPart;
	//float CDmod = 3.0; for zombies?
	float CDmod = 1.7;
	if ((g_llSkillCastTick - globalCooldown) / (g_llSkillFreq / 1000) <= 300) return false;

	/*if (*mana >= lastMana && lastSkill != NULL){
		if (*mana - lastMana >= 18) lastSkill->tickAtCast = 1; //my sp recovery is 15...
	}*/

	if (X == 0 && Y == 0)
	{
		//MessageBox(g_hwndMain, L"Bruh", L"yuh2", MB_OK);
		char* packet = new char[7];
		//cout << "time: " << (ticknow - toUse->tickAtCast) / (freq / 1000) << "\tcooldown: " << toUse.cooldown << endl;
		if (((g_llSkillCastTick - toUse->tickAtCast) / (g_llSkillFreq / 1000)) >= (toUse->cooldown * 1000) || toUse->tickAtCast == 0)
		{
			//MessageBox(g_hwndMain, L"Bruh", L"yuh3", MB_OK);
			//cout << dec << "Casting skill " << *(WORD*)toUse->id << "(Cooldown: " << toUse->cooldown << ", time elapsed: " << ((g_llSkillCastTick - toUse->tickAtCast) / (g_llSkillFreq / 1)) << ")" << endl;
			if (1)
			{
				packet[0] = 0x06;
				packet[1] = 0x40;
				packet[2] = 0x24;
				packet[3] = toUse->id[0];
				packet[4] = toUse->id[1];
				packet[5] = mobId[0];
				packet[6] = mobId[1];

				sendCrypt(*g_pdwSendNormal, packet, 7, 0);
			}

			Sleep(400);
			QueryPerformanceCounter(&g_liSkillTimerCounter);
			
			used = true;
			globalCooldown = g_liSkillTimerCounter.QuadPart;
			toUse->tickAtCast = g_liSkillTimerCounter.QuadPart;
			if (toUse->iAnimationTime > 0)
			{
				Sleep(toUse->iAnimationTime);
			}

		}
		delete[] packet;
	}
	//0c 41 24 b8 10 87 3e 00 00 ca 38 00 00
	else
	{
		char* packet = new char[13];
		IDtoBYTE xpos, ypos;
		xpos.id = (int)X;
		ypos.id = (int)Y;
		if (((g_llSkillCastTick - toUse->tickAtCast) / (g_llSkillFreq / 1000)) >= (toUse->cooldown * 1000) || toUse->tickAtCast == 0)
		{
			//cout << "Casting skill " << *(WORD*)toUse->id << endl;
			packet[0] = 0x0C;
			packet[1] = 0x41;
			packet[2] = 0x24;
			packet[3] = toUse->id[0];
			packet[4] = toUse->id[1];
			packet[5] = xpos.bytes[0];
			packet[6] = xpos.bytes[1];
			packet[7] = 0x00;
			packet[8] = 0x00;
			packet[9] = ypos.bytes[0];
			packet[10] = ypos.bytes[1];
			packet[11] = 0x00;
			packet[12] = 0x00;

			sendCrypt(*g_pdwSendNormal, packet, 13, 0);

			//sendCrypt(*pdwSendNormal, packet, 13, 0);
			//ReleaseKey('w', file_kbd);
			Sleep(500);
			QueryPerformanceCounter(&g_liSkillTimerCounter);
			toUse->tickAtCast = g_liSkillTimerCounter.QuadPart;
			used = true;

			globalCooldown = g_liSkillTimerCounter.QuadPart;

		}
		delete[] packet;
	}

	return used;
}