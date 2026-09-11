#include "pch.h"
#include "Engine/Renderer/Text/FFontAtlas.h"

#include <fstream>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include "ImGui/imstb_truetype.h"

namespace
{
    // ASCII 95자 + 한글 11,172자를 담으려면 2048로는 부족해서 4096으로 키움
    constexpr int AtlasWidth = 4096;
    constexpr int AtlasHeight = 4096;
}

bool FFontAtlas::Build(ID3D11Device* Device, const FString& TTFPath, float PixelHeight)
{
    // 1. TTF 파일을 통째로 메모리에 읽는다.
    std::ifstream File(TTFPath, std::ios::binary | std::ios::ate);
    if (!File.is_open())
        return false;

    const std::streamsize FileSize = File.tellg();
    File.seekg(0, std::ios::beg);

    std::vector<unsigned char> FontData(static_cast<size_t>(FileSize));
    if (!File.read(reinterpret_cast<char*>(FontData.data()), FileSize))
        return false;

    // 2. CPU 비트맵 버퍼 준비 (1채널, 커버리지 값 0~255)
    std::vector<unsigned char> Bitmap(static_cast<size_t>(AtlasWidth) * AtlasHeight, 0);

    stbtt_pack_context PackContext;
    if (!stbtt_PackBegin(&PackContext, Bitmap.data(), AtlasWidth, AtlasHeight, 0, 1, nullptr))
        return false;

    // 3. ASCII(32~126) + 현대 한글 전체 음절(가~힣, U+AC00~U+D7A3, 11172자)을 함께 굽는다.
    //    현대 한글 음절 블록은 초성19×중성21×종성28 조합이 빈틈없이 이어져 있어서
    //    U+AC00~U+D7A3를 그냥 연속 범위로 잡으면 존재하는 모든 한글 음절을 다 포함하게 된다.
    constexpr int AsciiFirst = 32;
    constexpr int AsciiCount = 95;
    constexpr int HangulFirst = 0xAC00;
    constexpr int HangulCount = 0xD7A3 - 0xAC00 + 1; // 11172

    std::vector<stbtt_packedchar> AsciiPacked(AsciiCount);
    std::vector<stbtt_packedchar> HangulPacked(HangulCount);

    stbtt_pack_range Ranges[2] = {};
    Ranges[0].font_size = PixelHeight;
    Ranges[0].first_unicode_codepoint_in_range = AsciiFirst;
    Ranges[0].num_chars = AsciiCount;
    Ranges[0].chardata_for_range = AsciiPacked.data();

    Ranges[1].font_size = PixelHeight;
    Ranges[1].first_unicode_codepoint_in_range = HangulFirst;
    Ranges[1].num_chars = HangulCount;
    Ranges[1].chardata_for_range = HangulPacked.data();

    const bool bPacked = stbtt_PackFontRanges(&PackContext, FontData.data(), 0, Ranges, 2) != 0;
    stbtt_PackEnd(&PackContext);
    if (!bPacked)
        return false;

    // 4. stb가 채운 packedchar를 우리 FGlyphInfo로 변환해 맵에 저장.
    auto StoreRange = [this](const std::vector<stbtt_packedchar>& Packed, int First)
    {
        for (int i = 0; i < static_cast<int>(Packed.size()); ++i)
        {
            const stbtt_packedchar& PC = Packed[i];
            FGlyphInfo Glyph;
            Glyph.U0 = static_cast<float>(PC.x0) / AtlasWidth;
            Glyph.V0 = static_cast<float>(PC.y0) / AtlasHeight;
            Glyph.U1 = static_cast<float>(PC.x1) / AtlasWidth;
            Glyph.V1 = static_cast<float>(PC.y1) / AtlasHeight;
            Glyph.XOffset = PC.xoff;
            Glyph.YOffset = PC.yoff;
            Glyph.Width = static_cast<float>(PC.x1 - PC.x0);
            Glyph.Height = static_cast<float>(PC.y1 - PC.y0);
            Glyph.XAdvance = PC.xadvance;
            Glyphs[static_cast<char32_t>(First + i)] = Glyph;
        }
    };
    StoreRange(AsciiPacked, AsciiFirst);
    StoreRange(HangulPacked, HangulFirst);

    // 5. CPU 비트맵을 D3D11 텍스처(R8, 1채널)로 업로드.
    D3D11_TEXTURE2D_DESC TexDesc = {};
    TexDesc.Width = AtlasWidth;
    TexDesc.Height = AtlasHeight;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_R8_UNORM;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.Usage = D3D11_USAGE_IMMUTABLE;
    TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA SubResource = {};
    SubResource.pSysMem = Bitmap.data();
    SubResource.SysMemPitch = AtlasWidth;

    if (FAILED(Device->CreateTexture2D(&TexDesc, &SubResource, &AtlasTexture)))
        return false;
    if (FAILED(Device->CreateShaderResourceView(AtlasTexture, nullptr, &AtlasSRV)))
        return false;

    return true;
}

void FFontAtlas::Release()
{
    if (AtlasSRV) { AtlasSRV->Release(); AtlasSRV = nullptr; }
    if (AtlasTexture) { AtlasTexture->Release(); AtlasTexture = nullptr; }
    Glyphs.clear();
}

const FGlyphInfo* FFontAtlas::FindGlyph(char32_t Codepoint) const
{
    const auto It = Glyphs.find(Codepoint);
    return It != Glyphs.end() ? &It->second : nullptr;
}