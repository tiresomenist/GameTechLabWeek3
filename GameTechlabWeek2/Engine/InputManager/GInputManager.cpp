#include "GInputManager.h"

GInputManager* GInputManager::GetInstance()
{
    static GInputManager Instance{};
    return &Instance;
}

void GInputManager::SetKey(EInputStatus Key, bool Status)
{
	if (Key == EI_LMOUSE && Status && !bKeyStatus[Key])
	{
		bLeftClickPending = true;
	}else if (Key == EI_SPACE && Status && !bKeyStatus[Key])
	{
		bSpacePressPending = true;
	}
	bKeyStatus[Key] = Status;
}

bool GInputManager::GetKey(EInputStatus Key)
{
	return bKeyStatus[Key];
}

void GInputManager::KillFocus()
{
	for (auto& i : bKeyStatus)
	{
		i = false;
	}
	EndRightDrag();
	EndLeftDrag();
	bLeftClickPending = false;
	bSpacePressPending = false;
}

void GInputManager::BeginRightDrag(int32 X, int32 Y)
{
	RightCursorPixelX = X;
	RightCursorPixelY = Y;
	RightDragDeltaX = RightDragDeltaY = 0;
	SetKey(EI_RMOUSE, true);
}
void GInputManager::BeginLeftDrag(int32 X, int32 Y)
{
	LeftCursorPixelX = X;
	LeftCursorPixelY = Y;
	LeftDragDeltaX = LeftDragDeltaY = 0;
	SetKey(EI_LMOUSE, true);
}
void GInputManager::UpdateRightDrag(int32 X, int32 Y)
{
	if (!GetKey(EI_RMOUSE)) return;
	RightDragDeltaX += X - RightCursorPixelX;
	RightDragDeltaY += Y - RightCursorPixelY;
	RightCursorPixelX = X;
	RightCursorPixelY = Y;
}

void GInputManager::UpdateLeftDrag(int32 X, int32 Y)
{
	if (!GetKey(EI_LMOUSE)) return;
	LeftDragDeltaX += X - LeftCursorPixelX;
	LeftDragDeltaY += Y - LeftCursorPixelY;
	LeftCursorPixelX = X;
	LeftCursorPixelY = Y;
}

void GInputManager::EndRightDrag()
{
	SetKey(EI_RMOUSE, false);
	RightDragDeltaX = RightDragDeltaY = 0;
}

void GInputManager::EndLeftDrag()
{
	SetKey(EI_LMOUSE, false);
	LeftDragDeltaX = LeftDragDeltaY = 0;
}


void GInputManager::ConsumeRightDragDelta(int32& X, int32& Y)
{
	X = RightDragDeltaX;
	Y = RightDragDeltaY;
	RightDragDeltaX = RightDragDeltaY = 0;
}

void GInputManager::ConsumeLeftDragDelta(int32& X, int32& Y)
{
	X = LeftDragDeltaX;
	Y = LeftDragDeltaY;
	LeftDragDeltaX = LeftDragDeltaY = 0;
}

void GInputManager::SetRightCursorX(const float& InCursor)
{
	RightCursorX = InCursor;
}

void GInputManager::SetRightCursorY(const float& InCursor)
{
	RightCursorY = InCursor;
}

void GInputManager::SetLeftCursorX(const float& InCursor)
{
	LeftCursorX = InCursor;
}

void GInputManager::SetLeftCursorY(const float& InCursor)
{
	LeftCursorY = InCursor;
}

float GInputManager::GetRightCursorX() const
{
	return RightCursorX;
}

float GInputManager::GetRightCursorY() const
{
	return RightCursorY;
}

float GInputManager::GetLeftCursorX() const
{
	return LeftCursorX;
}

float GInputManager::GetLeftCursorY() const
{
	return LeftCursorY;
}
bool GInputManager::ConsumeLeftClick()
{
	const bool bClicked = bLeftClickPending;
	bLeftClickPending = false;
	return bClicked;
}
bool GInputManager::ConsumeSpacePress()
{
	const bool bPressed = bSpacePressPending;
	bSpacePressPending = false;
	return bPressed;
}
void GInputManager::SetRightCursorPixelX(const int32& InPixel)
{
	RightCursorPixelX = InPixel;
}
void GInputManager::SetRightCursorPixelY(const int32& InPixel)
{
	RightCursorPixelY = InPixel;
}
void GInputManager::SetLeftCursorPixelX(const int32& InPixel)
{
	LeftCursorPixelX = InPixel;
}
void GInputManager::SetLeftCursorPixelY(const int32& InPixel)
{
	LeftCursorPixelY = InPixel;
}
int32 GInputManager::GetRightCursorPixelX() const
{
	return RightCursorPixelX;
}
int32 GInputManager::GetRightCursorPixelY() const
{
	return RightCursorPixelY;
}

int32 GInputManager::GetLeftCursorPixelX() const
{
	return LeftCursorPixelX;
}

int32 GInputManager::GetLeftCursorPixelY() const
{
	return LeftCursorPixelY;
}
