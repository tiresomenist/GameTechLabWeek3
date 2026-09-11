#include "pch.h"
#include "FClassRegistry.h"
#include "Engine/Object/FClassType.h"

#include "Editor/Gizmo/UGizmo.h"
#include "Editor/Gizmo/UObjectAxisGizmo.h"
#include "Editor/Gizmo/UWorldAxisGizmo.h"
#include "Editor/Gizmo/UWorldGridGizmo.h"

#include "Editor/UGrid.h"

#include "Engine/Object/UObject.h"
#include "Engine/Actor/UActor.h"
#include "Engine/Component/UActorComponent.h"
#include "Engine/Component/USceneComponent.h"
#include "Engine/Component/UCameraComponent.h"

#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/Primitive/UCubeComponent.h"
#include "Engine/Component/Primitive/UPlaneComponent.h"
#include "Engine/Component/Primitive/USphereComponent.h"
#include "Engine/Component/Primitive/UPepeComponent.h"
#include "Engine/Component/Primitive/UArrowBlueComponent.h"
#include "Engine/Component/Primitive/UArrowGreenComponent.h"
#include "Engine/Component/Primitive/UArrowRedComponent.h"
#include "Engine/Component/Primitive/UOctopusComponent.h"

#include <cassert>
#include <format>

void* FClassRegistry::__INTERNAL__Add(FClassType* Type)
{
	for (auto Item : ClassTypeList)
	{
		if (Item->Name == Type->Name)
		{
			assert(std::format("FClassType.Name이 중복되었습니다. 중복되는 이름: {}", Item->Name).c_str());
		}
	}

	ClassTypeList.Add(Type);
	return nullptr;
}

FClassType* FClassRegistry::FindClassType(FStringView TypeName)
{
	for (auto Item : ClassTypeList)
	{
		if (Item->Name == TypeName)
		{
			return Item;
		}
	}

	return nullptr;
}
