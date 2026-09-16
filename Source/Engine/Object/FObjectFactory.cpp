#include "pch.h"
#include "FObjectFactory.h"

#include <stdexcept>

#include "Engine/Object/FClassType.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Log.h"
#include "Core/Container/TMap.h"

namespace
{
    //수동으로 발급한 이름에 대해서는 어떻게 처리할건지 생각해야됨. (Ex:수동으로 Cube_100을 생성한 경우)
    uint32 GenerateNameNumber(const FClassType* Type)
    {
        // 타입별로 마지막에 자동으로 발급한 내부 Number를 보관함
        static TMap<const FClassType*, uint32> LastNumbers;

        if (uint32* LastNumber = LastNumbers.Find(Type))
        {
            if (*LastNumber == (std::numeric_limits<uint32>::max)())
            {
                throw std::overflow_error("객체 넘버 오버플로우");
            }

            // 기존 타입은 마지막 번호를 증가시켜 반환함
            return ++(*LastNumber);
        }

        // 처음 생성하는 타입은 내부 Number 1로 시작함
        LastNumbers.Add(Type, 1u);
        return 1u;
    }
}

UObject* FObjectFactory::_ConstructObject(FClassType* Type, EObjectDomain Domain, uint32 UUID)
{
    if (!Type) throw std::logic_error("ConstructObject: Type is nullptr");
    if (UUID == static_cast<uint32>(-1)) UUID = GObjectStatics::GenerateUUID(Domain);
    const uint32 Index = GObjectStatics::ReserveSlot();
    UObject* Object = nullptr;
    try
    {
        const FObjectCreateInfo Info{.UUID = UUID, .InternalIndex = Index, .ClassType = Type, .Domain = Domain,.NameNumber=GenerateNameNumber(Type)};
        Object = Type->ClassConstructor(Info);
        if (!Object) throw std::runtime_error("Object construction failed");
        GObjectStatics::CommitSlot(Index, Object);
        Object->Initialize();
        UE_LOG("[Object Created] Class:{} UUID:{} Domain:{} Name:{}", Type->DisplayName, UUID, static_cast<size_t>(Domain),Object->GetName().ToString());
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
