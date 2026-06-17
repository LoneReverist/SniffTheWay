// GameplaySceneLoader.ixx

module;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

export module GameplaySceneLoader;

import GameplaySceneData;
import Polygon2d;
import SniffTheWayConstants;
import StoryData;
import UILabel;

using json = nlohmann::json;

glm::vec2 parse_gameplay_vec2(json const & j, glm::vec2 fallback)
{
	if (!j.is_array() || j.size() != 2)
		return fallback;

	return glm::vec2{ j[0].get<float>(), j[1].get<float>() };
}

glm::vec4 parse_gameplay_vec4(json const & j, glm::vec4 fallback)
{
	if (!j.is_array() || j.size() != 4)
		return fallback;

	return glm::vec4{ j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>() };
}

UILabel::Align parse_gameplay_align(std::string const & align)
{
	if (align == "left")
		return UILabel::Align::Left;
	if (align == "right")
		return UILabel::Align::Right;

	return UILabel::Align::Center;
}

SniffTheWay::SceneState parse_gameplay_scene_state(std::string const & state)
{
	if (state == "story")
		return SniffTheWay::SceneState::Story;
	if (state == "paused")
		return SniffTheWay::SceneState::Paused;

	return SniffTheWay::SceneState::Gameplay;
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

StoryText parse_gameplay_story_text(json const & j)
{
	StoryText story_text;
	story_text.text = j.value("text", "");
	story_text.font_size = j.value("font_size", story_text.font_size);
	if (j.contains("pos"))
		story_text.pos = parse_gameplay_vec2(j["pos"], story_text.pos);
	story_text.align = parse_gameplay_align(j.value("align", "center"));
	story_text.show_time = j.value("show_time", story_text.show_time);
	story_text.fade_duration = j.value("fade_duration", story_text.fade_duration);
	if (j.contains("color"))
		story_text.color = parse_gameplay_vec4(j["color"], story_text.color);

	return story_text;
}

GameplayAdjacentScene parse_gameplay_adjacent_scene(json const & j)
{
	GameplayAdjacentScene adjacent_scene;

	const std::string scene_id = j.value("scene_id", std::string{ "exit" });
	if (std::optional<SniffTheWay::SceneId> parsed_scene_id = SniffTheWay::SceneIdFromString(scene_id))
	{
		adjacent_scene.scene_id = *parsed_scene_id;
	}
	else
	{
		std::cout << "GameplaySceneLoader: Unknown adjacent scene id '" << scene_id << "'. Using exit." << std::endl;
	}

	if (j.contains("collider"))
		adjacent_scene.collider = parse_gameplay_polygon(j["collider"]);
	if (j.contains("dog_spawn"))
		adjacent_scene.dog_spawn_pos = parse_gameplay_vec2(j["dog_spawn"], adjacent_scene.dog_spawn_pos);
	if (j.contains("baby_spawn"))
		adjacent_scene.baby_spawn_pos = parse_gameplay_vec2(j["baby_spawn"], adjacent_scene.baby_spawn_pos);

	return adjacent_scene;
}

export namespace GameplaySceneLoader
{
	GameplaySceneData LoadSceneData(std::filesystem::path const & filepath)
	{
		GameplaySceneData scene_data;

		std::ifstream file(filepath);
		if (!file)
		{
			std::cout << "GameplaySceneLoader: Failed to open gameplay scene file: " << filepath << std::endl;
			return scene_data;
		}

		json root;
		try
		{
			file >> root;

			scene_data.bg_image_filename = root.value("background", "");
			scene_data.initial_state = parse_gameplay_scene_state(root.value("initial_state", "gameplay"));

			if (root.contains("bounds"))
				scene_data.bounds = parse_gameplay_polygon(root["bounds"]);

			json const spawn_json = root.value("default_spawn", json::object());
			if (spawn_json.contains("dog"))
				scene_data.dog_spawn_pos = parse_gameplay_vec2(spawn_json["dog"], scene_data.dog_spawn_pos);
			if (spawn_json.contains("baby"))
				scene_data.baby_spawn_pos = parse_gameplay_vec2(spawn_json["baby"], scene_data.baby_spawn_pos);

			for (json const & text_json : root.value("story_texts", json::array()))
				scene_data.story_texts.push_back(parse_gameplay_story_text(text_json));

			for (json const & adjacent_json : root.value("adjacent_scenes", json::array()))
				scene_data.adjacent_scenes.push_back(parse_gameplay_adjacent_scene(adjacent_json));
		}
		catch (std::exception const & ex)
		{
			std::cout << "GameplaySceneLoader: Failed to parse gameplay scene file " << filepath << ": " << ex.what() << std::endl;
			return GameplaySceneData{};
		}

		return scene_data;
	}
}
