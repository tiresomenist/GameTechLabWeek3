#pragma once

#include "Core/Core.h"
#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"

// 좌측 상단부터 행 단위로 저장된 RGBA 8비트 이미지 데이터
struct FPNGImage
{
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 RowPitch = 0;

    // 픽셀당 R, G, B, A 순서이며 RGB에 알파를 미리 곱하지 않음
    TArray<unsigned char> Pixels;
};

namespace PNGLoader
{
    // stb_image로 PNG만 디코딩하고 자체 TArray에 픽셀을 보관함
    // UTF-8 경로의 PNG를 읽으며 상대 경로는 현재 작업 디렉터리 기준으로 해석함
    // GPU 리소스는 생성하지 않으며 실패 시 예외를 전달함
    FPNGImage LoadPNG(FStringView FilePath);
}
