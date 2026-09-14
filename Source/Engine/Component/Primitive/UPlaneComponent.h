#pragma once

#include "UPrimitiveComponent.h"

class UPlaneComponent : public UPrimitiveComponent
{

	UCLASS(UPlaneComponent, "Plane", UPrimitiveComponent)

public:
	virtual void Initialize() override;

};
