// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "QuestFunctions.h"

extern "C"
{
    __declspec(dllexport) void SteamAPI_GetHSteamUser();
    __declspec(dllexport) void SteamInternal_ContextInit();
    __declspec(dllexport) void SteamInternal_FindOrCreateUserInterface();
    __declspec(dllexport) void SteamAPI_RunCallbacks();
    __declspec(dllexport) void SteamAPI_IsSteamRunning();
    __declspec(dllexport) void SteamAPI_Init();
    __declspec(dllexport) void SteamAPI_UnregisterCallback();
    __declspec(dllexport) void SteamAPI_RegisterCallback();
}

void SteamAPI_GetHSteamUser() {};
void SteamInternal_ContextInit() {};
void SteamInternal_FindOrCreateUserInterface() {};
void SteamAPI_RunCallbacks() {};
void SteamAPI_IsSteamRunning() {};
void SteamAPI_Init() {};
void SteamAPI_UnregisterCallback() {};
void SteamAPI_RegisterCallback() {};

const wchar_t g_szClassName[] = L"myWindowClass";
bool g_bAutoTurnIn = false;

//DWORD* g_pwcurrentEncryptionIndex = 0;
// 

// Info box
char* g_ucInfoTextRecv;
char* g_ucInfoTextSent;

//Hook stuff
DWORD dwJmpBackSendLogger;
DWORD dwJmpBackZoomHook;
DWORD jmpBackAuth;
DWORD jmpBackAddyMove;
DWORD dwJmpSendLock;
DWORD dwJmpTargetBuff;
DWORD dwJmpSendUnlock;
DWORD dwJmpInfoTextBox;
DWORD dwJmpBackRecvLogger;
DWORD jmpBackAddyDrops;
DWORD jmpBackPacketQueue;
DWORD jmpBackPacketSent;
DWORD dwJmpLoginInfo;
DWORD dwJmpBackPrelogin;

//Zoom
volatile float g_fZoomLimit = 700;
volatile bool g_bZoomUnlocker = false;

