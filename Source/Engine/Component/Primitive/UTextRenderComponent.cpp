#include "pch.h"
#include "UTextRenderComponent.h"
#include "Engine/Font/FFontAtlas.h"
#include "Engine/Renderer/GDevice.h"


namespace
{
    std::string WStringToUTF8(const std::wstring& WStr)
    {
        if (WStr.empty()) return "";
        int SizeNeeded = WideCharToMultiByte(CP_UTF8, 0, WStr.data(), static_cast<int>(WStr.size()), nullptr, 0, nullptr, nullptr);
        std::string Result(SizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, WStr.data(), static_cast<int>(WStr.size()), Result.data(), SizeNeeded, nullptr, nullptr);
        return Result;
    }

    std::wstring UTF8ToWString(const std::string& Str)
    {
        if (Str.empty()) return L"";
        int SizeNeeded = MultiByteToWideChar(CP_UTF8, 0, Str.data(), static_cast<int>(Str.size()), nullptr, 0);
        std::wstring Result(SizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, Str.data(), static_cast<int>(Str.size()), Result.data(), SizeNeeded);
        return Result;
    }
}


UTextRenderComponent::~UTextRenderComponent()
{
    ReleaseBuffers();
}

void UTextRenderComponent::Initialize()
{
    Super::Initialize();
    if (Text.empty())
    {
        Text = L"Text";
    }
    RebuildTextMesh();
}

void UTextRenderComponent::SetText(const std::wstring& InText)
{
    if (Text == InText) return;
    Text = InText;
    RebuildTextMesh();
}

void UTextRenderComponent::SetTextSize(float InSize)
{
    if (TextSize == InSize) return;
    TextSize = InSize;
    RebuildTextMesh();
}

void UTextRenderComponent::ReleaseBuffers()
{
    if (VertexBuffer) { VertexBuffer->Release(); VertexBuffer = nullptr; }
    if (IndexBuffer) { IndexBuffer->Release();  IndexBuffer = nullptr; }
    IndexCount = 0;
}

void UTextRenderComponent::RebuildTextMesh()
{
    ReleaseBuffers();
    if (Text.empty()) return;

    FFontAtlas& Atlas = FFontAtlas::GetInstance();
    TArray<FFontVertex> Vertices;
    TArray<uint32> Indices;

    float CursorY = 0.0f; // 엔진 좌표계(+Y가 Right)에 맞춤
    uint32 VertexOffset = 0;

    for (wchar_t Ch : Text)
    {
        if (Ch == L' ')
        {
            CursorY += TextSize * 0.5f;
            continue;
        }

        CharacterInfo Info{};
        if (!Atlas.GetCharacterInfo(Ch, Info))
        {
            CursorY += TextSize * 0.5f;
            continue;
        }

        float AspectRatio = (Info.height > 0.0f) ? (Info.width / Info.height) : 1.0f;
        float CharW = TextSize * AspectRatio;
        float HalfH = TextSize * 0.5f;

        // X=0 (Forward 평면), Y=가로(Right), Z=세로(Up)
        FFontVertex V[4] =
        {
            { 0.0f, CursorY,          HalfH,  Info.u,              Info.v },
            { 0.0f, CursorY + CharW,  HalfH,  Info.u + Info.width, Info.v },
            { 0.0f, CursorY + CharW, -HalfH,  Info.u + Info.width, Info.v + Info.height },
            { 0.0f, CursorY,         -HalfH,  Info.u,              Info.v + Info.height }
        };

        for (int i = 0; i < 4; ++i)
        {
            Vertices.Add(V[i]);
        }

        Indices.Add(VertexOffset + 0);
        Indices.Add(VertexOffset + 1);
        Indices.Add(VertexOffset + 2);

        Indices.Add(VertexOffset + 0);
        Indices.Add(VertexOffset + 2);
        Indices.Add(VertexOffset + 3);

        VertexOffset += 4;
        CursorY += CharW;
    }

    if (Vertices.IsEmpty()) return;

    ID3D11Device* D3DDevice = GDevice::GetInstance()->GetDevice();

    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(FFontVertex) * Vertices.Num());
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData{ Vertices.GetData() };
    D3DDevice->CreateBuffer(&vbDesc, &vbData, &VertexBuffer);

    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32) * Indices.Num());
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData{ Indices.GetData() };
    D3DDevice->CreateBuffer(&ibDesc, &ibData, &IndexBuffer);

    IndexCount = static_cast<UINT>(Indices.Num());
}

void UTextRenderComponent::UpdateBillboard(const FMatrix& ViewMatrix, float HeadOffsetZ)
{
    FVector TextPos = GetWorldLocation() + FVector(0.0f, 0.0f, HeadOffsetZ);

    FVector Scale = GetRelativeScale3D();

    FMatrix InvView = ViewMatrix.Inverse();

    BillboardMatrix = FMatrix(
        InvView.M[0][0] * Scale.X, InvView.M[0][1] * Scale.X, InvView.M[0][2] * Scale.X, 0.0f,
        InvView.M[1][0] * Scale.Y, InvView.M[1][1] * Scale.Y, InvView.M[1][2] * Scale.Y, 0.0f,
        InvView.M[2][0] * Scale.Z, InvView.M[2][1] * Scale.Z, InvView.M[2][2] * Scale.Z, 0.0f,
        TextPos.X, TextPos.Y, TextPos.Z, 1.0f
    );
}

FPrimitiveRenderData UTextRenderComponent::CreateRenderData(bool bSelected) const
{
    FPrimitiveRenderData OutData{};
    if (!VertexBuffer || !IndexBuffer || IndexCount == 0) return OutData;

    OutData.VertexBuffer = VertexBuffer;
    OutData.IndexBuffer = IndexBuffer;
    OutData.IndexCount = IndexCount;
    OutData.Stride = sizeof(FFontVertex);
    OutData.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    if (bUseBillboard)   OutData.WorldMatrix = &BillboardMatrix;
    else   OutData.WorldMatrix = &GetWorldMatrix();
    OutData.isSelected = bSelected;
    OutData.RenderPass = ERenderPass::Translucent;

    return OutData;
}

void UTextRenderComponent::Serialize(FArchive& Archive)
{
    Super::Serialize(Archive);
    Archive.SetFloat("TextSize", TextSize);

    FString UTF8Text = WStringToUTF8(Text);
    Archive.SetString("Text", FString(UTF8Text.c_str()));
}

void UTextRenderComponent::Deserialize(FArchive& Archive)
{
    Super::Deserialize(Archive);

    try
    {
        TextSize = Archive.GetFloat("TextSize");

        FString LoadedText = Archive.GetString("Text");
        Text = UTF8ToWString(FString(LoadedText.data()));

        RebuildTextMesh();
    }
    catch (const std::exception&)
    {
        TextSize = 1.0f;
        Text = L"";
    }
}