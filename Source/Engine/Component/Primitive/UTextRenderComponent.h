#pragma once

#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Core/Container/FString.h"
#include "Engine/Renderer/FRenderer.h"
#include "Engine/Object/FArchive.h"

class UTextRenderComponent : public UPrimitiveComponent
{
    UCLASS(UTextRenderComponent, "Text", UPrimitiveComponent)

public:
    bool bUseBillboard = true;
    FMatrix BillboardMatrix = FMatrix::Identity;

public:
    UTextRenderComponent() = default;
    virtual ~UTextRenderComponent();

    virtual void Initialize() override;
    virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;

    void UpdateBillboard(const FMatrix& ViewMatrix, float HeadOffsetZ = 120.0f);

    // 텍스트 및 속성 설정
    void SetText(const std::wstring& InText);
    const std::wstring& GetText() const { return Text; }

    void SetTextSize(float InSize);
    float GetTextSize() const { return TextSize; }

    // 직렬화 지원 (씬 로드/저장 연동)
    virtual void Serialize(FArchive& Archive) override;
    virtual void Deserialize(FArchive& Archive) override;

private:
    void RebuildTextMesh();
    void ReleaseBuffers();

    std::wstring Text = L"Text";
    float TextSize = 1.0f;

    ID3D11Buffer* VertexBuffer = nullptr;
    ID3D11Buffer* IndexBuffer = nullptr;
    UINT IndexCount = 0;
};