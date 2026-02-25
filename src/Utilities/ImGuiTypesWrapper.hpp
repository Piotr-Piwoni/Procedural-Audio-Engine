#pragma once
#include "IMGUI/imgui.h"

namespace MT::ImGuiTypes
{
struct Vec2 : ImVec2
{
	Vec2() :
		ImVec2{0.f, 0.f} {}

	Vec2(const float _x, const float _y):
		ImVec2{_x, _y} {}

	Vec2(const ImVec2& v) :
		ImVec2{v.x, v.y} {}


	Vec2 operator+(const ImVec2& val) const
	{
		return {x + val.x, y + val.y};
	}

	Vec2 operator-(const ImVec2& val) const
	{
		return {x - val.x, y - val.y};
	}

	Vec2& operator=(const ImVec2& val)
	{
		x = val.x;
		y = val.y;
		return *this;
	}
};
}
