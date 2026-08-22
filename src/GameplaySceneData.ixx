// GameplaySceneData.ixx

module;

#include <string>
#include <vector>

#include <glm/glm.hpp>

export module GameplaySceneData;

import GameplayMessageData;
import CharacterFacing;
import Polygon2d;
import SniffTheWayConstants;

export struct GameplayCharacterArrival
{
	glm::vec2 position{ 0.0f };
	CharacterCameraFacing camera_facing = CharacterCameraFacing::TowardsCamera;
	CharacterHorizontalFacing horizontal_facing = CharacterHorizontalFacing::Right;
};

export struct GameplaySceneLink
{
	SniffTheWay::SceneId target_scene_id = SniffTheWay::SceneId::Exit;
	Polygon2d trigger;
	GameplayCharacterArrival dog_arrival;
	GameplayCharacterArrival baby_arrival;
};

export struct ScentTrailData
{
	std::vector<glm::vec2> points;
};

export struct GameplayCameraData
{
	glm::vec3 position{ 0.0f, -6.0f, 1.0f };
	glm::vec3 direction{ 0.0f, 0.9847835302352905f, -0.17378532886505127f };
	float fov_degrees = 45.0f;
};

export enum class GameplayMessageRepeat
{
	None,
};

export struct GameplayMessageTriggerData
{
	std::string id;
	Polygon2d trigger;
	GameplayMessageRepeat repeat = GameplayMessageRepeat::None;
	GameplayMessage message;
};

export struct GameplaySceneData
{
	std::string bg_image_filename;
	glm::vec4 tint{ 1.0f };
	GameplayCameraData camera;
	Polygon2d bounds;
	std::vector<ScentTrailData> scent_trails;
	std::vector<GameplayMessageTriggerData> message_triggers;
	std::vector<GameplaySceneLink> scene_links;
};
