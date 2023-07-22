#pragma once
//Send stuff
BYTE* pbSendPacketBuffer = nullptr;
int iSendBufferSize = 0;
unsigned char ucUnencryptedSendPacket[16384] = { 0 };
DWORD g_dwCurrentSendingSocket = 0;

std::atomic<bool> bLockEncryption = false;
DWORD* g_pdwSendNormal = 0;
DWORD* g_pdwCurrentEncryptionIndex = 0;
WORD* g_pwCurrentEncryptionIndex = 0;
WORD g_wCurrentEncryptionIndex = 0;
typedef void(__stdcall* _encryptPacketFunc)(char* buffer, int numBytes);
_encryptPacketFunc encryptPacketFunc;
void sendEncryptPacketFunc(SOCKET socSendSocket, char* buffer, int numBytes) {
    WORD* pwPacketEncryptionIndex = g_pwCurrentEncryptionIndex;

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