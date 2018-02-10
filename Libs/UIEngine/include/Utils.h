#pragma once

#include "glm/glm.hpp"

struct Rect
{
	glm::vec2 pos;
	glm::vec2 extent;

	Rect(float x = 0, float y = 0, float w = 0, float h = 0)
		: pos(x, y)
		, extent(w, h)
	{}

	Rect(const glm::vec2& _pos, const glm::vec2& _extent)
		: pos(_pos)
		, extent(_extent)
	{}

	bool isPointInside(const glm::vec2& point) const
	{
		return !((point.x < pos.x || point.x > pos.x + extent.x)
			|| (point.y < pos.y || point.y > pos.y + extent.y));
	}

	glm::vec4 toVec4() const
	{
		return glm::vec4(pos, extent);
	}

	void addOffset(const glm::vec2& offset)
	{
		pos += offset;
	}

	void append(const Rect& other)
	{
		glm::vec2 bottom01 = pos + extent;
		glm::vec2 bottom02 = other.pos + other.extent;

		pos.x = glm::min(other.pos.x, pos.x);
		pos.y = glm::min(other.pos.y, pos.y);
		extent.x = glm::max(bottom01.x, bottom02.x) - pos.x;
		extent.y = glm::max(bottom01.y, bottom02.y) - pos.y;
	}
};
