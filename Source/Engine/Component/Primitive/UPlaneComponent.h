#pragma once

#include "UPrimitiveComponent.h"

class UPlaneComponent : public UPrimitiveComponent
{

	UCLASS(UPlaneComponent, "Plane", UPrimitiveComponent)

public:
	virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;
};
