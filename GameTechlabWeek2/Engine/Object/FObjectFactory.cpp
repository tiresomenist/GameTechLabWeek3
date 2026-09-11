#include "FObjectFactory.h"

#include <stdexcept>

#include "Engine/Object/FClassType.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Log.h"

UObject* FObjectFactory::_ConstructObject(FClassType* Type, EObjectDomain Domain, uint32 UUID)
{
	if (Type == nullptr)
	{
		throw std::logic_error("ConstructObject: Type is nullptr");
	}

	if (UUID == -1)
	{
		// Note: uint32에서 -1 ==> (2^32 - 1)로 변환됨
		UUID = GObjectStatics::GenerateUUID(Domain);
	}

	const FObjectCreateInfo Info
	{
		.UUID = UUID,
		.InternalIndex = GObjectStatics::GetNextIndex(),
		.ClassType = Type,
		.Domain = Domain,
	};

	UE_LOG("[Object Created] Name:{} UUID:{} Domain:{} ", Type->Name, UUID, static_cast<size_t>(Domain));

	//
	UObject* Object = Type->ClassConstructor(Info);

	Object->Initialize();

	GObjectStatics::AddObject(Object);

	return Object;
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
