// SniffTheWayConstants.ixx

module;

#include <cstdint>

#include <glm/vec4.hpp>

export module SniffTheWayConstants;

export namespace SniffTheWay
{
	constexpr char const * FullTitle = "Sniff the Way - A Tail to Guide You Home";
	constexpr char const * ShortTitle = "Sniff the Way";

	constexpr float LabelFontSize = 18.0f;
	constexpr float TitleFontSize = 32.0f;
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
		Creek,
		DarkForest,
		Night,
		ForestIntersection,
		Home,
	};
}
