// SniffTheWayConstants.ixx

module;

#include <cstdint>

#include <glm/vec4.hpp>

export module SniffTheWayConstants;

export namespace SniffTheWay
{
	constexpr char const * FullTitle = "Sniff the Way - A Tail to Guide You Home";
	constexpr char const * ShortTitle = "Sniff the Way";

	// top left corner of the viewport is (0, 0), bottom right is (UIWidth, UIHeight)
	constexpr float UIWidth = 1920.0f;
	constexpr float UIHeight = 1080.0f;

	// This is the font size at 1920x1080 resolution.
	// It will be scaled proportionally to the height of the viewport.
	constexpr float LabelFontSize = 36.0f;
	constexpr float TitleFontSize = 64.0f;

	constexpr glm::vec4 StoryTextColor{ 0.96f, 0.90f, 0.78f, 1.0f };

	enum class SceneState : std::uint8_t
	{
		Story,
		Gameplay,
		Paused,
	};

	enum class SceneId : std::uint8_t
	{
		Exit, // flag to close the application
		Picnic,
		ForestPath,
		Playground,
		Playground2,
		Creek,
		DarkForest,
		Night,
		ForestIntersection,
		Home,
	};
}
