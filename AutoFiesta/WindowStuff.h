#pragma once

#include <atomic>
#include <vector>
#include <WinSock2.h>
#include <windows.h>
#include <string>

#define DLG_AUTOTI 1000
#define DLG_MOBHP 1001
#define RBTN_MAIN 1002

#define RBTN_LH 1100
#define RBTN_LHRED1 1101
#define RBTN_LHRED2 1102
#define RBTN_LHBLUE1 1103
#define RBTN_LHBLUE2 1104
#define CBX_LHBAGS 1105
#define DLG_SAVEHPPOTS 1106
#define DLG_SAVESPPOTS 1107
#define BTN_STARTLH 1108

std::vector<WORD> g_vecLHButtons{1101, 1102, 1103, 1104, 1105, 1106, 1107};
std::vector<WORD> g_vecMainButtons{1000, 1001};

//int g_iLHButtonDisableList[] = { 1000,1001,1002,1100,1101,1102,1103,1104,1105,1106,1107 };
HWND g_hwndMain;
HWND g_hwndMainRBTN;
HWND g_hwndLHRBTN;
HWND g_hwndComboBags;

wchar_t g_wchBags[7][2] = {L"2", L"3", L"4", L"5" ,L"6", L"7", L"8"};