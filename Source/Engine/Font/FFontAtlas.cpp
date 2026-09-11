#include "pch.h"
#include "FFontAtlas.h"
bool FFontAtlas::Initialize(ID3D11Device* InDevice, const std::wstring& InFilePath)
{
    if (!InDevice) return false;

    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        InDevice,
        InFilePath.c_str(),
        AtlasTexture.ReleaseAndGetAddressOf(),
        TextureSRV.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr)) return false;

    BuildCharacterMap();
	return true;
}

bool FFontAtlas::GetCharacterInfo(wchar_t InChar, CharacterInfo& OutInfo) const
{
    const CharacterInfo* Found = CharInfoMap.Find(InChar);
    if (Found)
    {
        OutInfo = *Found;
        return true;
    }

    return false;
}

void FFontAtlas::BuildCharacterMap()
{
    const std::wstring AtlasChars =
        L"0123456789 !\"#$%&'()*+,-./:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
        L"오브젝트액터이름플레어적아템체력마나공격방속도위치회전크기카메라월드좌표값상태성실패시작종료벨점수간거리초분프임인덱스버퍼텍처엔진렌더가다바사자차타파하한글테";

    for (size_t i = 0; i < AtlasChars.length(); ++i)
    {
        wchar_t ch = AtlasChars[i];

        int Col = static_cast<int>(i % GridCols);
        int Row = static_cast<int>(i / GridRows);

        CharacterInfo Info;

        Info.u = Col * CellUVSize;
        Info.v = Row * CellUVSize;

        Info.width = CellUVSize;
        Info.height = CellUVSize;

        CharInfoMap[ch] = Info;
    }
}

void FFontAtlas::Release()
{
    TextureSRV.Reset();
    AtlasTexture.Reset();
    CharInfoMap.Clear();
}