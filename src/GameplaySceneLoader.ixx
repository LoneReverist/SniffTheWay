// GameplaySceneLoader.ixx

module;

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glog/logging.h>
#include <nlohmann/json.hpp>

export module GameplaySceneLoader;

import GameplaySceneData;
import CharacterFacing;
import GameplayMessageData;
import Polygon2d;
import SniffTheWayConstants;
import SceneAudioJson;

using json = nlohmann::ordered_json;

glm::vec2 parse_gameplay_vec2(json const & j, glm::vec2 fallback)
{
	if (!j.is_array() || j.size() != 2)
		return fallback;

	return glm::vec2{ j[0].get<float>(), j[1].get<float>() };
}

glm::vec3 parse_gameplay_vec3(json const & j, glm::vec3 fallback)
{
	if (!j.is_array() || j.size() != 3)
		return fallback;

	return glm::vec3{ j[0].get<float>(), j[1].get<float>(), j[2].get<float>() };
}

glm::vec4 parse_gameplay_tint(json const & j, glm::vec4 fallback)
{
	if (!j.is_array() || j.size() != 4)
		return fallback;

	glm::vec4 tint;
	for (std::size_t i = 0; i < 4; ++i)
	{
		if (!j[i].is_number())
			return fallback;

		float const component = j[i].get<float>();
		if (!std::isfinite(component) || component < 0.0f || component > 1.0f)
			return fallback;
		tint[i] = component;
	}

	return tint;
}

Polygon2d parse_gameplay_polygon(json const & j)
{
	std::vector<glm::vec2> vertices;
	if (!j.is_array())
		return Polygon2d{};

	vertices.reserve(j.size());
	for (json const & vertex_json : j)
		vertices.push_back(parse_gameplay_vec2(vertex_json, glm::vec2{ 0.0f }));

	return Polygon2d{ std::move(vertices) };
}

GameplayMessage parse_gameplay_message(json const & j)
{
	GameplayMessage message;
	message.text = j.value("text", "");
	if (j.contains("position"))
		message.position = parse_gameplay_vec2(j["position"], message.position);
	message.font_size = j.value("font_size", message.font_size);
	if (j.contains("hold_duration"))
		message.hold_duration = j["hold_duration"].get<float>();
	message.fade_in_duration = j.value("fade_in_duration", message.fade_in_duration);
	message.fade_out_duration = j.value("fade_out_duration", message.fade_out_duration);
	return message;
}

GameplayMessageRepeat parse_gameplay_message_repeat(std::string const & repeat)
{
	if (repeat != "none")
		LOG(WARNING) << "GameplaySceneLoader: Unknown message repeat policy '" << repeat << "'. Using none.";

	return GameplayMessageRepeat::None;
}

GameplayMessageTriggerData parse_gameplay_message_trigger(json const & j)
{
	GameplayMessageTriggerData message_trigger;
	message_trigger.requires_trigger = j.value("requires_trigger", "");
	message_trigger.requires_not_trigger = j.value("requires_not_trigger", "");
	message_trigger.id = j.value("id", "");
	if (j.contains("trigger"))
		message_trigger.trigger = parse_gameplay_polygon(j["trigger"]);
	message_trigger.repeat = parse_gameplay_message_repeat(j.value("repeat", "none"));
	message_trigger.message = parse_gameplay_message(j.value("message", json::object()));
	return message_trigger;
}

ScentTrailData parse_scent_trail(json const & j)
{
	ScentTrailData scent_trail;
	if (!j.is_object())
		return scent_trail;

	for (json const & point_json : j.value("points", json::array()))
		scent_trail.points.push_back(parse_gameplay_vec2(point_json, glm::vec2{ 0.0f }));

	return scent_trail;
}

GameplayCameraData parse_gameplay_camera(json const & j, GameplayCameraData fallback)
{
	if (!j.is_object())
		return fallback;

	if (j.contains("position"))
		fallback.position = parse_gameplay_vec3(j["position"], fallback.position);
	if (j.contains("direction"))
	{
		glm::vec3 const direction = parse_gameplay_vec3(j["direction"], fallback.direction);
		if (glm::length(direction) > 1e-6f)
			fallback.direction = glm::normalize(direction);
	}
	fallback.fov_degrees = j.value("fov_degrees", fallback.fov_degrees);

	return fallback;
}

CharacterCameraFacing parse_character_camera_facing(std::string const & value)
{
	if (value == "away")
		return CharacterCameraFacing::AwayFromCamera;
	if (value != "towards")
		LOG(WARNING) << "GameplaySceneLoader: Unknown character camera facing '" << value << "'. Using towards.";
	return CharacterCameraFacing::TowardsCamera;
}

