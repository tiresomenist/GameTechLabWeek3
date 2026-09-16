#include "pch.h"
#include "FClassRegistry.h"
#include "Engine/Object/FClassType.h"

#include "Editor/Gizmo/UGizmo.h"
#include "Editor/Gizmo/UObjectAxisGizmo.h"
#include "Editor/Gizmo/UWorldAxisGizmo.h"
#include "Editor/Gizmo/UWorldGridGizmo.h"

#include "Editor/UGrid.h"

#include "Engine/Object/UObject.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/UActorComponent.h"
#include "Engine/Component/USceneComponent.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Component/UStaticMeshComponent.h"

#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/Primitive/UTextComponent.h"
#include "Engine/Component/Primitive/UFlipbookComponent.h"

#include "Engine/Component/Light/ULightComponent.h"
#include "Engine/Component/Light/USpotLightComponent.h"
#include "Core/Container/TMap.h"
#include <stdexcept>
#include <format>


namespace
{
	TMap<FName, FClassType*>& GetClassTypeMap()
	{
		// 최초 등록 또는 조회 시 레지스트리를 생성함
		static TMap<FName, FClassType*> ClassTypes;
		return ClassTypes;
	}
}

void* FClassRegistry::__INTERNAL__Add(FClassType* Type)
{
	if (Type == nullptr){ throw std::invalid_argument("Class type is null"); }

	if (Type->Name.IsNone()){ throw std::invalid_argument("Class name is None"); }

	auto& ClassTypes = GetClassTypeMap();

	if (FClassType** Existing = ClassTypes.Find(Type->Name))
	{
		// 동일 클래스의 재등록은 허용함
		if (*Existing == Type){	return nullptr; }

		// 동일 이름을 사용하는 다른 클래스의 등록을 거부함
		throw std::logic_error(std::format("FClassType.Name이 중복되었습니다. 중복되는 이름: {}",Type->DisplayName)
		);
	}

	ClassTypes.Add(Type->Name, Type);
	return nullptr;
}

FClassType* FClassRegistry::FindClassType(const FName& TypeName)
{
	if (TypeName.IsNone()){	return nullptr; }

	if (FClassType** Found = GetClassTypeMap().Find(TypeName)){	return *Found; }

	return nullptr;
}
