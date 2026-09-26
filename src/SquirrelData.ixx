module;

#include <cmath>
#include <string>
#include <glm/glm.hpp>

export module SquirrelData;

import Polygon2d;

export struct SquirrelPoseData
{
	std::string texture;
	glm::vec2 size{ 0.65f, 0.8f };
	// World-space offset from the shared, bottom-center emergence point.
	glm::vec3 offset{ 0.0f };
};

export struct SquirrelData
{
	std::string id;
	glm::vec3 position{ 0.0f };
	SquirrelPoseData hidden_pose;
	SquirrelPoseData surprised_pose;
	Polygon2d discovery_region;
	glm::vec4 tint{ 1.0f };
	bool flip_horizontal = false;
	float bounce_height = 0.35f;
	float bounce_duration = 0.4f;
	float fade_duration = 0.45f;

	bool IsValid() const
	{
		auto finite = [](auto v) {
			for (int i = 0; i < v.length(); ++i)
				if (!std::isfinite(v[i])) return false;
			return true;
		};
		auto valid_pose = [&](SquirrelPoseData const & pose) {
			return !pose.texture.empty() && finite(pose.size) && finite(pose.offset)
				&& pose.size.x > 0 && pose.size.y > 0;
		};
		for (auto const & vertex : discovery_region.GetVertices())
			if (!finite(vertex)) return false;
		return !id.empty() && finite(position) && finite(tint)
			&& valid_pose(hidden_pose) && valid_pose(surprised_pose) && discovery_region.IsValid()
			&& std::isfinite(bounce_height) && bounce_height >= 0
			&& std::isfinite(bounce_duration) && bounce_duration > 0
			&& std::isfinite(fade_duration) && fade_duration > 0;
	}
};
