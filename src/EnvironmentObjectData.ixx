module;
#include <string>
#include <glm/glm.hpp>

export module EnvironmentObjectData;

export enum class EnvironmentPlacement { World, Background };

export struct EnvironmentObjectData
{
	std::string id;
	// Relative to resources/textures.
	std::string texture;
	EnvironmentPlacement placement = EnvironmentPlacement::World;
	glm::vec3 position{ 0.0f };
	glm::vec2 size{ 1.0f };
	// Normalized anchor measured from the bottom-left of the quad.
	glm::vec2 anchor{ 0.5f, 0.0f };
	// Normalized top-left x/y and width/height in the background canvas.
	glm::vec4 image_rect{ 0.0f, 0.0f, 1.0f, 1.0f };
	float depth = 3.0f;
	glm::vec4 tint{ 1.0f };
};
