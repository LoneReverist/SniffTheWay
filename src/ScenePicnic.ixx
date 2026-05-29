// ScenePicnic.ixx

module;

#include <array>
#include <string_view>

export module ScenePicnic;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class ScenePicnic : public StoryScene
{
public:
	explicit ScenePicnic(dh::RenderContext const & render_context);

private:
	// Purpose: Establish love, safety, family bond, and the accidental separation.
	// Emotional arc: Warmth → distraction → curiosity → separation
	static constexpr std::array<std::string_view, 4> StoryImages{
		"picnic.png",
		"gust_of_wind.png",
		"following_butterflies.png",
		"lost.png",
	};
	static constexpr std::array<std::string_view, 4> StoryTexts{
		// warm fade in, slight floating motion on text, gentle ambient music
		"It was a perfect day for a picnic.<pause>A little baby.\nA loyal dog.<pause>And a family together.<pause>",
		// leaves in wind effect, text blown sideways, slight camera shake
		"Then,<short pause>WHOOSH!<pause>",
		// soft typewriter reveal, butterfly movement in scene
		"While everyone looked away...<pause>tiny footsteps wandered.<pause>And one brave nose followed close behind.<pause>",
		// music softens, longer silence before gameplay starts
		"When they stopped,<pause>home was nowhere to be seen.<long pause>But the puppy stayed close.<pause>",
	};
};

ScenePicnic::ScenePicnic(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryImages, StoryTexts)
{
	m_next_scene_id = SceneId::ForestPath;
}
