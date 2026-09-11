#pragma once
#include "Container/TArray.h"
#include "Engine/Core.h"
#include "Engine/Object/EObjectDomain.h"

class UObject;
struct FObjectSlot
{
    UObject* Object = nullptr;
    bool Reserved = false;
};

class GObjectStatics
{
    inline static TArray<uint32> NextUUID = TArray<uint32>(static_cast<size_t>(EObjectDomain::MAX_ITEMS));
    inline static TArray<FObjectSlot> Slots;
public:
    static uint32 GenerateUUID(EObjectDomain Domain);
    static uint32 GetNextUUID(EObjectDomain Domain) { return NextUUID[static_cast<size_t>(Domain)]; }
    static void SetNextUUID(EObjectDomain Domain, uint32 UUID);
    static uint32 ReserveSlot();
    static void CommitSlot(uint32 Index, UObject* Object);
    static void CancelSlot(uint32 Index) noexcept;
    static void Unregister(uint32 Index, UObject* Object) noexcept;
    static size_t GetSlotCount() { return Slots.GetVector().size(); }
    static void Release();
};
