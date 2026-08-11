// GameplayMessageData.ixx

module;

#include <string>
#include <optional>

#include <glm/glm.hpp>

export module GameplayMessageData;

import SniffTheWayConstants;

export struct GameplayMessage
{
	std::string text;
	glm::vec2 position{
		SniffTheWay::UIWidth * 0.5f,
		SniffTheWay::UIHeight * 0.85f
	};
	float font_size = SniffTheWay::StoryMediumFontSize;
	std::optional<float> hold_duration;
	float fade_in_duration = 0.25f;
	float fade_out_duration = 0.75f;
};
