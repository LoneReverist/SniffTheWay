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
	// Emotional arc: Exhaustion → fear → comfort → determination
	static constexpr std::array<std::string_view, 2> StoryImages{
		//"dusk.png",
		//"exhausted.png",
		"night.png",
		"morning.png",
	};
	static constexpr std::array<std::string_view, 4> StoryTexts{
		// forest darkening
		"The sun slipped away.<pause>And the woods grew dark.<pause>",
		// baby exhausted
		"The little one was tired.<pause>Very tired.<pause>",
		// dog awake while baby sleeps, slow fade per line, gentle glowing fireflies, soft piane/music
		"So they rested.<long pause>The puppy did not sleep.<pause>He listened.<pause>He watched.<pause>He stayed close.<pause>",
		// early sunlight, light rays, slight glowing text on "familiar scent"
		"Morning came.<pause>And somewhere in the wind,<pause>a familiar scent returned.",
	};
};

SceneNight::SceneNight(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryImages, StoryTexts)
{
	m_next_scene_id = SceneId::ForestIntersection;
}
