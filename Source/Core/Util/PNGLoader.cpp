#include "pch.h"
#include "Core/Util/PNGLoader.h"

#include <cstring>
#include <format>
#include <limits>
#include <memory>
#include <stdexcept>

// PNG 디코더 구현은 이 CPP에서만 생성함
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_WINDOWS_UTF8
#include "stb/stb_image.h"

FPNGImage PNGLoader::LoadPNG(FStringView FilePath)
{
    if (FilePath.empty() || FilePath.find('\0') != FStringView::npos ||
        FilePath.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        throw std::invalid_argument("Invalid PNG file path");
    }

    // 잘못된 UTF-8 경로가 다른 파일명으로 대체되는 것을 방지함
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, FilePath.data(),
        static_cast<int>(FilePath.size()), nullptr, 0) == 0)
    {
        throw std::invalid_argument("PNG file path is not valid UTF-8");
    }

    // FStringView가 부분 문자열이어도 파일명 끝에 널 문자가 오도록 복사함
    const FString Path(FilePath);
    int Width = 0;
    int Height = 0;

    // 좌측 상단부터 읽고 iPhone PNG의 BGRA 및 선곱 알파도 RGBA로 변환함
    stbi_set_flip_vertically_on_load_thread(0);
    stbi_set_unpremultiply_on_load_thread(1);
    stbi_convert_iphone_png_to_rgb_thread(1);

    // 자체 배열 할당 중 예외가 발생해도 stb의 픽셀 메모리를 해제함
    const std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> DecodedPixels(
        stbi_load(Path.c_str(), &Width, &Height, nullptr, STBI_rgb_alpha),
        &stbi_image_free);

    if (!DecodedPixels)
    {
        const char* Reason = stbi_failure_reason();
        throw std::runtime_error(std::format(
            "PNG load failed: {} ({})", Path, Reason ? Reason : "unknown error"));
    }

    if (Width <= 0 || Height <= 0)
    {
        throw std::length_error("Invalid PNG dimensions");
    }

    // 정수 곱셈 오버플로를 피하고 TArray::Num()의 int 범위를 검사함
    const uint64 RowPitch = static_cast<uint64>(Width) * STBI_rgb_alpha;
    const uint64 ByteCount = RowPitch * static_cast<uint64>(Height);
    if (ByteCount > static_cast<uint64>((std::numeric_limits<int>::max)()))
    {
        throw std::length_error("PNG pixel buffer exceeds TArray capacity");
    }

    FPNGImage Image;
    Image.Width = static_cast<uint32>(Width);
    Image.Height = static_cast<uint32>(Height);
    Image.RowPitch = static_cast<uint32>(RowPitch);
    Image.Pixels.SetNum(static_cast<size_t>(ByteCount));
    std::memcpy(Image.Pixels.GetData(), DecodedPixels.get(), static_cast<size_t>(ByteCount));

    return Image;
}
