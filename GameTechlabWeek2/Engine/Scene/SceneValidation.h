#pragma once
#include "Container/FString.h"
#include "Engine/Core.h"
#include <charconv>
#include <limits>
#include <stdexcept>

inline bool IsAllowedSceneType(FStringView Name)
{
    return Name == "Cube" || Name == "Sphere" || Name == "Plane" ||
        Name == "Triangle" || Name == "Pepe" || Name == "Octopus" ||
        Name == "CameraComponent" || Name == "ArrowRed" ||
        Name == "ArrowGreen" || Name == "ArrowBlue";
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
