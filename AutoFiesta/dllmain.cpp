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
DWORD jmpBackAuth;
DWORD jmpBackAddyMove;
DWORD dwJmpSendLock;
DWORD dwJmpTargetBuff;
DWORD dwJmpSendUnlock;
DWORD dwJmpInfoTextBox;
DWORD jmpBackAddyDrops;
DWORD jmpBackPacketQueue;
DWORD jmpBackPacketSent;
DWORD dwJmpLoginInfo;
DWORD dwJmpBackPrelogin;

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

void InspectInfoText()
{
    std::string strInfoTextRecv(g_ucInfoTextRecv);
    std::string strInfoTextSent(g_ucInfoTextSent);

    //cout << "Message found: " << strInfoText << endl;
    strInfoTextSent.resize(0x20, '\0');
    size_t sztLoc = strInfoTextSent.find("/target ");
    if (sztLoc != std::string::npos)
    {
        sztLoc += 8;
        size_t sztNameLength = 0;
        while (strInfoTextSent[sztLoc] != '\0')
        {
            sztNameLength++;
            sztLoc++;
        }
        g_strCharToTarget.clear();
        g_strCharToTarget = strInfoTextSent.substr(sztLoc - sztNameLength, sztNameLength);

        std::string strNewRecv;
        std::string strOldRecv = "Unknown Command.";
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

void ToggleMobHP()
{
    BYTE* pbyPatchAddy = (BYTE*)(g_dwFiestaBase + 0x27C7A1);
    DWORD curProtection;
    VirtualProtect(pbyPatchAddy, 1, PAGE_EXECUTE_READWRITE, &curProtection);

    // for mob hp 27C7A1 set byte to 74 for no hp, EB for hp
    if (*(BYTE*)(g_dwFiestaBase + 0x27C7A1) == 0x74)
    {
        *(BYTE*)(g_dwFiestaBase + 0x27C7A1) = 0xEB;
    }
    else
    {
        *(BYTE*)(g_dwFiestaBase + 0x27C7A1) = 0x74;
    }

    DWORD temp;
    VirtualProtect(pbyPatchAddy, 1, curProtection, &temp);
}

void InitializeHooks()
{
    //Start of encryption function at mov ebp,esp
    DWORD dwLockEncryption = g_dwFiestaBase + 0x4A1961; // done
    dwJmpSendLock = dwLockEncryption + 0x6;

    // After send is done
    // mov esp,ebp
    DWORD dwUnlockEncryption = g_dwFiestaBase + 0x44094F; // done
    //dwJmpSendUnlock = dwUnlockEncryption + 0x7;

    //DWORD hookAddressMove = fiestaBase + 0x288918;
    //push ebx
    //push edi
    DWORD dwHookPreEncryption = g_dwFiestaBase + 0x4A196D; //encryption hook, preRNG function done
    dwJmpBackSendLogger = dwHookPreEncryption + 0xA;

    //lea eax,dword ptr ss:[ebp-114]
    //AOB: 8D85E8FDFFFF508D8D Look for GetModuleFileNameA calls
    //DWORD hookAuthentication = fiestaBase + 0x113BAE; // done
    //jmpBackAuth = hookAuthentication + 0x6;

    //lea eax, ss:[ebp-0x104]
    DWORD dwHookInfoTextBox = g_dwFiestaBase + 0x2C8EE7;
    dwJmpInfoTextBox = dwHookInfoTextBox + 0x6;

    //mov edx, dword ptr ss:[ebp-0xC]
   // DWORD dwHookLoginInfo = fiestaBase + 0x114634;
    //dwJmpLoginInfo = dwHookLoginInfo + 0x6;

    //HookFunctionAddy((void*)dwHookRecv, RecvPacketLogger, 6);
    ////HookFunctionAddy((void*)dwTargetBuff, BuffInspector, 6);
    //HookFunctionAddy((void*)dwHookPreEncryption, SendPacketLogger, 10);
    HookFunctionAddy((void*)dwLockEncryption, LockEncryption, 6);
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
    g_dwFiestaBase = (DWORD)GetModuleHandleA(NULL);

    encryptPacketFunc = (_encryptPacketFunc)(g_dwFiestaBase + 0x4A1960);

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

    InitializeHooks();

    WinMain((HINSTANCE)param, 0, 0, SW_SHOW);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CreateWindow(L"button", L"Auto Turn-in",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 20, 185, 35,
            hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindow(L"button", L"Mob HP",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 55, 185, 35,
            hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateThread(0, 0, CheckQuestsThread, hwnd, 0, 0);
        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case 1:
        {
            BOOL checked = IsDlgButtonChecked(hwnd, 1);
            if (checked) {
                CheckDlgButton(hwnd, 1, BST_UNCHECKED);
                g_bAutoTurnIn = false;
                SetWindowText(hwnd, L"Not Checked");
            }
            else {
                CheckDlgButton(hwnd, 1, BST_CHECKED);
                g_bAutoTurnIn = true;
                SetWindowText(hwnd, L"Checked");
            }
            break;
        }
        case 2:
        {
            BOOL checked = IsDlgButtonChecked(hwnd, 2);
            if (checked) {
                CheckDlgButton(hwnd, 2, BST_UNCHECKED);
                ToggleMobHP();
            }
            else {
                CheckDlgButton(hwnd, 2, BST_CHECKED);
                ToggleMobHP();
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
    HWND hwnd;
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"AutoFiesta",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 160,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

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

