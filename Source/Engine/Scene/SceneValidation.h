#pragma once

#include "Core/Core.h"
#include "Core/Container/FString.h"
#include "Core/Name/FName.h"

#include "Engine/Object/FClassType.h"
#include "Engine/Object/FClassRegistry.h"
#include "Engine/Component/UActorComponent.h"
#include "Engine/Resource/FMeshNames.h"

#include <charconv>
#include <limits>
#include <stdexcept>

// 씬에 기록된 타입의 복원 방식임
enum class ESceneTypeKind
{
    Unsupported,
    Component,
    LegacyStaticMesh,
    SkipRuntimeWidget
};

// 타입 검증과 실제 복원에서 공유하는 해석 결과임
struct FResolvedSceneType
{
    ESceneTypeKind Kind = ESceneTypeKind::Unsupported;
    FClassType* ClassType = nullptr;

    bool IsValid() const
    {
        if (Kind == ESceneTypeKind::SkipRuntimeWidget)
        {
            return true;
        }

        return
            (Kind == ESceneTypeKind::Component || Kind == ESceneTypeKind::LegacyStaticMesh) && ClassType != nullptr;
    }
};

inline FResolvedSceneType ResolveSceneType(const FName& TypeName)
{
    // 반복 사용하는 타입 이름을 최초 호출 시 한 번 생성함
    static const FName StaticMeshType("StaticMeshComponent");
    static const FName CameraType("CameraComponent");
    static const FName TextType("Text");
    static const FName SpotLightType("SpotLight");
    static const FName WidgetType("WidgetComponent");

    // 현재 등록 이름과 호환용 이름을 함께 유지함
    static const FName FlameType("Flame");
    static const FName FlipbookAlias("FlipbookComponent");

    if (TypeName.IsNone()) { return {}; }

    // UUID 위젯은 씬 복원 후 다시 생성하므로 직접 복원하지 않음
    if (TypeName == WidgetType)
    {
        return { ESceneTypeKind::SkipRuntimeWidget,nullptr };
    }

    // 대상 클래스의 등록 여부와 컴포넌트 상속 관계를 확인함
    const auto ResolveComponent = [](const FName& ClassName, ESceneTypeKind Kind) -> FResolvedSceneType
        {
            FClassType* ClassType = FClassRegistry::FindClassType(ClassName);

            if (ClassType == nullptr || !ClassType->IsA(UActorComponent::GetClass()))
            { return {}; }

            return { Kind, ClassType };
        };

    const FMeshNames& MeshNames = GetMeshNames();

    // 구형 메시 타입을 현재 StaticMeshComponent로 연결함
    const bool bLegacyStaticMesh =
        TypeName == MeshNames.Plane ||
        TypeName == MeshNames.Cube ||
        TypeName == MeshNames.Sphere ||
        TypeName == MeshNames.Triangle ||
        TypeName == MeshNames.Pepe ||
        TypeName == MeshNames.Octopus ||
        TypeName == MeshNames.ArrowRed ||
        TypeName == MeshNames.ArrowGreen ||
        TypeName == MeshNames.ArrowBlue;

    if (bLegacyStaticMesh)
    {
        return ResolveComponent(StaticMeshType, ESceneTypeKind::LegacyStaticMesh);
    }

    // 두 표기를 현재 등록된 Flame 클래스로 연결함
    if (TypeName == FlameType || TypeName == FlipbookAlias)
    {
        return ResolveComponent(FlameType, ESceneTypeKind::Component);
    }

    // 현재 씬에서 직접 저장하고 복원하는 타입만 허용함
    const bool bSupportedComponent =
        TypeName == StaticMeshType ||
        TypeName == CameraType ||
        TypeName == TextType ||
        TypeName == SpotLightType;

    if (bSupportedComponent)
    {
        return ResolveComponent(TypeName, ESceneTypeKind::Component);
    }

    return {};
}


inline uint32 ParseSceneUUID(FStringView Text, bool AllowExhaustedCounter = false)
{
    if (Text.empty()) throw std::runtime_error("Empty UUID");
    uint32 Value{};
    const auto [End, Error] = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
    if (Error != std::errc{} || End != Text.data() + Text.size() ||
        (!AllowExhaustedCounter && Value == (std::numeric_limits<uint32>::max)()) ||
        std::to_string(Value) != Text)
        throw std::runtime_error("Invalid UUID");
    return Value;
}
