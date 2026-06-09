// SceneCreek.ixx

module;

#include <array>
#include <string_view>

export module SceneCreek;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneCreek : public StoryScene
{
public:
	explicit SceneCreek(dh::RenderContext const & render_context);

private:
	// Purpose: Show growing teamwork and deepen emotional bond.
	// Emotional arc: Obstacle -> fear -> trust -> teamwork
	// water blocks path
	static constexpr std::array<StoryText, 1> ApproachingCreekTexts{
		StoryText{ .text = "The forest grew thicker.<pause>And soon,\nthe path disappeared.<pause>", .font_size = TitleFontSize },
	};
	// words appear slower, slight wobble on "Too scary."
	static constexpr std::array<StoryText, 1> BabyReluctantTexts{
		StoryText{ .text = "The water looked cold.<pause>Too far.<pause>Too scary.<pause>", .font_size = TitleFontSize },
	};
	// dog guiding path
	static constexpr std::array<StoryText, 1> CrossingCreekTexts{
		StoryText{ .text = "But the puppy searched carefully.<pause>One safe step...<pause>then another.<pause>", .font_size = TitleFontSize },
	};
	// they make it across, music becomes hopeful, camera slowly pushes in
	static constexpr std::array<StoryText, 1> BeyondTheCreekTexts{
		StoryText{ .text = "Together,<pause>they kept going.<pause>", .font_size = TitleFontSize },
	};
	static constexpr std::array<StoryPage, 4> StoryPages{
		StoryPage{ .bg_image_filename = "approaching_creek.png", .story_texts = ApproachingCreekTexts },
		StoryPage{ .bg_image_filename = "baby_reluctant.png", .story_texts = BabyReluctantTexts },
		StoryPage{ .bg_image_filename = "crossing_creek.png", .story_texts = CrossingCreekTexts },
		StoryPage{ .bg_image_filename = "beyond_the_creek.png", .story_texts = BeyondTheCreekTexts },
	};
	static constexpr SceneId NextSceneId = SceneId::DarkForest;
};

SceneCreek::SceneCreek(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryPages, NextSceneId)
{
}
