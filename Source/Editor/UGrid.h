#pragma once

#include "Editor/FEditor.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Object/UObject.h"

class UGrid : public UObject
{
    UCLASS(UGrid, "Grid", UObject)

private:
    FEditor* Editor = nullptr;

public:
    void Initialize(FEditor* InEditor);

    virtual TArray<FPrimitiveRenderData> GetRenderData();
    FMeshResource* GetMeshResource() { return MeshResource; }

    FPrimitiveRenderData RenderData;
    FMeshResource* MeshResource = nullptr;

    virtual void Render() {};
};