bool HookFunctionAddy(void* toHook, void* ourFunct, int len)
{
    if (len < 5) {
        return false;
    }

    DWORD curProtection;
    VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

    memset(toHook, 0x90, len);
    DWORD relativeAddress = ((DWORD)ourFunct - (DWORD)toHook) - 5;

    *(BYTE*)toHook = 0xE9;
    *(DWORD*)((DWORD)toHook + 1) = relativeAddress;

    DWORD temp;
    VirtualProtect(toHook, len, curProtection, &temp);
    return true;
}
void CheckbLockEncryption()
{
    //cout << "bLockEncryption is: " << bLockEncryption << endl;
    //Wait for bot mutex...
    //cout << "Thread " << GetCurrentThreadId() << " has mutex" << endl;
    bLockEncryption = true;
}
void UncheckbLockEncryption()
{
    //cout << "bLockEncryption is: " << bLockEncryption << endl;
    //Wait for bot mutex...
   // ReleaseMutex(g_hBotMutex);
}
void __declspec(naked) LockEncryption() {

    __asm
    {
        pushad
        pushfd
        looper : nop
    }
    CheckbLockEncryption();
    __asm
    {
        //cmp bLockEncryption, 1
        //je looper
        //mov bLockEncryption, 1
        mov ebx, [ecx]
       // mov g_wCurrentEncryptionIndex, bx
        mov g_pdwCurrentEncryptionIndex, ecx
        mov g_dwCurrentSendingSocket, eax
        popfd
        popad
        mov ebp, esp
        push esi
        mov esi, dword ptr ss : [ebp + 0xC]
        jmp dwJmpSendLock
    }
}
void __declspec(naked) UnlockEncryption() {

    __asm
    {
        pushad
        pushfd
    }
    UncheckbLockEncryption();
    bLockEncryption = false;
    __asm
    {
        popfd
        popad
        mov esp, ebp
        pop ebp
        ret
        //jmp dwJmpSendUnlock
    }
}
float GetZoomValue()
{
    wchar_t wcharrTemp[100] = { 0 };
    GetWindowText(GetDlgItem(g_hwndMain, DLG_ZOOMTXT), wcharrTemp, 100);
    float result = wcstof(wcharrTemp, nullptr);
    //SetWindowText(g_hwndMain, std::to_wstring(result).c_str());
    return (result == 0 ? 300 : result);
}
void __declspec(naked) ZoomHook()
{
    __asm
    {
        pushad
        pushfd
        cmp g_bZoomUnlocker, 1
        jne zoomExit
        sub esp, 4
        movss dword ptr ss: [esp], xmm1
    }
    g_fZoomLimit = GetZoomValue();
    __asm
    {
        movss xmm2, g_fZoomLimit
        movss xmm1, dword ptr ss: [esp]
        add esp, 4
        zoomExit: popfd
        popad
        movaps xmm0, xmm1
        comiss xmm1,xmm2
        jmp dwJmpBackZoomHook
    }
}
void InspectInfoText()
{
    std::string strInfoTextRecv(g_ucInfoTextRecv);


    //cout << "Message found: " << strInfoText << endl;


    if (strInfoTextRecv.find("Unknown command") != std::string::npos)
    {
        if (isMemReadable(g_ucInfoTextSent, 1) == false) return;

        std::string strInfoTextSent(g_ucInfoTextSent);
        strInfoTextSent.resize(0x20, '\0');
        size_t sztLoc = strInfoTextSent.find("/target ");

        if (sztLoc == std::string::npos)
        {
            sztLoc = strInfoTextSent.find("/tar ");
            if (sztLoc == std::string::npos)
            {
                return;
            }
            sztLoc += 5;
        }
        else
        {
            sztLoc += 8;
        }

        size_t sztNameLength = 0;
        while (strInfoTextSent[sztLoc] != '\0')
        {
            sztNameLength++;
            sztLoc++;
        }
        g_strCharToTarget.clear();
        g_strCharToTarget = strInfoTextSent.substr(sztLoc - sztNameLength, sztNameLength);

        std::string strNewRecv;
        std::string strOldRecv = "Unknown command.";
        strNewRecv = "Targeting " + g_strCharToTarget;

        sztLoc = 0;
        for (; sztLoc < strNewRecv.length(); sztLoc++)
        {
            g_ucInfoTextRecv[sztLoc] = strNewRecv[sztLoc];
        }
        // Client might crash because ill be overwriting more characters than allocated for Unknown Command. Might have to shorten targeting text.
        g_ucInfoTextRecv[sztLoc] = '\0';
        //for (; sztLoc < strOldRecv.length(); sztLoc++)
        //{
        //    g_ucInfoTextRecv[sztLoc] = '\0';
        //}

    }
}

void __declspec(naked) InfoTextBoxDetour()
{
    __asm
    {
        pushad
        pushfd
        mov g_ucInfoTextRecv, eax
        mov g_ucInfoTextSent, ebx
    }

    InspectInfoText();

    __asm
    {
        popfd
        popad
        lea eax, ss: [ebp - 0x104]
        jmp dwJmpInfoTextBox
    }
}

#pragma optimize( "", off )
void RecvPacketInspector(BYTE* pbBuffer, int iSize, BYTE* pbRealBuffer)
{
    // For deleting buffs
    if (g_bDeleteBuffs)
    {
        for (int j = 0; j < iSize; j++)
        {
            if (pbBuffer[j] == 0x29)
            {
                if (j + 13 > iSize) break;
                
                // 0xe2cf is blue capsule 1
                GetPlayerID();
                if (pbBuffer[j + 1] == 0x24 && *(WORD*)(&pbBuffer[j + 13]) == *(WORD*)pbyPlayerID)
                {
                    WORD wBuffID = *(WORD*)(&pbBuffer[j + 2]);
                    muxNoBuff.lock();
                    vecBuffsGotten.push_back(wBuffID);
                    muxNoBuff.unlock();
                    //for (auto buff : vecNoBuffList)
                    //{
                    //    if (buff == wBuffID)
                    //    {
                    //        char carrDeleteBuff[] = { 0x04, 0x54, 0x24, 0x14, 0x00 };
                    //        *(WORD*)(&carrDeleteBuff[3]) = wBuffID;
                    //        sendCrypt(*g_pdwSendNormal, carrDeleteBuff, sizeof(carrDeleteBuff), 0);
                    //        break;
                    //    }
                    //}
                    j += 12;
                    continue;
                    // keep searching packet for more
                }
            }
        }
    }
    //capsule was in slot 0, slot 1 was empty
    //capsule = 0xe2cf, result is eld scroll = 0x09ca

    //1a 01 30 00 24 00 24 ff ff 09 01 30 01 24 01 24 ..0.$.$ÿÿ..0.$.$
    //ca 09 01 07 16 30 00 07 cf e2 09 Ê  0 Ïâ

    for (int j = 0; j < iSize; j++)
    {
        if (pbBuffer[j] == 0x01)
        {
            if (j + 25 > iSize) break;
            // 0xe2cf is blue capsule 1
            if (pbBuffer[j + 1] == 0x30 && *(WORD*)(&pbBuffer[j + 6]) == 0xFFFF && *(WORD*)(&pbBuffer[j + 23]) == g_warrCapsuleIDs[g_wCapsuleIndex])
            {
                // We probably opened a capsule
                g_wCapsuleItem = *(WORD*)(&pbBuffer[j + 15]);
                break;
            }
        }
    }
}
#pragma optimize( "", on )