CharacterHorizontalFacing parse_character_horizontal_facing(std::string const & value)
{
	if (value == "left")
		return CharacterHorizontalFacing::Left;
	if (value != "right")
		LOG(WARNING) << "GameplaySceneLoader: Unknown character horizontal facing '" << value << "'. Using right.";
	return CharacterHorizontalFacing::Right;
}

GameplayCharacterArrival parse_gameplay_character_arrival(json const & j, GameplayCharacterArrival fallback)
{
	if (!j.is_object())
		return fallback;

	if (j.contains("position"))
		fallback.position = parse_gameplay_vec2(j["position"], fallback.position);
	fallback.camera_facing = parse_character_camera_facing(j.value("camera_facing", std::string{ "towards" }));
	fallback.horizontal_facing = parse_character_horizontal_facing(j.value("horizontal_facing", std::string{ "right" }));
	return fallback;
}

GameplaySceneLink parse_gameplay_scene_link(json const & j)
{
	GameplaySceneLink scene_link;
	scene_link.requires_trigger = j.value("requires_trigger", "");
	scene_link.requires_not_trigger = j.value("requires_not_trigger", "");

	const std::string target_scene_id = j.value("target_scene_id", std::string{ "exit" });
	if (std::optional<SniffTheWay::SceneId> parsed_target_scene_id = SniffTheWay::SceneIdFromString(target_scene_id))
	{
		scene_link.target_scene_id = *parsed_target_scene_id;
	}
	else
	{
		LOG(WARNING) << "GameplaySceneLoader: Unknown scene link target id '" << target_scene_id << "'. Using exit.";
	}

	if (j.contains("trigger"))
		scene_link.trigger = parse_gameplay_polygon(j["trigger"]);

	json const arrival_points_json = j.value("arrival_points", json::object());
	if (arrival_points_json.contains("dog"))
		scene_link.dog_arrival = parse_gameplay_character_arrival(arrival_points_json["dog"], scene_link.dog_arrival);

	if (arrival_points_json.contains("baby"))
		scene_link.baby_arrival = parse_gameplay_character_arrival(arrival_points_json["baby"], scene_link.baby_arrival);

	return scene_link;
}

json serialize_gameplay_vec2(glm::vec2 value)
{
	return json::array({ value.x, value.y });
}

json serialize_gameplay_vec3(glm::vec3 value)
{
	return json::array({ value.x, value.y, value.z });
}

json serialize_gameplay_vec4(glm::vec4 value)
{
	return json::array({ value.x, value.y, value.z, value.w });
}

json serialize_gameplay_polygon(Polygon2d const & polygon)
{
	json vertices = json::array();
	for (glm::vec2 const & vertex : polygon.GetVertices())
		vertices.push_back(serialize_gameplay_vec2(vertex));

	return vertices;
}

std::string serialize_gameplay_message_repeat(GameplayMessageRepeat repeat)
{
	switch (repeat)
	{
	case GameplayMessageRepeat::None:
		return "none";
	}

	return "none";
}

json serialize_gameplay_message(GameplayMessage const & message)
{
	json result{
		{ "text", message.text },
		{ "position", serialize_gameplay_vec2(message.position) },
		{ "font_size", message.font_size },
		{ "fade_in_duration", message.fade_in_duration },
		{ "fade_out_duration", message.fade_out_duration }
	};
	if (message.hold_duration.has_value())
		result["hold_duration"] = *message.hold_duration;
	return result;
}

json serialize_gameplay_message_trigger(GameplayMessageTriggerData const & message_trigger)
{
	json result{
		{ "id", message_trigger.id },
		{ "trigger", serialize_gameplay_polygon(message_trigger.trigger) },
		{ "repeat", serialize_gameplay_message_repeat(message_trigger.repeat) },
		{ "message", serialize_gameplay_message(message_trigger.message) }
	};
	if (!message_trigger.requires_trigger.empty())
		result["requires_trigger"] = message_trigger.requires_trigger;
	if (!message_trigger.requires_not_trigger.empty())
		result["requires_not_trigger"] = message_trigger.requires_not_trigger;
	return result;
}

json serialize_scent_trail(ScentTrailData const & scent_trail)
{
	json points = json::array();
	for (glm::vec2 const & point : scent_trail.points)
		points.push_back(serialize_gameplay_vec2(point));

	return json{
		{ "points", std::move(points) }
	};
}

