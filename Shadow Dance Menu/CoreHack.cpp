#pragma once
#include "CoreHack.h"
#include "common.h"
#include "vmt.h"
#include "Schema.h"
#include "sdk/CGameEntitySystem.h"
#include "sdk/ICVar.h"
#include <set>
#include <deque>

class IEngineClient;

VMT* entityVMT;
CGameEntitySystem* entity;
IEngineClient* engine;
std::vector<CEntityInstance*> Heroes;
ICVar* VEngine;
ICVar::CvarNode* sv_cheats;
ICVar::CvarNode* camera_distance;
ICVar::CvarNode* drawrange;
ICVar::CvarNode* r_farz;
ICVar::CvarNode* fog_enable;
ICVar::CvarNode* weather;
ICVar::CvarNode* particle_hack;
SchemaNetvarCollection* Netvars = 0;;

int localHero = -1;
int localPlayerIndex = -1;

int isAlive = 0, iHealth = 0, iMaxHealth = 0, iTeamNum = 0;
bool bDormant = false;
float isVisibleByEnemy = 0.0f;
int iAttackRange = 0;

int& GetLocalPlayer(int& = localPlayerIndex, int screen = 0) {
    typedef int& (*Fn)(void*, int&, int);
    return getvfunc<Fn>(engine, 20)(engine, localPlayerIndex, screen);
}


