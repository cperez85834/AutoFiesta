#pragma once
#include "SkillStuff.h"

char carrAuthPacket[] = { 0x22, 0x2e, 0x0c, 0x30, 0x62, 0x31, 0x37, 0x32, 0x36, 0x61, 0x38, 0x30, 0x35, 0x66, 0x32, 0x34,
0x65, 0x63, 0x39, 0x37, 0x34, 0x32, 0x36, 0x64, 0x36, 0x64, 0x63, 0x38, 0x62, 0x30, 0x63, 0x36,
0x30, 0x37, 0x33 };
bool g_bDistributed = false;
// Left ring stuff
volatile bool g_bLeftRingEquipper = false;
volatile BYTE g_bySlotToEquip = 0xFF;

std::vector<WORD> vecNoBuffList{598, 600, 24, 501, 767, 28, 599, 48, 601, 49, 20, 23};
std::vector<WORD> vecBuffsGotten;
std::vector<WORD> vecBuffsGottenCopy;
std::mutex muxNoBuff;

LARGE_INTEGER timerTcounter;
class Timer
{
public:
	Timer()
	{
		QueryPerformanceFrequency(&timerTcounter);
		m_llTimerFreq = timerTcounter.QuadPart;

		QueryPerformanceCounter(&timerTcounter);
		m_llTickStart = timerTcounter.QuadPart;
		m_llTickNow = timerTcounter.QuadPart;
	}

	bool HasDurationPassed(float fDuration)
	{
		QueryPerformanceFrequency(&timerTcounter);
		m_llTimerFreq = timerTcounter.QuadPart;

		QueryPerformanceCounter(&timerTcounter);
		m_llTickNow = timerTcounter.QuadPart;

		if ((m_llTickNow - m_llTickStart) / (m_llTimerFreq / 1000) >= fDuration * 1000)
		{
			return true;
		}

		return false;
	}

	void Reset()
	{
		QueryPerformanceFrequency(&timerTcounter);
		m_llTimerFreq = timerTcounter.QuadPart;

		QueryPerformanceCounter(&timerTcounter);
		m_llTickStart = timerTcounter.QuadPart;
		m_llTickNow = timerTcounter.QuadPart;
	}

	double TimeElapsedms()
	{
		QueryPerformanceFrequency(&timerTcounter);
		m_llTimerFreq = timerTcounter.QuadPart;

		QueryPerformanceCounter(&timerTcounter);
		m_llTickNow = timerTcounter.QuadPart;

		return ((m_llTickNow - m_llTickStart) / (m_llTimerFreq / 1000));
	}

	double TimeElapsedSec()
	{
		QueryPerformanceFrequency(&timerTcounter);
		m_llTimerFreq = timerTcounter.QuadPart;

		QueryPerformanceCounter(&timerTcounter);
		m_llTickNow = timerTcounter.QuadPart;

		return ((m_llTickNow - m_llTickStart) / (m_llTimerFreq / 1000000));
	}
private:
	//LARGE_INTEGER m_liTimerCount{};
	LONGLONG m_llTickNow{};
	LONGLONG m_llTickStart{};
	LONGLONG m_llTimerFreq{};
};

//Checks to see if program memory is readable before accessing it, otherwise game crashes with ACCESS_VIOLATION
bool isMemReadable(LPCVOID memory, int bytes, void* pvStorage = nullptr) {

	DWORD dwMemBuf = 0;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	int return_status = 0;
	return_status = ReadProcessMemory(hProcess, memory, &dwMemBuf, bytes, NULL);

	CloseHandle(hProcess);

	if (return_status > 0)
	{
		if (pvStorage != nullptr)
		{
			if (bytes == 1)
			{
				*(BYTE*)(pvStorage) = (BYTE)dwMemBuf;
			}
			if (bytes == 2)
			{
				*(WORD*)(pvStorage) = (WORD)dwMemBuf;
			}
			if (bytes == 4)
			{
				*(DWORD*)(pvStorage) = (DWORD)dwMemBuf;
			}
		}
		return true;
	}
	else {
		//cout << "RPM failed at " << hex << memory << " with " << GetLastError() << endl;
		return false;
	}
}

bool CheckForKey(BYTE byKey)
{
	DWORD dwCurrentProcessID = GetCurrentProcessId();
	DWORD dwActiveWindowProcessID = 0;
	GetWindowThreadProcessId(GetForegroundWindow(), &dwActiveWindowProcessID);

	if (dwActiveWindowProcessID == dwCurrentProcessID)
	{
		if (GetAsyncKeyState(byKey) & 0x8000)
		{
			return true;
		}
	}

	return false;
}