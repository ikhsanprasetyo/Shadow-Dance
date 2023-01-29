#pragma once
#include "includes.h"


int getVBE();
void InitHack();
void RemoveVmtHooks();
bool isEntityPopulated();
void InitConvars();
void ResetConvars();

//void SetSvCheats(int val);
void SetWeather(int val);
void SetDrawRange(int val);
void SetParticleHack(int val);
void SetNoFog(int val);
void SetCamDistance(int val);
template <class TipeData> 
void Print(const char* label, TipeData nilai);
void PrintHero1();
int GetHeroValue();