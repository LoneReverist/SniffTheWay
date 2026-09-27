module;

#include <cmath>
#include <string>
#include <glm/glm.hpp>

export module SquirrelData;

import Polygon2d;

export struct SquirrelData
{
	std::string id;
	glm::vec3 position{ 0.0f };
	Polygon2d discovery_region;
	bool flip_horizontal = false;

	bool IsValid() const
	{
		for (int i = 0; i < position.length(); ++i)
			if (!std::isfinite(position[i])) return false;
		for (auto const & vertex : discovery_region.GetVertices())
			for (int i = 0; i < vertex.length(); ++i)
				if (!std::isfinite(vertex[i])) return false;
		return !id.empty() && discovery_region.IsValid();
	}
};
