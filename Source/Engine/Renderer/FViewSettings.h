#pragma once
#include "Core/Core.h"

// 뷰 모드 enum과 UI 목록을 함께 생성하는 공통 목록
#define VIEW_MODE_LIST(X) \
    X(Lit)               \
    X(Unlit)             \
    X(Wireframe)

// 기본 렌더링 표현 방식
enum class EViewModeIndex : uint32
{
#define MAKE_VIEW_MODE_ENUM(Name) VMI_##Name,
    VIEW_MODE_LIST(MAKE_VIEW_MODE_ENUM)
#undef MAKE_VIEW_MODE_ENUM
};

// 뷰 모드 UI 항목
struct FViewModeEntry
{
    EViewModeIndex Mode;
    const char* Name;
};
inline constexpr FViewModeEntry ViewModeEntries[] =
{
#define MAKE_VIEW_MODE_ENTRY(Name) { EViewModeIndex::VMI_##Name, #Name },
    VIEW_MODE_LIST(MAKE_VIEW_MODE_ENTRY)
#undef MAKE_VIEW_MODE_ENTRY
};
#undef VIEW_MODE_LIST

// 독립적으로 켜고 끌 수 있는 표시 기능
enum class EEngineShowFlag : uint32 { 
    UUID = 1u << 0,
    Primitives = 1u << 1,
    Grid = 1u << 2,
    Bounds = 1u << 3,
};

// 표시 기능의 상태와 조회·변경 기능
struct FEngineShowFlags { public:
    bool IsEnabled(EEngineShowFlag Flag) const
    {
        return (Flags & static_cast<uint32>(Flag)) != 0;
    }

    void SetEnabled(EEngineShowFlag Flag, bool bEnabled)
    {
        const uint32 Mask = static_cast<uint32>(Flag);

        if (bEnabled)
            Flags |= Mask;
        else
            Flags &= ~Mask;
    }

private:
    // 기본적으로 모든 표시 기능을 활성화함
    uint32 Flags =
        static_cast<uint32>(EEngineShowFlag::UUID) |
        static_cast<uint32>(EEngineShowFlag::Primitives) |
        static_cast<uint32>(EEngineShowFlag::Grid) |
        static_cast<uint32>(EEngineShowFlag::Bounds); };

// 하나의 뷰에서 사용하는 렌더링 설정
struct FViewSettings
{
    EViewModeIndex ViewMode = EViewModeIndex::VMI_Unlit;
    FEngineShowFlags ShowFlags;
};