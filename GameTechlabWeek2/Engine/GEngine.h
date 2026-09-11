#pragma once
#include "Windows.h"
#include "FRenderer.h"

#include <chrono>

class FConsole;
class FEditor;

class GEngine
{
public:
	static GEngine* GetInstance();

	void Initialize(HWND InHwnd);

	void Tick();

	void Destroy();

	FConsole* GetConsole() const { return Console; };

	// 게임 시작 이후 얼마나 흘렀는지 반환합니다.
	float GetTime();

private:
	float LastTickTime = 0;
	float StartTime = 0;

	FRenderer Renderer;
	FConsole* Console = nullptr;
	FEditor* Editor = nullptr;

	// 싱글톤
	GEngine() = default;
	~GEngine() = default;
	GEngine(const GEngine&) = delete;
	GEngine& operator=(const GEngine&) = delete;
};