CEntityInstance* OnAddEntity(CGameEntitySystem* ecx, CEntityInstance* ptr, EntityHandle index)
{
    auto ret = entityVMT->GetOriginalMethod(OnAddEntity)(ecx, ptr, index);
    const char* typeName = ptr->Schema_DynamicBinding()->bindingName;
    
    if (strstr(typeName, "DOTA_Unit_Hero")) {

        bool alreadyExists = false;
        for (auto hero : Heroes)
        {
            if (typeName == hero->Schema_DynamicBinding()->bindingName)
            {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists)
        {
                Heroes.emplace_back(ptr);
        }
    }

    return ret;
}

CEntityInstance* OnRemoveEntity(CGameEntitySystem* ecx, CEntityInstance* ptr, EntityHandle index)
{
    const char* typeName = ptr->Schema_DynamicBinding()->bindingName;

    if (strstr(typeName, "DOTA_Unit_Hero")) {
        for (size_t i = Heroes.size(); i-- > 0; ) {
            if (Heroes[i] == ptr) {
                Heroes.erase(Heroes.begin() + i);
                if (Heroes.size() == 0)
                {
                    localHero = -1;
                    std::cout << "not populated" << std::endl;
                }
                break;
            }
        }
    }

    return entityVMT->GetOriginalMethod(OnRemoveEntity)(ecx, ptr, index);
}

void InitConvars() {
    VEngine = (ICVar*)GetInterface("tier0.dll", "VEngineCvar007");
    ICVar::CvarNode* cvarNode = (*(ICVar::CvarNode**)(VEngine + 0x40));

    for (size_t i = 0; i < 4226; i++)
    {
        if (strcmp(cvarNode->var->name, "sv_cheats") == 0)
        {
            sv_cheats = cvarNode;
        }
        else if (strcmp(cvarNode->var->name, "dota_camera_distance") == 0)
        {
            camera_distance = cvarNode;
        }
        else if (strcmp(cvarNode->var->name, "dota_range_display") == 0)
        {
            drawrange = cvarNode;
        }
        else if (strcmp(cvarNode->var->name, "r_farz") == 0)
        {
            r_farz = cvarNode;
        }
        else if (strcmp(cvarNode->var->name, "fog_enable") == 0)
        {
            fog_enable = cvarNode;
        }
        else if (strcmp(cvarNode->var->name, "cl_weather") == 0)
        {
            weather = cvarNode;
        }
        else if (strcmp(cvarNode->var->name, "dota_use_particle_fow") == 0)
        {
            particle_hack = cvarNode;
        }
        cvarNode = ((ICVar::CvarNode*)((uint64_t)cvarNode + 0x10));
    }

}

void InitEntity() {
    void* client = GetInterface("client.dll", "Source2Client002");
    uintptr_t* vmt_slot = *(uintptr_t**)client + 25; //25th function in Source2Client vtable
    uintptr_t addr_start = *vmt_slot + 3; //stores the relative address portion of the mov rax, [rip + 0x2512059] instruction
    entity = *(CGameEntitySystem**)(addr_start + *(uint32_t*)(addr_start)+4); //pointer to CEntitySystem is at 2512059 + addr_start + 4
    // Init CEngine
    engine = (IEngineClient*)GetInterface("engine2.dll", "Source2EngineToClient001");
    // Hook our entity
    entityVMT = new VMT(entity); //loads CEntitySystem VMT into vmt.entity
    entityVMT->HookVMT(OnAddEntity, 14);
    entityVMT->HookVMT(OnRemoveEntity, 15);
    entityVMT->ApplyVMT(entity);
}

void InitSchema() {
    CMsg = (ConMsg)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg");
    SchemaSystem = (uint64_t)GetInterface("schemasystem.dll", "SchemaSystem_001");
    //Panorama = (uint64_t)GetInterface("panorama.dll", "PanoramaUIEngine001"); //29-Jan-23
    Netvars = new SchemaNetvarCollection;
    Netvars->Add("C_DOTA_BaseNPC", "client.dll");
    Netvars->Add("C_DOTA_BaseNPC_Hero", "client.dll");
    Netvars->Add("C_DOTABaseAbility", "client.dll");
    Netvars->Add("C_DOTAGamerules", "client.dll");
    Netvars->Add("C_BaseEntity", "client.dll");
    Netvars->Add("C_BaseModelEntity", "client.dll");
    Netvars->Add("C_BaseCombatCharacter", "client.dll");

    m_iTeamNum = Netvars->Get((uint64_t)"m_iTeamNum")->offset;
    m_hOwnerEntity = Netvars->Get((uint64_t)"m_hOwnerEntity")->offset;
    m_flStartSequenceCycle = Netvars->Get((uint64_t)"m_flStartSequenceCycle")->offset;
    m_fGameTime = Netvars->Get((uint64_t)"m_fGameTime")->offset;
    m_nGameState = Netvars->Get((uint64_t)"m_nGameState")->offset;
    m_iGameMode = Netvars->Get((uint64_t)"m_iGameMode")->offset;
    m_hReplicatingOtherHeroModel = Netvars->Get((uint64_t)"m_hReplicatingOtherHeroModel")->offset;
    m_lifeState = Netvars->Get((uint64_t)"m_lifeState")->offset;

    m_iHealth = Netvars->Get((uint64_t)"m_iHealth")->offset;
    m_iMaxHealth = Netvars->Get((uint64_t)"m_iMaxHealth")->offset;

    //m_bDormant = Netvars->Get((uint64_t)"m_bDormant")->offset; //30-Jan-23 test
    //m_clrRender //ganti warna hero
    //m_hOwnerEntity = Netvars->Get((uint64_t)"m_hAssignedHero")->offset; //29-Jan-23 test
   
    //m_iHealthBarOffset
    //m_iAttackRange = Netvars->Get((uint64_t)"m_iAttackRange")->offset; //not working
    
}

void InitHack() {
    InitConvars();
    InitEntity();
    InitSchema();
}

void RemoveVmtHooks()
{
    entityVMT->RevertVMT(entity); // Unhook entity
}


int threshold = 6;
std::deque<float> duplicates;

int unique_num() {
    std::set<float> nums;
    for (auto var : duplicates)
    {
        nums.insert(var);
    }
    return (int)nums.size(); //29-Jan-23
}


int GetHeroValue()
{

    if (localHero > -1)
    {
        iHealth = Heroes[localHero]->Health();
        iMaxHealth = Heroes[localHero]->MaxHealth();
        isAlive = Heroes[localHero]->IsAlive();
        iTeamNum = Heroes[localHero]->TeamNum();
        isVisibleByEnemy = Heroes[localHero]->IsVisibleByEnemy();
        //bDormant = (bool)Heroes[localHero]->IsDormant();
    }

    return 1;
}



int getVBE() {
    if (Heroes.size() == 0) {
        localHero = -1;
        return -1; //cek
    }

    if (localHero == -1)
    {
        GetLocalPlayer(localPlayerIndex);
        localPlayerIndex++;
        for (size_t i = 0; i < Heroes.size(); i++)
        {
            if (localPlayerIndex == Heroes[i]->OwnerIndex())
            {
                localHero = (int)i; //29-Jan-23
                break;
            }
            else
            {
                localHero = -1;
            }

        }
        if (localHero == -1)
        {
            return -1;
        }

    }

    float VBE = Heroes[localHero]->IsVisibleByEnemy();
    

    
    /*duplicates.push_front(VBE);
    if (duplicates.size() == threshold + 1)
        duplicates.pop_back();

    if (unique_num() < 2)
    {
        return 0;
    }*/

    if (VBE == 0.0f)
    {
        return 0;
    }
    return 1;
}


void ResetConvars()
{
    if (VEngine)
    {
        weather->var->value.i64 = (0);
        r_farz->var->value.flt = (-1.0f);
        const auto old_val = camera_distance->var->value;
        camera_distance->var->value.flt = (1200.0f);
        fog_enable->var->value.boolean = true;
        particle_hack->var->value.boolean = true;
        drawrange->var->value.flt = (0);
        sv_cheats->var->value.boolean = (0);
        
        
        if (auto callback = VEngine->GetCVarCallback(camera_distance->var->CALLBACK_INDEX); callback)
        {
            callback(ICVar::ConVarID{ .impl = static_cast<std::uint64_t>(3293), .var_ptr = (void*)&camera_distance }, 0, &camera_distance->var->value, &old_val); //works 29-Jan-23
        }
        
        
        /*
         if (auto callback = VEngine->GetCVarCallback(camera_distance->var->CALLBACK_INDEX); callback)
        {
            callback(ICVar::ConVarID{ .impl = static_cast<std::uint64_t>(4000), .var_ptr = (void*)&camera_distance }, 0, &camera_distance->var->value, &old_val); //works 29-Jan-23
        }
        */
        //if (auto callback = VEngine->GetCVarCallback(camera_distance->var->CALLBACK_INDEX); callback) {
            //callback(ICVar::ConVarID{ .impl = static_cast<std::uint64_t>(3293), .var_ptr = (void*)&camera_distance }, 0, &camera_distance->var->value, &old_val);
        //}
    }
}

//void SetSvCheats(int val)
//{
    //sv_cheats->var->value.boolean = (1);
//}
void SetWeather(int val) {
    if (weather)
    {
        weather->var->value.i64 = (val);
    }
}
void SetDrawRange(int val) {
    if (sv_cheats && drawrange)
    {

        sv_cheats->var->value.boolean = (1); //29-Jan-23
        drawrange->var->value.flt = ((float)val); //29-Jan-23
    }
}
void SetParticleHack(int val) {
    if (particle_hack)
    {
        particle_hack->var->value.boolean = (val);

    }
}
void SetNoFog(int val) {
    if (fog_enable)
    {
        fog_enable->var->value.boolean = (val);
    }
}
void SetCamDistance(int val) {
    if (camera_distance && r_farz)
    {
        const auto old_val = camera_distance->var->value;
        camera_distance->var->value.flt = ((float)val); //29-Jan-23
        r_farz->var->value.flt = ((float)(val * 2)); //29-Jan-23
        
        if (auto callback = VEngine->GetCVarCallback(camera_distance->var->CALLBACK_INDEX); callback) 
        {
            callback(ICVar::ConVarID{ .impl = static_cast<std::uint64_t>(3293), .var_ptr = (void*)&camera_distance }, 0, &camera_distance->var->value, &old_val); //works 29-Jan-23
        }
        
        
        
        /*
        if (auto callback = VEngine->GetCVarCallback(camera_distance->var->CALLBACK_INDEX); callback) 
        {
            callback(ICVar::ConVarID{ .impl = static_cast<std::uint64_t>(4000), .var_ptr = (void*)&camera_distance }, 0, &camera_distance->var->value, &old_val); //works 29-Jan-23
        }
        */
        //if (auto callback = VEngine->GetCVarCallback(camera_distance->var->CALLBACK_INDEX); callback) {
            //callback(ICVar::ConVarID{ .impl = static_cast<std::uint64_t>(3293), .var_ptr = (void*)&camera_distance }, 0, &camera_distance->var->value, &old_val);
        //}
    }
}

template <class TipeData> 
void Print(const char* label, TipeData nilai)
{
    std::cout << label << nilai << "\n";
    return;
}

void PrintHero1()
{
    std::cout << "localHero Index Number :\t\t" << localHero << "\n";
    std::cout << "localHero IsAlive :\t\t" << isAlive << "\n";
    std::cout << "localHero isVisibleByEnemy :\t\t" << isVisibleByEnemy << "\n";
    std::cout << "localHero iHealth :\t\t" << iHealth << "\n";
    std::cout << "localHero iMaxHealth :\t\t" << iMaxHealth << "\n";
    std::cout << "localHero iTeamNum :\t\t" << iTeamNum << "\n";
    //std::cout << "localHero enemy bDormant:\t" << bDormant << "\n";

    //std::cout << "localHero iAttackRange :\t" << iAttackRange << "\n";

    //std::cout << "localHero IsAlive :\t" << Heroes[localHero]->IsAlive() << "\n";
    //std::cout << "localHero TeamNum :\t" << Heroes[localHero]->TeamNum() << "\n";
    //std::cout << "localHero AttackRange :\t" << Heroes[localHero]->AttackRange() << "\n";
}
