#pragma once

#include "Engine/Object/EObjectDomain.h"
#include "Core/Core.h"

struct FClassType;
class UObject;

class FObjectFactory
{

private: 

	static UObject* _ConstructObject(FClassType* Type, EObjectDomain Domain, uint32 UUID);

public:

	static UObject* ConstructSceneObject(FClassType* Type, uint32 UUID = -1);
	static UObject* ConstructEngineObject(FClassType* Type);
	static UObject* ConstructEditorObject(FClassType* Type);
};
