#pragma once

#include "Editor/FEditor.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Object/UObject.h"
#include "Engine/Renderer/FGrid.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"

class UGrid : public UObject
{
    UCLASS(UGrid, "Grid", UObject)

private:
    FEditor* Editor = nullptr;

public:
    void Initialize(FEditor* InEditor);

    virtual TArray<FPrimitiveRenderData> GetRenderData();
    FMeshResource* GetMeshResource() { return MeshResource; }
    FLineDrawRequest BuildLineDrawRequest(const FGrid& Grid,const FVector& CameraPosition) const;
    FPrimitiveRenderData RenderData;
    FMeshResource* MeshResource;

    virtual void Render() {};
};