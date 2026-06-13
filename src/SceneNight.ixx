// SceneNight.ixx

module;

#include <array>
#include <string_view>

export module SceneNight;

import Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
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
		StoryText{ .text = "The sun slipped behind the hills.<pause>And the woods turned blue and quiet.<pause>", .font_size = TitleFontSize },
	};
	// baby exhausted
	static constexpr std::array<StoryText, 1> ExhaustedTexts{
		StoryText{ .text = "The little one rubbed his tired eyes.<pause>His brave heart had carried him far.<pause>But even brave hearts need rest.<pause>", .font_size = TitleFontSize },
	};
	// dog awake while baby sleeps, slow fade per line, gentle glowing fireflies, soft piane/music
	static constexpr std::array<StoryText, 1> NightTexts{
		StoryText{ .text = "They curled up beneath the trees.<long pause>While the little one slept,<pause>the dog listened.<pause>He watched.<pause>He waited.<pause>And he stayed close.<pause>", .font_size = TitleFontSize },
	};
	// early sunlight, light rays, slight glowing text on "familiar scent"
	static constexpr std::array<StoryText, 1> MorningTexts{
		StoryText{ .text = "Morning warmed the leaves.<pause>The dog lifted his nose.<pause>And on the breeze came something familiar.<pause>The scent of home.", .font_size = TitleFontSize },
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