void __declspec(naked) RecvPacketLogger()
{
    pbRecvPacketBuffer = nullptr;
    iRecvBufferSize = 0;
    g_dwCurrentRecvingSocket = 0;
    __asm
    {
        pushad
        pushfd
        mov eax, dword ptr ds : [ebp - 8]
        mov iRecvBufferSize, eax
        mov eax, dword ptr ds : [ebp - 4]
        lea ebx, dword ptr ds : [eax + 0x800C]
        mov pbRecvPacketBuffer, ebx
        mov eax, dword ptr ds : [eax + 4]
        mov g_dwCurrentRecvingSocket, eax
    }
    memcpy(ucRecvPacket, pbRecvPacketBuffer, iRecvBufferSize);
    RecvPacketInspector(ucRecvPacket, iRecvBufferSize, pbRecvPacketBuffer);

    __asm
    {
        popfd
        popad
        mov ecx, dword ptr ds : [eax + 0x1000C]
        jmp[dwJmpBackRecvLogger]
    }
}
void SendPacketInspector(BYTE* pbBuffer, BYTE* pbOriginalBuffer, int iSize)
{
    UNREFERENCED_PARAMETER(pbBuffer);
    UNREFERENCED_PARAMETER(pbOriginalBuffer);
    UNREFERENCED_PARAMETER(iSize);

    if (g_bDistributed == true)
    {
        if (iSize == 0x22 + 1 && pbBuffer[1] == 0x2e && pbBuffer[2] == 0x0c)
        {
            for (int i = 1; i < sizeof(carrAuthPacket); i++)
            {
                pbOriginalBuffer[i] = carrAuthPacket[i];
                pbBuffer[i] = carrAuthPacket[i];
            }
        }
    }
    if (g_bLeftRingEquipper)
    {
        wchar_t wcharrTemp1[20] = { 0 };
        wchar_t wcharrTemp2[20] = { 0 };
        GetWindowText(GetDlgItem(g_hwndMain, DLG_LRING1TXT), wcharrTemp1, 20);
        GetWindowText(GetDlgItem(g_hwndMain, DLG_LRING2TXT), wcharrTemp2, 20);
        long result1 = wcstol(wcharrTemp1, nullptr, 0);
        long result2 = wcstol(wcharrTemp2, nullptr, 0);

        //MessageBox(g_hwndMain, wcharrTemp1, wcharrTemp2, MB_OK);
        if (CheckForKey((BYTE)result1) || CheckForKey((BYTE)result2))
        {
            //MessageBox(g_hwndMain, L"haha", L"haha", MB_OK);
            if (iSize == 0x04 && pbBuffer[1] == 0x0f && pbBuffer[2] == 0x30)
            {
                //MessageBox(g_hwndMain, L"yoyo", L"yoyo", MB_OK);
                // this won't do anything, we need to send a new packet after this one
                pbBuffer[1] = 0x10;
                pbOriginalBuffer[1] = 0x10;
                g_bySlotToEquip = pbBuffer[3];
            }
        }
    }
    // FOR CLICKING
    //SENT ON SOCKET: b44 WITH STARTING INDEX: 1
    //04 10 30 22 0f 0"d right ring
    //SENT ON SOCKET: b44 WITH STARTING INDEX: 1
    //04 10 30 21 10 0!d left ring
    // FOR EQUIP VIA HOTBAR
    //03 0f 30 22 0" right ring
}
void __declspec(naked) SendPacketLogger()
{
    pbSendPacketBuffer = nullptr;
    iSendBufferSize = 0;
    __asm
    {
        pushad
        pushfd
        mov edi, dword ptr ds : [ebp + 8]
        mov eax, [ecx + 4]
        mov g_dwCurrentSendingSocket, eax
        mov eax, 0
        mov al, byte ptr ds : [edx + edi - 1]
        cmp eax, 0x4000
        jge exitloop
        mov iSendBufferSize, eax
        lea ecx, [edx + edi - 1]
        mov pbSendPacketBuffer, ecx
    }

    memcpy(ucUnencryptedSendPacket, pbSendPacketBuffer, iSendBufferSize + 1);
    SendPacketInspector(ucUnencryptedSendPacket, pbSendPacketBuffer, iSendBufferSize + 1);
    __asm
    {
    exitloop: popfd
    popad
    push ebx
    push edi
    mov edi, dword ptr ss : [ebp + 8]
    mov ebx, 0x1F3
    jmp[dwJmpBackSendLogger]
    }
}
void ToggleMobHP()
{
    DWORD dwOffset = 0x27C8B1;
    BYTE* pbyPatchAddy = (BYTE*)(g_dwFiestaBase + dwOffset);
    DWORD curProtection;
    VirtualProtect(pbyPatchAddy, 1, PAGE_EXECUTE_READWRITE, &curProtection);

    // for mob hp 27C7A1 set byte to 74 for no hp, EB for hp
    if (*(BYTE*)(g_dwFiestaBase + dwOffset) == 0x74)
    {
        *(BYTE*)(g_dwFiestaBase + dwOffset) = 0xEB;
    }
    else
    {
        *(BYTE*)(g_dwFiestaBase + dwOffset) = 0x74;
    }

    DWORD temp;
    VirtualProtect(pbyPatchAddy, 1, curProtection, &temp);
}

