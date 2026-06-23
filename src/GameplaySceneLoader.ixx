// GameplaySceneLoader.ixx

module;

#include <filesystem>
#include <fstream>
#include <iomanip>
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

using json = nlohmann::ordered_json;

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

ScentTrailData parse_scent_trail(json const & j)
{
	ScentTrailData scent_trail;
	if (!j.is_object())
		return scent_trail;

	for (json const & point_json : j.value("points", json::array()))
		scent_trail.points.push_back(parse_gameplay_vec2(point_json, glm::vec2{ 0.0f }));

	return scent_trail;
}

GameplaySceneLink parse_gameplay_scene_link(json const & j)
{
	GameplaySceneLink scene_link;

	const std::string target_scene_id = j.value("target_scene_id", std::string{ "exit" });
	if (std::optional<SniffTheWay::SceneId> parsed_target_scene_id = SniffTheWay::SceneIdFromString(target_scene_id))
	{
		scene_link.target_scene_id = *parsed_target_scene_id;
	}
	else
	{
		std::cout << "GameplaySceneLoader: Unknown scene link target id '" << target_scene_id << "'. Using exit." << std::endl;
	}

	if (j.contains("trigger"))
		scene_link.trigger = parse_gameplay_polygon(j["trigger"]);

	json const arrival_points_json = j.value("arrival_points", json::object());
	if (arrival_points_json.contains("dog"))
		scene_link.dog_arrival_pos = parse_gameplay_vec2(arrival_points_json["dog"], scene_link.dog_arrival_pos);

	if (arrival_points_json.contains("baby"))
		scene_link.baby_arrival_pos = parse_gameplay_vec2(arrival_points_json["baby"], scene_link.baby_arrival_pos);

	return scene_link;
}

json serialize_gameplay_vec2(glm::vec2 value)
{
	return json::array({ value.x, value.y });
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

std::string serialize_gameplay_scene_state(SniffTheWay::SceneState state)
{
	switch (state)
	{
	case SniffTheWay::SceneState::Story:
		return "story";
	case SniffTheWay::SceneState::Paused:
		return "paused";
	case SniffTheWay::SceneState::Editing:
		return "gameplay";
	case SniffTheWay::SceneState::Gameplay:
		return "gameplay";
	}

	return "gameplay";
}

std::string serialize_gameplay_align(UILabel::Align align)
{
	switch (align)
	{
	case UILabel::Align::Left:
		return "left";
	case UILabel::Align::Right:
		return "right";
	case UILabel::Align::Center:
		return "center";
	}

	return "center";
}

json serialize_gameplay_story_text(StoryText const & story_text)
{
	return json{
		{ "text", story_text.text },
		{ "font_size", story_text.font_size },
		{ "pos", serialize_gameplay_vec2(story_text.pos) },
		{ "align", serialize_gameplay_align(story_text.align) },
		{ "show_time", story_text.show_time },
		{ "fade_duration", story_text.fade_duration },
		{ "color", serialize_gameplay_vec4(story_text.color) }
	};
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

json serialize_gameplay_scene_link(GameplaySceneLink const & scene_link)
{
	return json{
		{ "target_scene_id", std::string{ SniffTheWay::ToString(scene_link.target_scene_id) } },
		{ "arrival_points", json{
			{ "dog", serialize_gameplay_vec2(scene_link.dog_arrival_pos) },
			{ "baby", serialize_gameplay_vec2(scene_link.baby_arrival_pos) }
		} },
		{ "trigger", serialize_gameplay_polygon(scene_link.trigger) }
	};
}

json serialize_gameplay_scene_data(GameplaySceneData const & scene_data, json existing_root = json::object())
{
	json root = json::object();
	if (existing_root.contains("id"))
		root["id"] = existing_root["id"];

	root["background"] = scene_data.bg_image_filename;
	root["initial_state"] = serialize_gameplay_scene_state(scene_data.initial_state);
	root["bounds"] = serialize_gameplay_polygon(scene_data.bounds);
	root["default_spawn"] = json{
		{ "dog", serialize_gameplay_vec2(scene_data.dog_spawn_pos) },
		{ "baby", serialize_gameplay_vec2(scene_data.baby_spawn_pos) }
	};
	root["scent_trail"] = serialize_scent_trail(scene_data.scent_trail);

	json scene_links = json::array();
	for (GameplaySceneLink const & scene_link : scene_data.scene_links)
		scene_links.push_back(serialize_gameplay_scene_link(scene_link));
	root["scene_links"] = std::move(scene_links);

	json story_texts = json::array();
	for (StoryText const & story_text : scene_data.story_texts)
		story_texts.push_back(serialize_gameplay_story_text(story_text));
	root["story_texts"] = std::move(story_texts);

	for (auto const & [key, value] : existing_root.items())
	{
		if (key != "id" &&
			key != "background" &&
			key != "initial_state" &&
			key != "bounds" &&
			key != "default_spawn" &&
			key != "scent_trail" &&
			key != "story_texts" &&
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

			if (root.contains("scent_trail"))
				scene_data.scent_trail = parse_scent_trail(root["scent_trail"]);

			for (json const & text_json : root.value("story_texts", json::array()))
				scene_data.story_texts.push_back(parse_gameplay_story_text(text_json));

			for (json const & scene_link_json : root.value("scene_links", json::array()))
				scene_data.scene_links.push_back(parse_gameplay_scene_link(scene_link_json));
		}
		catch (std::exception const & ex)
		{
			std::cout << "GameplaySceneLoader: Failed to parse gameplay scene file " << filepath << ": " << ex.what() << std::endl;
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
					std::cout << "GameplaySceneLoader: Existing gameplay scene file could not be parsed before save: "
						<< filepath << ": " << ex.what() << std::endl;
					root = json::object();
				}
			}
		}

		root = serialize_gameplay_scene_data(scene_data, std::move(root));

		std::ofstream out_file(filepath);
		if (!out_file)
		{
			std::cout << "GameplaySceneLoader: Failed to open gameplay scene file for writing: " << filepath << std::endl;
			return false;
		}

		out_file << std::setw(2) << root << '\n';
		if (!out_file)
		{
			std::cout << "GameplaySceneLoader: Failed to write gameplay scene file: " << filepath << std::endl;
			return false;
		}

		std::cout << "GameplaySceneLoader: Saved gameplay scene file: " << filepath << std::endl;
		return true;
	}
}
