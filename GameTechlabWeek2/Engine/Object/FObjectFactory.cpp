#include "pch.h"
#include "FObjectFactory.h"

#include <stdexcept>

#include "Engine/Object/FClassType.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Log.h"

UObject* FObjectFactory::_ConstructObject(FClassType* Type, EObjectDomain Domain, uint32 UUID)
{
    if (!Type) throw std::logic_error("ConstructObject: Type is nullptr");
    if (UUID == static_cast<uint32>(-1)) UUID = GObjectStatics::GenerateUUID(Domain);
    const uint32 Index = GObjectStatics::ReserveSlot();
    UObject* Object = nullptr;
    try
    {
        const FObjectCreateInfo Info{.UUID = UUID, .InternalIndex = Index, .ClassType = Type, .Domain = Domain};
        Object = Type->ClassConstructor(Info);
        if (!Object) throw std::runtime_error("Object construction failed");
        GObjectStatics::CommitSlot(Index, Object);
        Object->Initialize();
        UE_LOG("[Object Created] Name:{} UUID:{} Domain:{}", Type->Name, UUID, static_cast<size_t>(Domain));
        return Object;
    }
    catch (...)
    {
        if (Object) delete Object;
        else GObjectStatics::CancelSlot(Index);
        throw;
    }
}

UObject* FObjectFactory::ConstructSceneObject(FClassType* Type, uint32 UUID)
{
	return _ConstructObject(Type, EObjectDomain::EOT_Scene, UUID);
}

UObject* FObjectFactory::ConstructEngineObject(FClassType* Type)
{
	return _ConstructObject(Type, EObjectDomain::EOT_Engine, -1);
}

UObject* FObjectFactory::ConstructEditorObject(FClassType* Type)
{
	return _ConstructObject(Type, EObjectDomain::EOT_Editor, -1);
}
