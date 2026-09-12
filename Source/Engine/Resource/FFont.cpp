#include "pch.h"
#include "Engine/Resource/FFont.h"
#include "Engine/Log.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Core/Util/File.h"
#include <cstring>
#include <DirectXTex.h>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

namespace
{
    using Microsoft::WRL::ComPtr;
    constexpr UINT AtlasColumns = 64;
    constexpr UINT AtlasRows = 64;
    constexpr uint32 JamoStartCell = 256;
    constexpr uint32 HangulStartCell = JamoStartCell + 51;
    constexpr int HangulCount = 2350;

    void Require(HRESULT Result, const char* Operation)
    {
        if (FAILED(Result))
            throw std::runtime_error(std::format("Font atlas: {} failed (0x{:08X})",
                Operation, static_cast<unsigned long>(Result)));
    }

    struct FCOMScope
    {
        HRESULT Result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        FCOMScope()
        {
            // WIC also works if this thread already has an STA apartment.
            if (Result != RPC_E_CHANGED_MODE) Require(Result, "COM initialization");
        }
        ~FCOMScope() { if (SUCCEEDED(Result)) CoUninitialize(); }
    };

    void UploadBuffer(ID3D11Device* Device, ID3D11DeviceContext* Context,
        ComPtr<ID3D11Buffer>& Buffer, UINT& Capacity, UINT BindFlags,
        const void* Data, UINT ByteCount)
    {
        if (Capacity < ByteCount)
        {
            D3D11_BUFFER_DESC Desc{};
            Desc.ByteWidth = (std::max)(ByteCount, Capacity * 2);
            Desc.Usage = D3D11_USAGE_DYNAMIC;
            Desc.BindFlags = BindFlags;
            Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            ComPtr<ID3D11Buffer> NewBuffer;
            Require(Device->CreateBuffer(&Desc, nullptr, NewBuffer.GetAddressOf()), "text buffer");
            Buffer = std::move(NewBuffer);
            Capacity = Desc.ByteWidth;
        }
        D3D11_MAPPED_SUBRESOURCE Mapped{};
        Require(Context->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped), "text buffer map");
        std::memcpy(Mapped.pData, Data, ByteCount);
        Context->Unmap(Buffer.Get(), 0);
    }
}
namespace UTF8
{
    // UTF-8한글을 유니코드로 디코딩하는 함수
    inline TArray<uint32> Decode(FStringView Text)
    {
        if (Text.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
            throw std::length_error("UTF-8 text exceeds TArray capacity");
        TArray<uint32> Result;
        Result.Reserve(Text.size());
        size_t Offset = 0;
        while (Offset < Text.size())
        {
            const auto First = static_cast<unsigned char>(Text[Offset++]);
            if (First < 0x80)
            {
                Result.Add(First);
                continue;
            }

            uint32 Code = 0;
            uint32 Minimum = 0;
            int Remaining = 0;
            if (First >= 0xC2 && First <= 0xDF)
            {
                Code = First & 0x1F; Minimum = 0x80; Remaining = 1;
            }
            else if (First >= 0xE0 && First <= 0xEF)
            {
                Code = First & 0x0F; Minimum = 0x800; Remaining = 2;
            }
            else if (First >= 0xF0 && First <= 0xF4)
            {
                Code = First & 0x07; Minimum = 0x10000; Remaining = 3;
            }
            else
            {
                Result.Add(0xFFFD);
                continue;
            }

            while (Remaining > 0 && Offset < Text.size())
            {
                const auto Next = static_cast<unsigned char>(Text[Offset]);
                if ((Next & 0xC0) != 0x80) break;
                Code = (Code << 6) | (Next & 0x3F);
                ++Offset;
                --Remaining;
            }
            if (Remaining != 0 || Code < Minimum || Code > 0x10FFFF ||
                (Code >= 0xD800 && Code <= 0xDFFF))
                Result.Add(0xFFFD);
            else
                Result.Add(Code);
        }
        return Result;
    }
}
void FFont::Initialize(ID3D11Device* Device)
{
    Release();
    // DirectXTex caches its WIC factory across loads. Keep this thread's COM
    // apartment alive across Initialize/Release instead of ending it per image.
    static thread_local FCOMScope COM;
    DirectX::ScratchImage Image;

    // 1. PNG를 읽어 CPU 이미지 데이터 생성
    Require(DirectX::LoadFromWICFile(L"Assets/Fonts/PretendardVariable.png",DirectX::WIC_FLAGS_NONE,nullptr,Image),"PNG load");

    const DirectX::TexMetadata& Metadata = Image.GetMetadata();

    if (Metadata.width < AtlasColumns || Metadata.height < AtlasRows ||
        Metadata.width % AtlasColumns != 0 || Metadata.height % AtlasRows != 0 ||
        Metadata.width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
        Metadata.height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
    {
        throw std::runtime_error("Font atlas: unsupported image dimensions");
    }

    TextureWidth = static_cast<UINT>(Metadata.width);
    TextureHeight = static_cast<UINT>(Metadata.height);

    // 2. CPU 이미지에서 GPU 텍스처와 SRV 생성
    Require(DirectX::CreateShaderResourceView(Device,Image.GetImages(),Image.GetImageCount(),Metadata,TextureView.GetAddressOf()),"texture SRV");

    // 256 아스키코드, 51 한글 자음모음, 2350 한글 글자 
    const float InsetU = 0.5f / TextureWidth;
    const float InsetV = 0.5f / TextureHeight;
    const auto AddCharacter = [&](uint32 Code, uint32 Cell)
    {
        if (Cell >= AtlasColumns * AtlasRows || Characters.Find(Code))
            throw std::runtime_error("Invalid or duplicate font atlas mapping");
        Characters.Add(Code, {
            (Cell % AtlasColumns) / float(AtlasColumns) + InsetU,
            (Cell / AtlasColumns) / float(AtlasRows) + InsetV,
            1.0f / AtlasColumns - 2 * InsetU,
            1.0f / AtlasRows - 2 * InsetV
        });
    };
    for (uint32 Code = 32; Code <= 126; ++Code)
        AddCharacter(Code, Code);
    for (uint32 Code = 0x3131; Code <= 0x3163; ++Code)
        AddCharacter(Code, JamoStartCell + Code - 0x3131);

    const auto HangulCodes = UTF8::Decode(File::ReadText("Assets/Fonts/hangul_2350.txt"));
    uint32 Cell = HangulStartCell;
    for (uint32 Code : HangulCodes)
    {
        // The exported UTF-8 list may contain a BOM or formatting whitespace.
        if (Code == 0xFEFF || Code == '\r' || Code == '\n' || Code == ' ' || Code == '\t') continue;
        if (Code < 0xAC00 || Code > 0xD7A3)
            throw std::runtime_error("Hangul atlas list contains a non-Hangul code point");
        AddCharacter(Code, Cell++);
    }
    if (Cell - HangulStartCell != HangulCount)
        throw std::runtime_error("Hangul atlas requires exactly 2350 characters in baked order");

    UE_LOG("Font atlas loaded: {} x {}, {} mapped characters", TextureWidth, TextureHeight, Characters.Num());
}

const FCharacterInfo& FFont::GetCharacterInfo(uint32 CharacterCode) const
{
    if (const FCharacterInfo* Info = Characters.Find(CharacterCode)) return *Info;
    if (const FCharacterInfo* Fallback = Characters.Find('?')) return *Fallback;
    // Safe lookup before initialization or after Release; never insert on lookup.
    static const FCharacterInfo EmptyCharacter{};
    return EmptyCharacter;
}

FFontMeshData FFont::BuildMesh(FStringView Text, const FVector& Center,const FVector& CameraRight, const FVector& CameraUp, float CharacterHeight) const
{
    FFontMeshData Mesh;
    if (!TextureView || Text.empty() || !std::isfinite(CharacterHeight) || CharacterHeight <= 0)    //예외처리
        return Mesh;
 
    // 한 글자의 Width 계산
    const float Width = CharacterHeight * (float(TextureWidth) / AtlasColumns) / (float(TextureHeight) / AtlasRows);

    const TArray<uint32> CodePoints = UTF8::Decode(Text);
    Mesh.Vertices.Reserve(static_cast<size_t>(CodePoints.Num()) * 4);
    Mesh.Indices.Reserve(static_cast<size_t>(CodePoints.Num()) * 6);

    // 텍스트 세로줄 계산을 위함
    int LineStart = 0;
    float LineY = 0;
    while (LineStart < CodePoints.Num())
    {
        int LineEnd = LineStart;
        while (LineEnd < CodePoints.Num() && CodePoints[LineEnd] != '\n') ++LineEnd;

        size_t Columns = 0; //텍스트 가로 길이 탐색
        for (int i = LineStart; i < LineEnd; ++i)
            if (CodePoints[i] != '\r') Columns += CodePoints[i] == '\t' ? 4 : 1;    //탭이면 4칸, 그외면 1칸
        float X = -float(Columns) * Width * 0.5f;   // 총 가로 길이의 절반만큼 시작점 이동 -> 중앙정렬

        //현재 줄의 각 문자에 대해
        for (int i = LineStart; i < LineEnd; ++i)
        {
            const uint32 Character = CodePoints[i];
            //공백, \r 처리
            if (Character == '\r') continue;
            if (Character == ' ' || Character == '\t')  
            {
                X += Width * (Character == '\t' ? 4 : 1);
                continue;
            }
            const FCharacterInfo& Glyph = GetCharacterInfo(Character);  //유니코드 코드 포인트로 UV 조회

            const auto AddVertex = [&](float LocalX, float LocalY, float U, float V)    //정점생성 람다함수
            {
                const FVector Position = Center + CameraRight * LocalX + CameraUp * LocalY; //빌보드 렌더링용 계산
                Mesh.Vertices.Add({Position.X, Position.Y, Position.Z, U, V});
            };

            const uint32_t Base = static_cast<uint32_t>(Mesh.Vertices.Num());   //현재 plane index 시작점
            
            //왼쪽위-오른쪽위-왼쪽아래-오른쪽아래
            AddVertex(X, LineY + CharacterHeight * 0.5f, Glyph.u, Glyph.v);
            AddVertex(X + Width, LineY + CharacterHeight * 0.5f, Glyph.u + Glyph.width, Glyph.v);
            AddVertex(X, LineY - CharacterHeight * 0.5f, Glyph.u, Glyph.v + Glyph.height);
            AddVertex(X + Width, LineY - CharacterHeight * 0.5f, Glyph.u + Glyph.width, Glyph.v + Glyph.height);

            Mesh.Indices.Append({Base, Base + 1, Base + 2, Base + 2, Base + 1, Base + 3});

            X += Width;
        }

        LineY -= CharacterHeight * 1.2f;    //줄 위치 내림
        LineStart = LineEnd + 1;
    }
    return Mesh;
}

void FFont::UpdateMesh(const FFontMeshData& Mesh)
{
    ID3D11Device* Device = GDevice::GetInstance()->GetDevice();
    ID3D11DeviceContext* Context = GDevice::GetInstance()->GetContext();
    IndexCount = 0;
    if (Mesh.Vertices.IsEmpty() || Mesh.Indices.IsEmpty()) return;
    
    UploadBuffer(Device, Context, VertexBuffer, VertexCapacityBytes, D3D11_BIND_VERTEX_BUFFER,
        Mesh.Vertices.GetData(), static_cast<UINT>(Mesh.Vertices.Num() * sizeof(FFontVertex)));
    UploadBuffer(Device, Context, IndexBuffer, IndexCapacityBytes, D3D11_BIND_INDEX_BUFFER,
        Mesh.Indices.GetData(), static_cast<UINT>(Mesh.Indices.Num() * sizeof(uint32_t)));
    IndexCount = static_cast<UINT>(Mesh.Indices.Num());
}

FPrimitiveRenderData FFont::CreateRenderData() const
{
    FPrimitiveRenderData Data{};
    if (!TextureView || !VertexBuffer || !IndexBuffer || IndexCount == 0) return Data;
    Data.VertexBuffer = VertexBuffer.Get();
    Data.IndexBuffer = IndexBuffer.Get();
    Data.Stride = sizeof(FFontVertex);
    Data.IndexCount = IndexCount;
    Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    Data.Material = TextureView.Get();
    // BuildMesh already produced world-space vertices. RenderText applies only VP.
    return Data;
}

void FFont::Release()
{
    TextureView.Reset();
    VertexBuffer.Reset();
    IndexBuffer.Reset();
    Characters.Empty();
    VertexCapacityBytes = IndexCapacityBytes = IndexCount = 0;
    TextureWidth = TextureHeight = 0;
}

FPrimitiveRenderData FFont::BuildRenderList(const TArray<FTextDrawRequest>& InRequests, FVector InRight, FVector InUp)
{
    FFontMeshData CombinedMesh;
    
    for (const auto& Request : InRequests) {
        const FFontMeshData Mesh = BuildMesh(Request.Text, Request.WorldPosition, InRight, InUp, Request.CharacterHeight);
        if (Mesh.Indices.IsEmpty())continue;
        
        const uint32 VertexBase = CombinedMesh.Vertices.Num();

        for (const auto& Vertex : Mesh.Vertices) {
            CombinedMesh.Vertices.Add(Vertex);
        }
        for (uint32 Index : Mesh.Indices) {
            CombinedMesh.Indices.Add(VertexBase + Index);
        }
        
    }
    UpdateMesh(CombinedMesh);   

    return CreateRenderData();
}
