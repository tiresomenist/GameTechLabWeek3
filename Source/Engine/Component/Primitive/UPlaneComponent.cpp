#include "pch.h"
#include "UPlaneComponent.h"
#include "Engine/Resource/GResourceManager.h"

FPrimitiveRenderData UPlaneComponent::CreateRenderData(bool bSelected) const
{
	FPrimitiveRenderData RenderData = Super::CreateRenderData(bSelected);
	RenderData.bDoubleSided = true;
	return RenderData;
}
