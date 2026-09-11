// stb 단일 헤더 라이브러리의 구현을 생성하는 유일한 번역 단위.
// 다른 파일에서는 define 없이 #include "stb/stb_truetype.h"만 한다.
// ImGui도 imgui_draw.cpp 안에 자체 구현을 갖고 있지만 STBTT_STATIC이라 서로 충돌하지 않는다.
// premake5.lua에서 이 파일은 PCH를 사용하지 않도록 설정되어 있다.

// stb_truetype의 pack API가 제대로 된 패커를 쓰도록 rect_pack을 먼저 include한다.
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
