// SceneNight.ixx

module;

#include <array>
#include <string_view>

export module SceneNight;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneNight : public StoryScene
{
public:
	explicit SceneNight(dh::RenderContext const & render_context);

private:
	// Purpose: Vulnerability + loyalty + hope.
	// Emotional arc: Exhaustion -> fear -> comfort -> determination
	// forest darkening
	static constexpr std::array<StoryText, 1> DuskTexts{
		StoryText{ .text = "The sun slipped away.<pause>And the woods grew dark.<pause>", .font_size = TitleFontSize },
	};
	// baby exhausted
	static constexpr std::array<StoryText, 1> ExhaustedTexts{
		StoryText{ .text = "The little one was tired.<pause>Very tired.<pause>", .font_size = TitleFontSize },
	};
	// dog awake while baby sleeps, slow fade per line, gentle glowing fireflies, soft piane/music
	static constexpr std::array<StoryText, 1> NightTexts{
		StoryText{ .text = "So they rested.<long pause>The puppy did not sleep.<pause>He listened.<pause>He watched.<pause>He stayed close.<pause>", .font_size = TitleFontSize },
	};
	// early sunlight, light rays, slight glowing text on "familiar scent"
	static constexpr std::array<StoryText, 1> MorningTexts{
		StoryText{ .text = "Morning came.<pause>And somewhere in the wind,<pause>a familiar scent returned.", .font_size = TitleFontSize },
	};
	static constexpr std::array<StoryPage, 4> StoryPages{
		StoryPage{ .bg_image_filename = "dusk.png", .story_texts = DuskTexts },
		StoryPage{ .bg_image_filename = "exhausted.png", .story_texts = ExhaustedTexts },
		StoryPage{ .bg_image_filename = "night.png", .story_texts = NightTexts },
		StoryPage{ .bg_image_filename = "morning.png", .story_texts = MorningTexts },
	};
	static constexpr SceneId NextSceneId = SceneId::ForestIntersection;
};

SceneNight::SceneNight(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryPages, NextSceneId)
{
}
