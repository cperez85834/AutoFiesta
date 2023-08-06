#pragma once
#include "WindowStuff.h"


LONGLONG globalCooldown = 0;
LONGLONG buffsCooldown = 0;
LONGLONG hpstoneCooldown = 0;
LONGLONG spstoneCooldown = 0;
LONGLONG potionCooldown = 0;

LARGE_INTEGER g_liSkillTimerCounter;
LONGLONG g_llSkillCastTick{};
LONGLONG g_llSkillFreq;
union IDtoBYTE
{
	short id;
	BYTE bytes[2];
};
struct skill {
	float cooldown = 0; //seconds
	volatile BYTE id[2];
	LONGLONG tickAtCast = 0;
	float fDelayAfterCasting = 0;
	int iAnimationTime = 0; // milliseconds
	void Set(WORD wSkillID)
	{
		id[0] = wSkillID & 0xFF;
		id[1] = wSkillID >> 8;
	}
	bool IsReady()
	{
		LARGE_INTEGER liSkillTimerCounter;
		LONGLONG llSkillCastTick{};
		LONGLONG llSkillFreq;

		QueryPerformanceFrequency(&liSkillTimerCounter);
		llSkillFreq = liSkillTimerCounter.QuadPart;

		QueryPerformanceCounter(&liSkillTimerCounter);
		llSkillCastTick = liSkillTimerCounter.QuadPart;

		if ((llSkillCastTick - globalCooldown) / (llSkillFreq / 1000) <= 300) return false;
		if (((llSkillCastTick - tickAtCast) / (llSkillFreq / 1000)) >= (cooldown * 1000) || tickAtCast == 0) return true;

		return false;
	}
};

skill Heal;
skill Rejuv;
void InitializeHeals(WORD wSkillHealID, WORD wSkillRejuvID)
{
	IDtoBYTE healy;
	healy.id = wSkillHealID;

	Heal.id[0] = healy.bytes[0];
	Heal.id[1] = healy.bytes[1];
	Heal.cooldown = 2.9;
	Heal.iAnimationTime = 1000;

	IDtoBYTE rejuvy;
	rejuvy.id = wSkillRejuvID;

	Rejuv.id[0] = rejuvy.bytes[0];
	Rejuv.id[1] = rejuvy.bytes[1];
	Rejuv.cooldown = 7.2;
	Rejuv.iAnimationTime = 1000;
}