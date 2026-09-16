#pragma once

#include "Core/Container/TArray.h"
#include "Core/Container/FString.h"
#include "Engine/Resource/FMeshNames.h"
#include "ImGui/imgui.h"

namespace MeshSelection
{
    struct FEntry
    {
        FName Key;
        const char* Label;
    };
    inline const TArray<FEntry>& GetEntries()
    {
        const FMeshNames& Names = GetMeshNames();
        static const TArray<FEntry> Entries
        {
            { Names.Sphere,   "Sphere" },
            { Names.Cube,     "Cube" },
            { Names.Plane,    "Plane" },
            { Names.Triangle,"Triangle" },
            { Names.Pepe,     "Pepe" },
            { Names.Octopus,  "Octopus" }
        };

        return Entries;
    }

    inline bool DrawCombo(const char* Label, FName& SelectedKey, ImGuiComboFlags Flags = 0)
    {
        const TArray<FEntry>& Entries = GetEntries();

        const char* Preview = nullptr;
        // 등록된 항목은 재사용
        for (const FEntry& Entry : Entries)
        {
            if (Entry.Key == SelectedKey)
            {
                Preview = Entry.Label;
                break;
            }
        }
        // 목록에 없는 메시도 현재 이름을 표시함
        FString FallbackLabel;

        if (!Preview)
        {
            FallbackLabel = SelectedKey.ToString();
            Preview = FallbackLabel.c_str();
        }

        bool bChanged = false;

        if (ImGui::BeginCombo(Label, Preview, Flags))
        {
            for (const FEntry& Entry : Entries)
            {
                const bool bSelected = SelectedKey == Entry.Key;

                if (ImGui::Selectable(Entry.Label, bSelected))
                {
                    if (!bSelected)
                    {
                        SelectedKey = Entry.Key;
                        bChanged = true;
                    }
                }

                if (bSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        return bChanged;
    }
}