json serialize_gameplay_camera(GameplayCameraData const & camera)
{
	return json{
		{ "position", serialize_gameplay_vec3(camera.position) },
		{ "direction", serialize_gameplay_vec3(camera.direction) },
		{ "fov_degrees", camera.fov_degrees }
	};
}

std::string serialize_character_camera_facing(CharacterCameraFacing facing)
{
	return facing == CharacterCameraFacing::AwayFromCamera ? "away" : "towards";
}

std::string serialize_character_horizontal_facing(CharacterHorizontalFacing facing)
{
	return facing == CharacterHorizontalFacing::Left ? "left" : "right";
}

json serialize_gameplay_character_arrival(GameplayCharacterArrival const & arrival)
{
	return json{
		{ "position", serialize_gameplay_vec2(arrival.position) },
		{ "camera_facing", serialize_character_camera_facing(arrival.camera_facing) },
		{ "horizontal_facing", serialize_character_horizontal_facing(arrival.horizontal_facing) }
	};
}

json serialize_gameplay_scene_link(GameplaySceneLink const & scene_link)
{
	json result{
		{ "target_scene_id", std::string{ SniffTheWay::ToString(scene_link.target_scene_id) } },
		{ "arrival_points", json{
			{ "dog", serialize_gameplay_character_arrival(scene_link.dog_arrival) },
			{ "baby", serialize_gameplay_character_arrival(scene_link.baby_arrival) }
		} },
		{ "trigger", serialize_gameplay_polygon(scene_link.trigger) }
	};
	if (!scene_link.requires_trigger.empty())
		result["requires_trigger"] = scene_link.requires_trigger;
	if (!scene_link.requires_not_trigger.empty())
		result["requires_not_trigger"] = scene_link.requires_not_trigger;
	return result;
}

json serialize_gameplay_scene_data(GameplaySceneData const & scene_data, json existing_root = json::object())
{
	json root = json::object();
	if (existing_root.contains("id"))
		root["id"] = existing_root["id"];

	root["audio"] = SerializeSceneAudio(scene_data.audio);
	if (!scene_data.on_enter_triggers.empty())
		root["on_enter_triggers"] = scene_data.on_enter_triggers;
	root["background"] = scene_data.bg_image_filename;
	root["environment_objects"] = json::array();
	for (auto const & object : scene_data.environment_objects)
	{
		json entry = { { "id", object.id }, { "texture", object.texture },
			{ "placement", object.placement == EnvironmentPlacement::World ? "world" : "background" },
			{ "tint", serialize_gameplay_vec4(object.tint) } };
		if (object.placement == EnvironmentPlacement::World)
		{
			entry["position"] = { object.position.x, object.position.y, object.position.z };
			entry["size"] = { object.size.x, object.size.y };
			entry["anchor"] = { object.anchor.x, object.anchor.y };
		}
		else
		{
			entry["image_rect"] = serialize_gameplay_vec4(object.image_rect);
			entry["depth"] = object.depth;
		}
		root["environment_objects"].push_back(std::move(entry));
	}
	root["tint"] = serialize_gameplay_vec4(scene_data.tint);
	root["camera"] = serialize_gameplay_camera(scene_data.camera);
	root["bounds"] = serialize_gameplay_polygon(scene_data.bounds);
	json scent_trails = json::array();
	for (ScentTrailData const & scent_trail : scene_data.scent_trails)
		scent_trails.push_back(serialize_scent_trail(scent_trail));
	root["scent_trails"] = std::move(scent_trails);

	json scene_links = json::array();
	for (GameplaySceneLink const & scene_link : scene_data.scene_links)
		scene_links.push_back(serialize_gameplay_scene_link(scene_link));
	root["scene_links"] = std::move(scene_links);

	json message_triggers = json::array();
	for (GameplayMessageTriggerData const & message_trigger : scene_data.message_triggers)
		message_triggers.push_back(serialize_gameplay_message_trigger(message_trigger));
	root["message_triggers"] = std::move(message_triggers);

	for (auto const & [key, value] : existing_root.items())
	{
		if (key != "audio" &&
			key != "id" &&
			key != "on_enter_triggers" &&
			key != "background" &&
			key != "environment_objects" &&
			key != "tint" &&
			key != "initial_state" &&
			key != "camera" &&
			key != "bounds" &&
			key != "default_spawn" &&
			key != "scent_trails" &&
			key != "story_texts" &&
			key != "message_triggers" &&
			key != "scene_links")
		{
			root[key] = value;
		}
	}

	return root;
}

