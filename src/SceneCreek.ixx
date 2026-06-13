// SceneCreek.ixx

module;

#include <array>
#include <string_view>

export module SceneCreek;

import Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
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
		StoryText{ .text = "By afternoon, the trees pressed in.<pause>The path grew thin.<pause>And soon,\nit disappeared into a creek.<pause>", .font_size = TitleFontSize },
	};
	// words appear slower, slight wobble on "Too scary."
	static constexpr std::array<StoryText, 1> BabyReluctantTexts{
		StoryText{ .text = "To the little one,\nthe water sounded enormous.<pause>Too cold.<pause>Too fast.<pause>Too wide to cross.<pause>", .font_size = TitleFontSize },
	};
	// dog guiding path
	static constexpr std::array<StoryText, 1> CrossingCreekTexts{
		StoryText{ .text = "But the dog lowered his nose<pause>and searched for the safest way.<pause>One careful step...<pause>then another.<pause>", .font_size = TitleFontSize },
	};
	// they make it across, music becomes hopeful, camera slowly pushes in
	static constexpr std::array<StoryText, 1> BeyondTheCreekTexts{
		StoryText{ .text = "On the other bank,<pause>the little one smiled.<pause>And together,<pause>they kept going.<pause>", .font_size = TitleFontSize },
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
