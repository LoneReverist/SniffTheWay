// StoryData.ixx

module;

#include <string>
#include <vector>

#include <glm/glm.hpp>

export module StoryData;

import DecorationAtlas;
import SniffTheWayConstants;
import UILabel;

export struct StoryText
{
	std::string text;
	float font_size = SniffTheWay::LabelFontSize;
	glm::vec2 pos{ 960.0f, 250.0f };
	UILabel::Align align = UILabel::Align::Center;
	float show_time = 0.0f;
	float fade_duration = 0.5f;
	glm::vec4 color = SniffTheWay::StoryTextColor;
};

export struct StoryDecoration
{
	SniffTheWay::Decorations::DecorationId decoration_id;
	glm::vec2 center{ 960.0f, 110.0f };
	float scale = 1.5f;
	float show_time = 0.0f;
	float fade_duration = 0.5f;
	glm::vec4 color = SniffTheWay::StoryTextColor;
};

export struct StoryPage
{
	std::string bg_image_filename;
	std::vector<StoryText> story_texts;
	std::vector<StoryDecoration> decorations;
};

export struct StorySceneData
{
	SniffTheWay::SceneId next_scene_id = SniffTheWay::SceneId::Exit;
	std::vector<StoryPage> pages;
};
