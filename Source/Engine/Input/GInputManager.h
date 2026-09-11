#pragma once
#include "Core/Core.h"

//각 오브젝트의 update에 대해서 InputManager::getInstance()->isKeyDown(EInputStatus)으로 조건 확인
//입력 추가시 EInputStatus 및 WndProc.h 업데이트

class GInputManager {
public:
	static GInputManager* GetInstance();

	enum EInputStatus {
		EI_W,
		EI_A,
		EI_S,
		EI_D,
		EI_Q,
		EI_E,
		EI_LMOUSE,
		EI_RMOUSE,
		EI_SPACE,
		KEY_COUNT
	};

	void SetKey(EInputStatus Key, bool Status);
	bool GetKey(EInputStatus Key);
	void KillFocus();
	void BeginRightDrag(int32 X, int32 Y);
	void BeginLeftDrag(int32 X, int32 Y);
	void UpdateRightDrag(int32 X, int32 Y);
	void UpdateLeftDrag(int32 X, int32 Y);
	void EndRightDrag();
	void EndLeftDrag();
	void ConsumeRightDragDelta(int32& X, int32& Y);
	void ConsumeLeftDragDelta(int32& X, int32& Y);
	void SetRightCursorX(const float& InCursor);
	void SetRightCursorY(const float& InCursor);
	void SetLeftCursorX(const float& InCursor);
	void SetLeftCursorY(const float& InCursor);
	float GetRightCursorX()const;
	float GetRightCursorY()const;
	float GetLeftCursorX()const;
	float GetLeftCursorY()const;
	bool ConsumeLeftClick();
	bool ConsumeSpacePress();
	void SetRightCursorPixelX(const int32& InPixel);
	void SetRightCursorPixelY(const int32& InPixel);
	void SetLeftCursorPixelX(const int32& InPixel);
	void SetLeftCursorPixelY(const int32& InPixel);

	int32 GetRightCursorPixelX()const;
	int32 GetRightCursorPixelY()const;
	int32 GetLeftCursorPixelX()const;
	int32 GetLeftCursorPixelY()const;

private:
	int32 RightDragDeltaX = 0;
	int32 RightDragDeltaY = 0;
	int32 LeftDragDeltaX = 0;
	int32 LeftDragDeltaY = 0;

	bool bKeyStatus[KEY_COUNT] = {};
	float RightCursorX;
	float RightCursorY;
	float LeftCursorX;
	float LeftCursorY;

	int32 RightCursorPixelX;
	int32 LeftCursorPixelX;
	int32 RightCursorPixelY;
	int32 LeftCursorPixelY;
	bool bLeftClickPending = false;
	bool bSpacePressPending = false;
	GInputManager() = default;
	~GInputManager() = default;
	GInputManager(const GInputManager&) = delete;
	GInputManager& operator=(const GInputManager&) = delete;
};
