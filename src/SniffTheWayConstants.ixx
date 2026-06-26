// SniffTheWayConstants.ixx

module;

#include <cstdint>
#include <optional>
#include <string_view>

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
	constexpr float StoryLargeFontSize = 80.0f;
	constexpr float StoryMediumFontSize = 56.0f;
	constexpr float StorySmallFontSize = 36.0f;

	constexpr glm::vec4 StoryTextColor{ 0.93f, 0.89f, 0.70f, 1.0f };

	enum class RenderLayer : std::uint8_t
	{
		Background = 0,
		Scene3d = 1,
		UIShadow = 2,
		UIForeground = 3,
	};

	enum class SceneState : std::uint8_t
	{
		Story,
		Gameplay,
		Editing,
		Paused,
	};

	enum class SceneId : std::uint8_t
	{
		Exit, // flag to close the application
		Picnic,
		ForestPath,
		ForestPath2,
		RightTurn,
		ForestLake,
		Playground,
		ForestHorizontal,
		ThickerForestTransition,
		BeforeCreek,
		Creek,
		AfterCreek,
		GoldenIntersection,
		FallenTree,
		GoldenPath,
		GoldenHour,
		DeepForest,
		DarkForest,
		DarkForest2,
		Night,
		ForestIntersection,
		Home,
	};

	constexpr bool IsStoryScene(SceneId scene_id)
	{
		switch (scene_id)
		{
		case SceneId::Picnic:
		case SceneId::Creek:
		case SceneId::Night:
		case SceneId::Home:
			return true;
		default:
			return false;
		}
	}

	constexpr bool IsGameplayScene(SceneId scene_id)
	{
		switch (scene_id)
		{
		case SceneId::ForestPath:
		case SceneId::ForestPath2:
		case SceneId::RightTurn:
		case SceneId::ForestLake:
		case SceneId::Playground:
		case SceneId::ForestHorizontal:
		case SceneId::ThickerForestTransition:
		case SceneId::BeforeCreek:
		case SceneId::AfterCreek:
		case SceneId::GoldenIntersection:
		case SceneId::FallenTree:
		case SceneId::GoldenPath:
		case SceneId::GoldenHour:
		case SceneId::DeepForest:
		case SceneId::DarkForest:
		case SceneId::DarkForest2:
		case SceneId::ForestIntersection:
			return true;
		default:
			return false;
		}
	}

	constexpr std::string_view ToString(SceneId scene_id)
	{
		switch (scene_id)
		{
		case SceneId::Exit:
			return "exit";
		case SceneId::Picnic:
			return "picnic";
		case SceneId::ForestPath:
			return "forest_path";
		case SceneId::ForestPath2:
			return "forest_path2";
		case SceneId::RightTurn:
			return "right_turn";
		case SceneId::ForestLake:
			return "forest_lake";
		case SceneId::Playground:
			return "playground";
		case SceneId::ForestHorizontal:
			return "forest_horizontal";
		case SceneId::ThickerForestTransition:
			return "thicker_forest_transition";
		case SceneId::BeforeCreek:
			return "before_creek";
		case SceneId::Creek:
			return "creek";
		case SceneId::AfterCreek:
			return "after_creek";
		case SceneId::GoldenIntersection:
			return "golden_intersection";
		case SceneId::FallenTree:
			return "fallen_tree";
		case SceneId::GoldenPath:
			return "golden_path";
		case SceneId::GoldenHour:
			return "golden_hour";
		case SceneId::DeepForest:
			return "deep_forest";
		case SceneId::DarkForest:
			return "dark_forest";
		case SceneId::DarkForest2:
			return "dark_forest2";
		case SceneId::Night:
			return "night";
		case SceneId::ForestIntersection:
			return "forest_intersection";
		case SceneId::Home:
			return "home";
		}

		return "exit";
	}

	constexpr std::optional<SceneId> SceneIdFromString(std::string_view scene_id)
	{
		if (scene_id == "exit")
			return SceneId::Exit;
		if (scene_id == "picnic")
			return SceneId::Picnic;
		if (scene_id == "forest_path")
			return SceneId::ForestPath;
		if (scene_id == "forest_path2")
			return SceneId::ForestPath2;
		if (scene_id == "right_turn")
			return SceneId::RightTurn;
		if (scene_id == "forest_lake")
			return SceneId::ForestLake;
		if (scene_id == "playground")
			return SceneId::Playground;
		if (scene_id == "forest_horizontal")
			return SceneId::ForestHorizontal;
		if (scene_id == "thicker_forest_transition")
			return SceneId::ThickerForestTransition;
		if (scene_id == "before_creek")
			return SceneId::BeforeCreek;
		if (scene_id == "creek")
			return SceneId::Creek;
		if (scene_id == "after_creek")
			return SceneId::AfterCreek;
		if (scene_id == "golden_intersection")
			return SceneId::GoldenIntersection;
		if (scene_id == "fallen_tree")
			return SceneId::FallenTree;
		if (scene_id == "golden_path")
			return SceneId::GoldenPath;
		if (scene_id == "golden_hour")
			return SceneId::GoldenHour;
		if (scene_id == "deep_forest")
			return SceneId::DeepForest;
		if (scene_id == "dark_forest")
			return SceneId::DarkForest;
		if (scene_id == "dark_forest2")
			return SceneId::DarkForest2;
		if (scene_id == "night")
			return SceneId::Night;
		if (scene_id == "forest_intersection")
			return SceneId::ForestIntersection;
		if (scene_id == "home")
			return SceneId::Home;

		return std::nullopt;
	}
}
