#pragma once
#include "WindowStuff.h"

union IDtoBYTE
{
	short id;
	BYTE bytes[2];
};

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