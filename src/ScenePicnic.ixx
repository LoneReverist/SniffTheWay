// ScenePicnic.ixx

module;

#include <array>
#include <string_view>

#include <glm/glm.hpp>

export module ScenePicnic;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;
import UILabel;

using namespace SniffTheWay;

export class ScenePicnic : public StoryScene
{
public:
	explicit ScenePicnic(dh::RenderContext const & render_context);

private:
	// Purpose: Establish love, safety, family bond, and the accidental separation.
	// Emotional arc: Warmth -> distraction -> curiosity -> separation
	// warm fade in, slight floating motion on text, gentle ambient music
	static constexpr std::array<StoryText, 3> PicnicTexts{
		StoryText{
			.text = "It was a perfect day for a picnic.",
			.font_size = TitleFontSize,
			.pos = glm::vec2{ 960, 250 },
			.align = UILabel::Align::Center,
			.show_time = 1.5f,
		},
		StoryText{
			.text = "A little baby. A loyal dog.",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 340 },
			.align = UILabel::Align::Center,
			.show_time = 3.0f,
		},
		StoryText{
			.text = "And a family together.",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 410 },
			.align = UILabel::Align::Center,
			.show_time = 4.5f,
		},
	};
	// leaves in wind effect, text blown sideways, slight camera shake
	static constexpr std::array<StoryText, 2> GustOfWindTexts{
		StoryText{
			.text = "Then,",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 860, 250 },
			.align = UILabel::Align::Right,
			.show_time = 0.75f,
		},
		StoryText{
			.text = "WHOOSH!",
			.font_size = TitleFontSize,
			.pos = glm::vec2{ 910, 250 },
			.align = UILabel::Align::Left,
			.show_time = 1.5f,
			.fade_duration = 0.25f,
		},
	};
	// soft typewriter reveal, butterfly movement in scene
	static constexpr std::array<StoryText, 3> FollowingButterfliesTexts{
		StoryText{
			.text = "While everyone looked away...",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 250 },
			.align = UILabel::Align::Center,
			.show_time = 0.75f,
		},
		StoryText{
			.text = "tiny footsteps wandered.",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 320 },
			.align = UILabel::Align::Center,
			.show_time = 2.25f,
		},
		StoryText{
			.text = "And one brave nose followed close behind.",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 390 },
			.align = UILabel::Align::Center,
			.show_time = 3.75f,
		},
	};
	// music softens, longer silence before gameplay starts
	static constexpr std::array<StoryText, 3> LostTexts{
		StoryText{
			.text = "When they stopped,",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 250 },
			.align = UILabel::Align::Center,
			.show_time = 0.75f,
		},
		StoryText{
			.text = "home was nowhere to be seen.",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 320 },
			.align = UILabel::Align::Center,
			.show_time = 2.0f,
		},
		StoryText{
			.text = "But the puppy stayed close.",
			.font_size = LabelFontSize,
			.pos = glm::vec2{ 960, 390 },
			.align = UILabel::Align::Center,
			.show_time = 4.0f,
		},
	};
	static constexpr std::array<StoryPage, 4> StoryPages{
		StoryPage{ .bg_image_filename = "picnic.png", .story_texts = PicnicTexts },
		StoryPage{ .bg_image_filename = "gust_of_wind.png", .story_texts = GustOfWindTexts },
		StoryPage{ .bg_image_filename = "following_butterflies.png", .story_texts = FollowingButterfliesTexts },
		StoryPage{ .bg_image_filename = "lost.png", .story_texts = LostTexts },
	};
};

ScenePicnic::ScenePicnic(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryPages)
{
	m_next_scene_id = SceneId::ForestPath;
}