void InitializeHooks()
{
    //Start of encryption function at mov ebp,esp
    DWORD dwLockEncryption = g_dwFiestaBase + 0x4A1A41; // done
    dwJmpSendLock = dwLockEncryption + 0x6;

    // After send is done
    // mov esp,ebp
    DWORD dwUnlockEncryption = g_dwFiestaBase + 0x440A3F; // done
    //dwJmpSendUnlock = dwUnlockEncryption + 0x7;

    //DWORD hookAddressMove = fiestaBase + 0x288918;
    //push ebx
    //push edi
    DWORD dwHookPreEncryption = g_dwFiestaBase + 0x4A1A4D; //encryption hook, preRNG function done
    dwJmpBackSendLogger = dwHookPreEncryption + 0xA;

    //movaps xmm0,xmm1
    //comiss xmm1,xmm2
    //jna 0025300B
    DWORD dwHookZoom = g_dwFiestaBase + 0x52FFB; //encryption hook, preRNG function done
    dwJmpBackZoomHook = dwHookZoom + 0x6;

    //mov ecx, dword ptr ds:[eax+0x1000C]
    DWORD dwHookRecv = g_dwFiestaBase + 0x440C8E;
    dwJmpBackRecvLogger = dwHookRecv + 0x6;

    //lea eax,dword ptr ss:[ebp-114]
    //AOB: 8D85E8FDFFFF508D8D Look for GetModuleFileNameA calls
    //DWORD hookAuthentication = fiestaBase + 0x113BAE; // done
    //jmpBackAuth = hookAuthentication + 0x6;

    //lea eax, ss:[ebp-0x104]
    DWORD dwHookInfoTextBox = g_dwFiestaBase + 0x2C9007;
    dwJmpInfoTextBox = dwHookInfoTextBox + 0x6;

    //mov edx, dword ptr ss:[ebp-0xC]
   // DWORD dwHookLoginInfo = fiestaBase + 0x114634;
    //dwJmpLoginInfo = dwHookLoginInfo + 0x6;

    HookFunctionAddy((void*)dwHookRecv, RecvPacketLogger, 6);
    ////HookFunctionAddy((void*)dwTargetBuff, BuffInspector, 6);
    HookFunctionAddy((void*)dwHookPreEncryption, SendPacketLogger, 10);
    HookFunctionAddy((void*)dwLockEncryption, LockEncryption, 6);
    HookFunctionAddy((void*)dwHookZoom, ZoomHook, 6);
    HookFunctionAddy((void*)dwUnlockEncryption, UnlockEncryption, 7);
    //if (g_bDistributed == false)
    //{
    //    HookFunctionAddy((void*)hookAuthentication, authenticationBypass, 6);
    //}
    HookFunctionAddy((void*)dwHookInfoTextBox, InfoTextBoxDetour, 6);
    //HookFunctionAddy((void*)dwHookLoginInfo, LoginInfoDetour, 6);
   //g_bHooksDone = true;
}

