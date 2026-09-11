#include "FClassRegistry.h"
#include "Engine/Object/FClassType.h"

#include "Engine/Scene/UScene.h"
#include "Engine/Gizmo/UGizmo.h"
#include "Engine/Gizmo/UObjectAxisGizmo.h"
#include "Engine/Gizmo/UWorldAxisGizmo.h"
#include "Engine/Gizmo/UWorldGridGizmo.h"

#include "Engine/Editor/UGrid.h"

#include "Engine/Object/UObject.h"
#include "Engine/Object/UActor.h"
#include "Engine/Object/UActorComponent.h"
#include "Engine/Object/USceneComponent.h"
#include "Engine/Object/UCameraComponent.h"

#include "Engine/Object/Primitive/UPrimitiveComponent.h"
#include "Engine/Object/Primitive/UCubeComponent.h"
#include "Engine/Object/Primitive/UPlaneComponent.h"
#include "Engine/Object/Primitive/USphereComponent.h"
#include "Engine/Object/Primitive/UPepeComponent.h"
#include "Engine/Object/Primitive/UArrowBlueComponent.h"
#include "Engine/Object/Primitive/UArrowGreenComponent.h"
#include "Engine/Object/Primitive/UArrowRedComponent.h"
#include "Engine/Object/Primitive/UOctopusComponent.h"

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
