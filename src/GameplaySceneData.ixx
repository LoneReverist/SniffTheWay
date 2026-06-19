// GameplaySceneData.ixx

module;

#include <string>
#include <vector>

#include <glm/glm.hpp>

export module GameplaySceneData;

import Polygon2d;
import SniffTheWayConstants;
import StoryData;

export struct GameplayAdjacentScene
{
	SniffTheWay::SceneId scene_id = SniffTheWay::SceneId::Exit;
	Polygon2d collider;
	glm::vec2 dog_spawn_pos{ 0.0f };
	glm::vec2 baby_spawn_pos{ 0.0f };
};

export struct ScentTrailData
{
	std::vector<glm::vec2> points;
	float width = 0.18f;
	float visible_distance = 4.0f;
};

export struct GameplaySceneData
{
	std::string bg_image_filename;
	SniffTheWay::SceneState initial_state = SniffTheWay::SceneState::Gameplay;
	Polygon2d bounds;
	glm::vec2 dog_spawn_pos{ 0.0f };
	glm::vec2 baby_spawn_pos{ 0.0f };
	ScentTrailData scent_trail;
	std::vector<StoryText> story_texts;
	std::vector<GameplayAdjacentScene> adjacent_scenes;
};
