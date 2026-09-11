#include "pch.h"
#include "GObjectStatics.h"
#include "Engine/Object/UObject.h"
#include <limits>
#include <stdexcept>

uint32 GObjectStatics::GenerateUUID(EObjectDomain Domain)
{
    auto& Next = NextUUID[static_cast<size_t>(Domain)];
    if (Next == (std::numeric_limits<uint32>::max)())
        throw std::overflow_error("UUID exhausted");
    return Next++;
}

void GObjectStatics::SetNextUUID(EObjectDomain Domain, uint32 UUID)
{
    NextUUID[static_cast<size_t>(Domain)] = UUID;
}

uint32 GObjectStatics::ReserveSlot()
{
    for (size_t I = 0; I < Slots.Num(); ++I)
        if (!Slots[I].Reserved)
        {
            Slots[I].Reserved = true;
            return static_cast<uint32>(I);
        }
    if (Slots.Num() >= static_cast<size_t>((std::numeric_limits<int>::max)()))
        throw std::overflow_error("Object slot limit");
    const uint32 Index = static_cast<uint32>(Slots.Num());
    Slots.Add(FObjectSlot{nullptr, true});
    return Index;
}

void GObjectStatics::CommitSlot(uint32 Index, UObject* Object)
{
    Slots[Index].Object = Object;
}

void GObjectStatics::CancelSlot(uint32 Index) noexcept
{
    if (Index < Slots.Num() && Slots[Index].Object == nullptr)
        Slots[Index] = FObjectSlot{};
}

void GObjectStatics::Unregister(uint32 Index, UObject* Object) noexcept
{
    if (Index < Slots.Num() && Slots[Index].Object == Object)
        Slots[Index] = FObjectSlot{};
}

void GObjectStatics::Release()
{
    for (size_t I = 0; I < Slots.Num(); ++I)
        delete Slots[I].Object;
    Slots.Empty();
}
