#pragma once
#include "common.h"
uint64_t SchemaSystem;
//uint64_t Panorama; //29-Jan-23

int vstrcmp(uint64_t a, uint64_t b) {
    return strcmp((cc)a, (cc)b);
}
int vstrcmp(cc a, uint64_t b) {
    return strcmp(a, (cc)b);
}
int vstrcmp(uint64_t a, cc b) {
    return strcmp((cc)a, b);
}
int vstrcmp(cc a, cc b) {
    return strcmp(a, b);
}
typedef void(__fastcall* ConMsg)(cc, uint64_t, uint64_t, uint64_t);
ConMsg CMsg = 0;
void CMSG(cc pure) {

    CMsg(pure, 0, 0, 0);

}

void CMSG(cc format, uint64_t p1) {

    CMsg(format, p1, 0, 0);

}

void CMSG(cc format, uint64_t p1, uint64_t p2) {

    CMsg(format, p1, p2, 0);

}

template<typename T, uint64_t SIZE>
class CArray {
public:
    T* elms[SIZE] = { 0 };
    uint64_t count = 0;
    void operator=(T* elem) {
        elms[count++] = elem;
    };
    bool operator!() {
        return count == 0;
    }
    explicit operator bool() const
    {
        return count != 0;
    }
    T** begin() {
        return &elms[0];
    }
    T** end() {
        return &elms[count];
    }
    __forceinline T* last() {
        return elms[count - 1];
    }
    __forceinline T* first() {
        return elms[0];
    }
    void RemoveAndShift(T* elm) {
        for (uint64_t i = 0; i < count; i++) {
            if (elms[i] == elm) {
                T* aa = elms[i];
                for (uint64_t j = i; j < count; j++) {
                    if (elms[j + 1]) elms[j] = elms[j + 1];
                    else {
                        elms[j] = 0;
                        break;
                    }
                }
                count--;

            }
        }
    }
    void Destroy() {
        for (uint64_t i = 0; i < count; i++) {
            delete elms[i];
        }
    }
};
struct ClassDescription;
struct SchemaParent {
    uint64_t idk;
    ClassDescription* parent;
};
struct ClassDescription {
    uint64_t idk;//0
    uint64_t classname;//8
    uint64_t modulename;//10
    int sizeofclass;//18
    short memberstoiterate;//1c
    char pad[6];//20
    uint64_t MemberInfo;//28
    uint64_t idk2;//30
    SchemaParent* parent;//38
};

struct SchemaTypeDescription {
    uint64_t idk;
    uint64_t name;
    uint64_t idk2;
};
struct MemberDescription {
    uint64_t name;
    SchemaTypeDescription* schematypeptr;
    int offset;
    int idk;
    uint64_t idk2;
};
class schemanetvar {
public:
    uint64_t classname;
    uint64_t name;
    uint64_t _typename;
    int offset;
    schemanetvar(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
        classname = a;
        name = b;
        _typename = c;
        offset = (int)d; //29-Jan-23
    }
};
class SchemaNetvarCollection {
public:
    CArray<schemanetvar, 1000> Netvars;
    void Add(cc _class, cc _module) {
        uint64_t Scope = ((uint64_t(__fastcall*)(uint64_t schemasys, const char* _mod))
            (*(uint64_t*)(*(uint64_t*)(SchemaSystem)+0x68)))(SchemaSystem, _module);
        if (!Scope) { CMSG("No such scope %s!\n", (uint64_t)_module); return; }
        uint64_t Class = ((uint64_t(__fastcall*)(uint64_t scope, const char* _class))
            (*(uint64_t*)(*(uint64_t*)(Scope)+0x10)))(Scope, _class);
        if (!Class) { CMSG("No such class %s!\n", (uint64_t)_class); return; }
        ClassDescription* a = (ClassDescription*)Class;
        for (uint64_t i = 0; i < a->memberstoiterate; i++) {
            MemberDescription* z = (MemberDescription*)(a->MemberInfo + i * 0x20);
            Netvars = new schemanetvar(a->classname, z->name, z->schematypeptr->name, (uint64_t)z->offset);
        }
    }
    schemanetvar* Get(uint64_t name) {
        for (schemanetvar* netvar : Netvars) {
            if (!vstrcmp(netvar->name, name)) return netvar;
        }
        CMSG("no such netvar found in manager: %s\n", name);
    }
    schemanetvar* Get(uint64_t _class, uint64_t name) {
        for (schemanetvar* netvar : Netvars) {
            if (!vstrcmp(netvar->name, name) && !vstrcmp(netvar->classname, _class)) return netvar;
        }
        CMSG("no such netvar found in manager: %s\n", name);

    }
};