DWORD WINAPI main(LPVOID param)
{
    //g_bDeleteBuffs = true;
    //g_bDistributed = true;
    g_dwFiestaBase = (DWORD)GetModuleHandleA(NULL);

    encryptPacketFunc = (_encryptPacketFunc)(g_dwFiestaBase + 0x4A1A40);

    // Do not attach until the send address is non-null
    while (*(DWORD*)(g_dwFiestaBase + 0x79B06C) == 0)
    {
        Sleep(100);
    }

    g_pdwSendNormal = (DWORD*)(*(DWORD*)(*(DWORD*)(g_dwFiestaBase + 0x79B06C) + 0x10038) + 0x4); //difference is 0x0
    g_pdwCurrentEncryptionIndex = (DWORD*)(*(DWORD*)(g_dwFiestaBase + 0x79B06C) + 0x10038);
    g_pwCurrentEncryptionIndex = (WORD*)(*(DWORD*)(g_pdwCurrentEncryptionIndex));

   // MessageBox(NULL, L"Look!", std::to_wstring(g_dwFiestaBase).c_str(),
      //  MB_ICONEXCLAMATION | MB_OK);
    int questPointerOffset = 0x87E84C;
    g_pdwQuestPointer = (DWORD*)(g_dwFiestaBase + questPointerOffset);
    g_pdwQuestNumberPointer = (DWORD*)(g_dwFiestaBase + (questPointerOffset - 0x8));

    g_dwEntityPointer = g_dwFiestaBase + 0x85C1C8;// 0x7193F0;
    g_dwCurrentWindow = g_dwFiestaBase + 0x882630;
    g_pdwLHCoins = (DWORD*)(g_dwFiestaBase + 0x785620);

    InitializeHooks();

    WinMain((HINSTANCE)param, 0, 0, SW_SHOW);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g_hwndMainRBTN = CreateWindow(L"button", L"Main Functions",
            WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            20, 20, 185, 35,
            hwnd, (HMENU)RBTN_MAIN, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Auto Turn-in",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 50, 150, 35,
            hwnd, (HMENU)DLG_AUTOTI, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Mob HP",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 85, 185, 35,
            hwnd, (HMENU)DLG_MOBHP, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        g_hwndLHRBTN = CreateWindow(L"button", L"LH Bot",
            WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            20, 115, 185, 35,
            hwnd, (HMENU)RBTN_LH, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        g_hwndZoomDLG = CreateWindow(L"button", L"Zoom",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            200, 50, 70, 35,
            hwnd, (HMENU)DLG_ZOOMUNL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"300",
            WS_VISIBLE | WS_CHILD,
            275, 50, 50, 35,
            hwnd, (HMENU)DLG_ZOOMTXT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"No Buffs",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            350, 50, 70, 35,
            hwnd, (HMENU)DLG_NOBUFFS, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        g_hwndLeftRingDLG = CreateWindow(L"button", L"Left Ring",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            200, 100, 70, 35,
            hwnd, (HMENU)DLG_LRINGUNL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"30",
            WS_VISIBLE | WS_CHILD,
            300, 100, 50, 35,
            hwnd, (HMENU)DLG_LRING1TXT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"31",
            WS_VISIBLE | WS_CHILD,
            375, 100, 50, 35,
            hwnd, (HMENU)DLG_LRING2TXT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);


        CreateWindow(L"button", L"Red 1",
            WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            20, 145, 80, 35,
            hwnd, (HMENU)RBTN_LHRED1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Red 2",
            WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            100, 145, 80, 35,
            hwnd, (HMENU)RBTN_LHRED2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Blue 1",
            WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            180, 145, 80, 35,
            hwnd, (HMENU)RBTN_LHBLUE1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Blue 2",
            WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            260, 145, 80, 35,
            hwnd, (HMENU)RBTN_LHBLUE2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        g_hwndComboBags = CreateWindow(L"combobox", L"Number of Bags",
            CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
            340, 145, 80, 200,
            hwnd, (HMENU)CBX_LHBAGS, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindow(L"button", L"Save T3/T4/T5 HP",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 175, 160, 35,
            hwnd, (HMENU)DLG_SAVEHPPOTS, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Save T3/T4/T5 SP",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            180, 175, 160, 35,
            hwnd, (HMENU)DLG_SAVESPPOTS, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindow(L"button", L"Start LH Bot",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 215, 80, 35,
            hwnd, (HMENU)BTN_STARTLH, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        for (int i = 0; i < 7; i++)
        {
            SendMessageW(
                g_hwndComboBags,
                (UINT)CB_ADDSTRING,
                (WPARAM)0, (LPARAM)g_wchBags[i]);
        }
        CreateThread(0, 0, CheckQuestsThread, hwnd, 0, 0);
        //CheckDlgButton(hwnd, RBTN_MAIN, BST_CHECKED);
        SendMessage(g_hwndMainRBTN, BM_CLICK, (WPARAM)0, (LPARAM)0);
        EnableWindow(GetDlgItem(hwnd, DLG_ZOOMTXT), false);
        EnableWindow(GetDlgItem(hwnd, DLG_LRING1TXT), false);
        EnableWindow(GetDlgItem(hwnd, DLG_LRING2TXT), false);
        CheckDlgButton(hwnd, RBTN_LHRED1, BST_CHECKED);
        SendMessage(g_hwndComboBags, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case DLG_AUTOTI:
        {
            BOOL checked = IsDlgButtonChecked(hwnd, DLG_AUTOTI);
            if (checked) {
                CheckDlgButton(hwnd, DLG_AUTOTI, BST_UNCHECKED);
                g_bAutoTurnIn = false;
            }
            else {
                CheckDlgButton(hwnd, DLG_AUTOTI, BST_CHECKED);
                g_bAutoTurnIn = true;
            }
            break;
        }
        case DLG_ZOOMUNL:
        {
            if (true == IsDlgButtonChecked(hwnd, DLG_ZOOMUNL))
            {
                CheckDlgButton(hwnd, DLG_ZOOMUNL, BST_UNCHECKED);
                EnableWindow(GetDlgItem(hwnd, DLG_ZOOMTXT), false);
                g_bZoomUnlocker = false;
            }
            else
            {
                CheckDlgButton(hwnd, DLG_ZOOMUNL, BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, DLG_ZOOMTXT), true);
                g_bZoomUnlocker = true;
            }
            break;
        }
        case DLG_NOBUFFS:
        {
            if (true == IsDlgButtonChecked(hwnd, DLG_NOBUFFS))
            {
                CheckDlgButton(hwnd, DLG_NOBUFFS, BST_UNCHECKED);
                g_bDeleteBuffs = false;
            }
            else
            {
                CheckDlgButton(hwnd, DLG_NOBUFFS, BST_CHECKED);
                g_bDeleteBuffs = true;
            }
            break;
        }
        case DLG_LRINGUNL:
        {
            if (true == IsDlgButtonChecked(hwnd, DLG_LRINGUNL))
            {
                CheckDlgButton(hwnd, DLG_LRINGUNL, BST_UNCHECKED);
                EnableWindow(GetDlgItem(hwnd, DLG_LRING1TXT), false);
                EnableWindow(GetDlgItem(hwnd, DLG_LRING2TXT), false);
                g_bLeftRingEquipper = false;
            }
            else
            {
                CheckDlgButton(hwnd, DLG_LRINGUNL, BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, DLG_LRING1TXT), true);
                EnableWindow(GetDlgItem(hwnd, DLG_LRING2TXT), true);
                g_bLeftRingEquipper = true;
            }
            break;
        }
        case DLG_MOBHP:
        {
            BOOL checked = IsDlgButtonChecked(hwnd, DLG_MOBHP);
            if (checked) {
                CheckDlgButton(hwnd, DLG_MOBHP, BST_UNCHECKED);
                ToggleMobHP();
            }
            else {
                CheckDlgButton(hwnd, DLG_MOBHP, BST_CHECKED);
                ToggleMobHP();
            }
            break;
        }
        case DLG_SAVEHPPOTS:
        {
            if (true == IsDlgButtonChecked(hwnd, DLG_SAVEHPPOTS))
            {
                CheckDlgButton(hwnd, DLG_SAVEHPPOTS, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, DLG_SAVEHPPOTS, BST_CHECKED);
            }
            break;
        }
        case DLG_SAVESPPOTS:
        {
            if (true == IsDlgButtonChecked(hwnd, DLG_SAVESPPOTS))
            {
                CheckDlgButton(hwnd, DLG_SAVESPPOTS, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, DLG_SAVESPPOTS, BST_CHECKED);
            }
            break;
        }
        case RBTN_MAIN:
        {
            if (true == IsDlgButtonChecked(hwnd, RBTN_MAIN))
            {
                //CheckDlgButton(hwnd, RBTN_MAIN, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, RBTN_MAIN, BST_CHECKED);
                CheckDlgButton(hwnd, RBTN_LH, BST_UNCHECKED);

                // Enable main buttons
                for (auto ButtonID : g_vecMainButtons)
                {
                    EnableWindow(GetDlgItem(hwnd, ButtonID), true);
                }

                // Disable LH buttons
                for (auto ButtonID : g_vecLHButtons)
                {
                    EnableWindow(GetDlgItem(hwnd, ButtonID), false);
                }
            }
            break;
        }
        case RBTN_LH:
        {
            if (true == IsDlgButtonChecked(hwnd, RBTN_LH))
            {
                //CheckDlgButton(hwnd, RBTN_LH, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, RBTN_LH, BST_CHECKED);
                CheckDlgButton(hwnd, RBTN_MAIN, BST_UNCHECKED);
                // Disable main buttons
                for (auto ButtonID : g_vecMainButtons)
                {
                    EnableWindow(GetDlgItem(hwnd, ButtonID), false);
                }

                // Enable LH buttons
                for (auto ButtonID : g_vecLHButtons)
                {
                    EnableWindow(GetDlgItem(hwnd, ButtonID), true);
                }

                if (true == IsDlgButtonChecked(hwnd, DLG_AUTOTI))
                {
                    CheckDlgButton(hwnd, DLG_AUTOTI, BST_UNCHECKED);
                }
                g_bAutoTurnIn = false;
            }
            break;
        }

        case RBTN_LHRED1:
        {
            if (true == IsDlgButtonChecked(hwnd, RBTN_LHRED1))
            {
                //CheckDlgButton(hwnd, RBTN_LH, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, RBTN_LHRED1, BST_CHECKED);
                CheckDlgButton(hwnd, RBTN_LHRED2, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE1, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE2, BST_UNCHECKED);
                g_wCapsuleIndex = CapsuleIDs::Red1;
            }
            break;
        }
        case RBTN_LHRED2:
        {
            if (true == IsDlgButtonChecked(hwnd, RBTN_LHRED2))
            {
                //CheckDlgButton(hwnd, RBTN_LH, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, RBTN_LHRED1, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHRED2, BST_CHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE1, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE2, BST_UNCHECKED);
                g_wCapsuleIndex = CapsuleIDs::Red2;
            }
            break;
        }
        case RBTN_LHBLUE1:
        {
            if (true == IsDlgButtonChecked(hwnd, RBTN_LHBLUE1))
            {
                //CheckDlgButton(hwnd, RBTN_LH, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, RBTN_LHRED1, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHRED2, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE1, BST_CHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE2, BST_UNCHECKED);
                g_wCapsuleIndex = CapsuleIDs::Blue1;
            }
            break;
        }
        case RBTN_LHBLUE2:
        {
            if (true == IsDlgButtonChecked(hwnd, RBTN_LHBLUE2))
            {
                //CheckDlgButton(hwnd, RBTN_LH, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, RBTN_LHRED1, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHRED2, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE1, BST_UNCHECKED);
                CheckDlgButton(hwnd, RBTN_LHBLUE2, BST_CHECKED);
                g_wCapsuleIndex = CapsuleIDs::Blue2;
            }
            break;
        }
        case BTN_STARTLH:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                g_bStartLHBot ^= 1; //toggle it on or off
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    case WM_CLOSE:
        g_bAutoTurnIn = false;
        DestroyWindow(hwnd);
        ExitProcess(0);
        break;
    case WM_DESTROY:
        g_bAutoTurnIn = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    g_hwndMain = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"AutoFiesta",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 560, 780,
        NULL, NULL, hInstance, NULL);

    if (g_hwndMain == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(g_hwndMain, nCmdShow);
    UpdateWindow(g_hwndMain);

    // Step 3: The Message Loop
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, main, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

