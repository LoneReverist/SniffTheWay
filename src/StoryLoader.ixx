// StoryLoader.ixx

module;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

export module StoryLoader;

import DecorationAtlas;
import SniffTheWayConstants;
import StoryData;
import UILabel;

using json = nlohmann::json;

glm::vec2 parse_vec2(json const & j, glm::vec2 fallback)
{
	if (!j.is_array() || j.size() != 2)
		return fallback;

	return glm::vec2{ j[0].get<float>(), j[1].get<float>() };
}

glm::vec4 parse_vec4(json const & j, glm::vec4 fallback)
{
	if (!j.is_array() || j.size() != 4)
		return fallback;

	return glm::vec4{ j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>() };
}

UILabel::Align parse_align(std::string const & align)
{
	if (align == "left")
		return UILabel::Align::Left;
	if (align == "right")
		return UILabel::Align::Right;

	return UILabel::Align::Center;
}

SniffTheWay::Decorations::DecorationId parse_decoration_id(std::string const & name)
{
	for (SniffTheWay::Decorations::DecorationInfo const & decoration : SniffTheWay::Decorations::AllDecorations)
	{
		if (name == decoration.name)
			return decoration.id;
	}

	std::cout << "StoryLoader: Unknown decoration id '" << name << "'. Using default." << std::endl;
	return SniffTheWay::Decorations::DecorationId::HorizontalDividerPawFlourish;
}

StoryText parse_story_text(json const & j)
{
	StoryText story_text;
	story_text.text = j.value("text", "");
	story_text.font_size = j.value("font_size", story_text.font_size);
	if (j.contains("pos"))
		story_text.pos = parse_vec2(j["pos"], story_text.pos);
	story_text.align = parse_align(j.value("align", "center"));
	story_text.show_time = j.value("show_time", story_text.show_time);
	story_text.fade_duration = j.value("fade_duration", story_text.fade_duration);
	if (j.contains("color"))
		story_text.color = parse_vec4(j["color"], story_text.color);

	return story_text;
}

StoryDecoration parse_story_decoration(json const & j)
{
	StoryDecoration decoration;
	decoration.decoration_id = parse_decoration_id(j.value("decoration_id", "horizontal_divider_paw_flourish"));
	if (j.contains("center"))
		decoration.center = parse_vec2(j["center"], decoration.center);
	decoration.scale = j.value("scale", decoration.scale);
	decoration.show_time = j.value("show_time", decoration.show_time);
	decoration.fade_duration = j.value("fade_duration", decoration.fade_duration);
	if (j.contains("color"))
		decoration.color = parse_vec4(j["color"], decoration.color);

	return decoration;
}

export namespace StoryLoader
{
	StorySceneData LoadSceneData(std::filesystem::path const & filepath)
	{
		StorySceneData scene_data;

		std::ifstream file(filepath);
		if (!file)
		{
			std::cout << "StoryLoader: Failed to open story file: " << filepath << std::endl;
			return scene_data;
		}

		json root;
		try
		{
			file >> root;

			const std::string next_scene_id = root.value("next_scene_id", std::string{ "exit" });
			if (std::optional<SniffTheWay::SceneId> scene_id = SniffTheWay::SceneIdFromString(next_scene_id))
			{
				scene_data.next_scene_id = *scene_id;
			}
			else
			{
				std::cout << "StoryLoader: Unknown next scene id '" << next_scene_id << "' in " << filepath << ". Using exit." << std::endl;
			}

			for (json const & page_json : root.at("pages"))
			{
				StoryPage page;
				page.bg_image_filename = page_json.value("background", "");

				for (json const & text_json : page_json.value("texts", json::array()))
					page.story_texts.push_back(parse_story_text(text_json));

				for (json const & decoration_json : page_json.value("decorations", json::array()))
					page.decorations.push_back(parse_story_decoration(decoration_json));

				scene_data.pages.push_back(std::move(page));
			}
		}
		catch (std::exception const & ex)
		{
			std::cout << "StoryLoader: Failed to parse story file " << filepath << ": " << ex.what() << std::endl;
			scene_data.pages.clear();
		}

		return scene_data;
	}
}