export namespace GameplaySceneLoader
{
	GameplaySceneData LoadSceneData(std::filesystem::path const & filepath)
	{
		GameplaySceneData scene_data;

		std::ifstream file(filepath);
		if (!file)
		{
			LOG(ERROR) << "GameplaySceneLoader: Failed to open gameplay scene file: " << filepath;
			return scene_data;
		}

		json root;
		try
		{
			file >> root;

			scene_data.audio = ParseSceneAudio(root, filepath);
			scene_data.on_enter_triggers = root.value("on_enter_triggers", std::vector<std::string>{});

			scene_data.bg_image_filename = root.value("background", "");
			if (root.contains("environment_objects") && root["environment_objects"].is_array())
			{
				for (auto const & entry : root["environment_objects"])
				{
					try
					{
						EnvironmentObjectData object;
						object.id = entry.value("id", "");
						object.texture = entry.at("texture").get<std::string>();
						auto const placement = entry.value("placement", "world");
						if (placement != "world" && placement != "background")
						{
							LOG(WARNING) << "Unknown environment placement: " << placement;
							continue;
						}
						object.placement = placement == "world" ? EnvironmentPlacement::World : EnvironmentPlacement::Background;
						if (entry.contains("position")) object.position = parse_gameplay_vec3(entry["position"], object.position);
						if (entry.contains("size")) object.size = parse_gameplay_vec2(entry["size"], object.size);
						if (entry.contains("anchor")) object.anchor = parse_gameplay_vec2(entry["anchor"], object.anchor);
						if (entry.contains("image_rect"))
						{
							auto const & rect = entry["image_rect"];
							if (!rect.is_array() || rect.size() != 4) continue;
							object.image_rect = { rect[0].get<float>(), rect[1].get<float>(), rect[2].get<float>(), rect[3].get<float>() };
						}
						if (entry.contains("tint")) object.tint = parse_gameplay_tint(entry["tint"], object.tint);
						object.depth = entry.value("depth", object.depth);
						scene_data.environment_objects.push_back(std::move(object));
					}
					catch (json::exception const & error)
					{
						LOG(WARNING) << "Skipping malformed environment object: " << error.what();
					}
				}
			}
			if (root.contains("tint"))
				scene_data.tint = parse_gameplay_tint(root["tint"], scene_data.tint);
			if (root.contains("camera"))
				scene_data.camera = parse_gameplay_camera(root["camera"], scene_data.camera);

			if (root.contains("bounds"))
				scene_data.bounds = parse_gameplay_polygon(root["bounds"]);

			for (json const & scent_trail_json : root.value("scent_trails", json::array()))
				scene_data.scent_trails.push_back(parse_scent_trail(scent_trail_json));

			for (json const & message_trigger_json : root.value("message_triggers", json::array()))
				scene_data.message_triggers.push_back(parse_gameplay_message_trigger(message_trigger_json));

			for (json const & scene_link_json : root.value("scene_links", json::array()))
				scene_data.scene_links.push_back(parse_gameplay_scene_link(scene_link_json));
		}
		catch (std::exception const & ex)
		{
			LOG(ERROR) << "GameplaySceneLoader: Failed to parse gameplay scene file " << filepath << ": " << ex.what();
			return GameplaySceneData{};
		}

		return scene_data;
	}

	bool SaveSceneData(std::filesystem::path const & filepath, GameplaySceneData const & scene_data)
	{
		json root = json::object();
		{
			std::ifstream in_file(filepath);
			if (in_file)
			{
				try
				{
					in_file >> root;
					if (!root.is_object())
						root = json::object();
				}
				catch (std::exception const & ex)
				{
					LOG(WARNING) << "GameplaySceneLoader: Existing gameplay scene file could not be parsed before save: "
						<< filepath << ": " << ex.what();
					root = json::object();
				}
			}
		}

		root = serialize_gameplay_scene_data(scene_data, std::move(root));

		std::ofstream out_file(filepath);
		if (!out_file)
		{
			LOG(ERROR) << "GameplaySceneLoader: Failed to open gameplay scene file for writing: " << filepath;
			return false;
		}

		out_file << std::setw(2) << root << '\n';
		if (!out_file)
		{
			LOG(ERROR) << "GameplaySceneLoader: Failed to write gameplay scene file: " << filepath;
			return false;
		}

		LOG(INFO) << "GameplaySceneLoader: Saved gameplay scene file: " << filepath;
		return true;
	}
}
